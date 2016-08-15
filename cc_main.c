#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include "sysfs_cc.h"
#include "cc.h"
#include "e1000_cc.h"

void carrier_on(struct ethernet_port *port)
{
    if (!netif_carrier_ok(port->netdev)) {
        enable_irq(port->irq);

        if (port->fns->hook_before_carrier_on) {
            port->fns->hook_before_carrier_on(port);
        }

        netif_carrier_on(port->netdev);
    }
}


void carrier_off(struct ethernet_port *port)
{
    if (netif_carrier_ok(port->netdev)) {
        disable_irq(port->irq);

        if (port->fns->hook_before_carrier_off) {
            port->fns->hook_before_carrier_off(port);
        }

        netif_carrier_off(port->netdev);
    }
}


static void default_init_variant_opaque(struct ethernet_port *port)
{
    port->variant_opaque = NULL;
}


enum ethernet_port_variants {
    unknown = 0,
    e1000
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
    }

    return unknown;
}

static struct ethernet_port_fns ethernet_port_variant_fns[] = {
    {
        default_init_variant_opaque,
        NULL,
        NULL },
    {
        e1000_init_variant_opaque,
        e1000_hook_before_carrier_off,
        e1000_hook_before_carrier_on }
};


static struct ethernet_port ethernet_ports;


static void alloc_ethernet_port(struct net_device *netdev)
{
    struct ethernet_port *port = NULL;
    unsigned int irq;

    if (!netdev) {
        return;
    }

    port = kzalloc(sizeof(struct ethernet_port), GFP_KERNEL);

    if (!port) {
        return;
    }

    port->netdev = netdev;
    port->has_carrier = netif_carrier_ok(netdev);
    port->irq = -1;
    port->fns = &ethernet_port_variant_fns[ethernet_port_variant(port)];
    port->fns->init_variant_opaque(port);

    for (irq = 0; irq < NR_IRQS; ++irq) {
        struct irq_desc *desc = irq_to_desc(irq);

        if (desc && port->irq < 0) {
            struct irqaction * action;

            for (action = desc->action; action != NULL; action = action->next) {
                if (!strcmp(action->name, netdev->name)) {
                    port->irq = irq;
                    break;
                }
            }
        }
    }

    if (port->irq < 0 || create_ethernet_port_dir(port) > 0) {
        kfree(port);
        return;
    }

    INIT_LIST_HEAD(&port->list);
    list_add_tail(&port->list, &ethernet_ports.list);
}


void create_ethernet_ports(void)
{
    struct net_device *dev;

    INIT_LIST_HEAD(&ethernet_ports.list);

    read_lock(&dev_base_lock);
    for_each_netdev(&init_net, dev) {
        if (strcmp(dev->name, "lo")) {
            alloc_ethernet_port(dev);
        }
    }
    read_unlock(&dev_base_lock);
}


void destroy_ethernet_ports(void)
{
    struct ethernet_port *port, *aux;

    list_for_each_entry_safe(port, aux, &ethernet_ports.list, list) {
        list_del(&port->list);
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

