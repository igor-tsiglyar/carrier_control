#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/interrupt.h>
#include <linux/netdevice.h>
#include "sysfs_cc.h"
#include "cc.h"

void carrier_on(struct ethernet_port *port)
{
    if (!netif_carrier_ok(port->netdev)) {
        enable_irq(port->irq);

        if (port->hook_before_on) {
            port->hook_before_on(port);
        }

        netif_carrier_on(port->netdev);
    }
}


void carrier_off(struct ethernet_port *port)
{
    if (netif_carrier_ok(port->netdev)) {
        disable_irq(port->irq);

        if (port->hook_before_off) {
            port->hook_before_off(port);
        }

        netif_carrier_off(port->netdev);
    }
}


static struct ethernet_port ethernet_ports;


static struct ethernet_port* alloc_ethernet_port(struct net_device *netdev)
{
    struct ethernet_port *port = kzalloc(sizeof(struct ethernet_port), GFP_KERNEL);
    unsigned int irq;

    if (!port) {
        return NULL;
    }

    port->netdev = netdev;
    port->has_carrier = netif_carrier_ok(netdev);

    for (irq = 0; irq < NR_IRQS; ++irq) {
        struct irq_desc *desc = irq_to_desc(irq);

        if (desc && !strcmp(desc->name, port->netdev->name)) {
            port->irq = irq;
            break;
        }
    }

    if (create_ethernet_port_dir(port)) {
        kfree(port);
        return NULL;
    }

    INIT_LIST_HEAD(&port->list);
    list_add_tail(&port->list, &ethernet_ports.list);

    return port;
}


int create_ethernet_ports(void)
{
    struct net_device *dev;

    INIT_LIST_HEAD(&ethernet_ports.list);

    read_lock(&dev_base_lock);
    for_each_netdev(&init_net, dev) {
        if (strcmp(dev->name, "lo")) {
            struct ethernet_port *port = alloc_ethernet_port(dev);

            if (!port) {
                read_unlock(&dev_base_lock);
                return -ENOMEM;
            }
        }
    }
    read_unlock(&dev_base_lock);

    return 0;
}


void destroy_ethernet_ports(void)
{
    if (!list_empty(&ethernet_ports.list)) {
        struct ethernet_port *port, *tmp;

        list_for_each_entry_safe(port, tmp, &ethernet_ports.list, list) {
            destroy_ethernet_port_dir(port);
            list_del(&port->list);
        }

        INIT_LIST_HEAD(&ethernet_ports.list);
    }
}


static int __init carrier_control_init(void)
{
    return create_ethernet_ports();
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

