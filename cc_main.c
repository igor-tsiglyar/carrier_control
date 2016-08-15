#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include "sysfs_cc.h"
#include "cc.h"
#include "e1000_cc.h"
#include "bnx2x_cc.h"
#include "tg3_cc.h"


void carrier_on(struct ethernet_port *port)
{
    if (!netif_carrier_ok(port->netdev)) {
        struct irq_context *context;

        list_for_each_entry(context, &port->context.list, list) {
            if (request_irq(context->irq, context->handler, context->flags,
                            context->dev_name, context->dev_id)) {
                return;
            }
        }

        if (port->fns->hook_before_carrier_on) {
            port->fns->hook_before_carrier_on(port);
        }

        netif_carrier_on(port->netdev);
    }
}


void carrier_off(struct ethernet_port *port)
{
    if (netif_carrier_ok(port->netdev)) {
        struct irq_context *context;

        list_for_each_entry(context, &port->context.list, list) {
            free_irq(context->irq, context->dev_id);
        }

        if (port->fns->hook_before_carrier_off) {
            port->fns->hook_before_carrier_off(port);
        }

        netif_carrier_off(port->netdev);
    }
}


static bool default_action_filter(void *action_dev_id, struct ethernet_port *port)
{
    return action_dev_id == port->netdev;
}


enum ethernet_port_variants {
    unknown = 0,
    e1000,
    bnx2x,
    tg3
};


static const char *cc_netdev_drivername(const struct net_device *dev)
{
    const struct device_driver *driver;
    const struct device *parent;
    const char *empty = "";

    parent = dev->dev.parent;
    if (!parent) {
        return empty;
    }

    driver = parent->driver;
    if (driver && driver->name) {
        return driver->name;
    }

    return empty;
}


static int ethernet_port_variant(struct ethernet_port *port)
{
    const char *driver_name = cc_netdev_drivername(port->netdev);

    if (!strcmp(driver_name, "e1000")) {
        return e1000;
    } else if (!strcmp(driver_name, "bnx2x")) {
        return bnx2x;
    } else if (!strcmp(driver_name, "tg3")) {
        return tg3;
    }

    return unknown;
}

static struct ethernet_port_fns ethernet_port_variant_fns[] = {
    {
        default_action_filter,
        NULL,
        NULL },
    {
        default_action_filter,
        e1000_hook_before_carrier_off,
        e1000_hook_before_carrier_on },
    {
        bnx2x_action_filter,
        NULL,
        NULL },
    {
        tg3_action_filter,
        NULL,
        NULL }
};


static struct ethernet_port ethernet_ports;


static int save_irq_context(struct ethernet_port *port)
{
    unsigned int irq;

    INIT_LIST_HEAD(&port->context.list);

    for (irq = 0; irq < NR_IRQS; ++irq) {
        struct irq_desc *desc = irq_to_desc(irq);

        if (desc) {
            struct irqaction *action;

            for (action = desc->action; action != NULL; action = action->next) {
                if (port->fns->action_filter(action->dev_id, port)) {
                    struct irq_context *context = kzalloc(sizeof(struct irq_context), GFP_KERNEL);

                    if (!context) {
                        return -ENOMEM;
                    }

                    context->irq = irq;
                    context->handler = action->handler;
                    context->flags = action->flags;
                    context->dev_name = kzalloc(strlen(action->name) + 1, GFP_KERNEL);

                    if (!context->dev_name) {
                        kfree(context);
                        return -ENOMEM;
                    }

                    strcpy(context->dev_name, action->name);
                    context->dev_id = action->dev_id;

                    list_add_tail(&context->list, &port->context.list);

                    break;
                }
            }
        }
    }

    return !list_empty(&port->context.list);
}


static void alloc_ethernet_port(struct net_device *netdev)
{
    struct ethernet_port *port = NULL;

    if (!netdev) {
        return;
    }

    port = kzalloc(sizeof(struct ethernet_port), GFP_KERNEL);

    if (!port) {
        return;
    }

    port->netdev = netdev;
    port->fns = &ethernet_port_variant_fns[ethernet_port_variant(port)];

    if (save_irq_context(port) || create_ethernet_port_dir(port)) {
        kfree(port);
        return;
    }

    list_add_tail(&port->list, &ethernet_ports.list);
}


void create_ethernet_ports(void)
{
    struct net_device *dev;
    static bool first_time = true;

    INIT_LIST_HEAD(&ethernet_ports.list);

    read_lock(&dev_base_lock);
    for_each_netdev(&init_net, dev) {
        if (strcmp(dev->name, "lo") && (!first_time || netif_carrier_ok(dev))) {
            alloc_ethernet_port(dev);
        }
    }
    read_unlock(&dev_base_lock);

    first_time = false;
}


void destroy_ethernet_ports(void)
{
    struct ethernet_port *port, *aux;

    list_for_each_entry_safe(port, aux, &ethernet_ports.list, list) {
        struct irq_context *context, *aux;

        list_del(&port->list);

        list_for_each_entry_safe(context, aux, &port->context.list, list) {
            kfree(context->dev_name);
            kfree(context);
        }

        destroy_ethernet_port_dir(port);
    }
}


static int __init carrier_control_init(void)
{
    int rc = create_module_dir();

    if (rc) {
        return rc;
    }

    create_ethernet_ports();

    return 0;
}


static void __exit carrier_control_exit(void)
{
    struct ethernet_port *port;

    list_for_each_entry(port, &ethernet_ports.list, list) {
        carrier_on(port);
    }

    destroy_ethernet_ports();
    destroy_module_dir();
}


module_init(carrier_control_init);
module_exit(carrier_control_exit);


MODULE_LICENSE("GPL");

