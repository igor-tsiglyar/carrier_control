#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include "cc_sysfs.h"
#include "cc_hook.h"
#include "cc.h"


void carrier_on(struct ethernet_port *port)
{
    if (!netif_carrier_ok(port->netdev)) {
        netif_carrier_on(port->netdev);
    }
}


void carrier_off(struct ethernet_port *port)
{
    if (netif_carrier_ok(port->netdev)) {
        netif_carrier_off(port->netdev);
    }
}


static struct ethernet_port ethernet_ports;


static void alloc_ethernet_port(struct net_device *netdev)
{
    struct ethernet_port *port = NULL;

    if (!netdev) {
        return;
    }

    port = kzalloc(sizeof(struct ethernet_port), GFP_KERNEL);

    if (!port) {
        pr_err("Cannot allocate ethernet_port for %s", netdev->name);
        return;
    }

    port->netdev = netdev;

    if (create_ethernet_port_dir(port)) {
        kfree(port);
        return;
    }

    list_add_tail(&port->list, &ethernet_ports.list);

    pr_info("port %s is created", netdev->name);
}


void create_ethernet_ports(void)
{
    struct net_device *dev;
    static bool first_time = true;

    INIT_LIST_HEAD(&ethernet_ports.list);

    read_lock(&dev_base_lock);
    for_each_netdev(&init_net, dev) {
        if (first_time && !netif_carrier_ok(dev)) {
            pr_notice("%s has no-carrier on initialization", dev->name);
        } else if (strcmp(dev->name, "lo")) {
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
        destroy_ethernet_port_dir(port);

        list_del(&port->list);
    }
}


bool should_drop_packet(const struct net_device *netdev)
{
    struct ethernet_port *port;

    list_for_each_entry(port, &ethernet_ports.list, list) {
        if (port->netdev == netdev) {
            unsigned char rand;

            get_random_bytes(&rand, sizeof(char));

            return ((int) rand) % 100 < port->packet_drop_rate;
        }
    }

    return false;
}


bool should_corrupt_bit(const struct net_device *netdev)
{
    struct ethernet_port *port;

    list_for_each_entry(port, &ethernet_ports.list, list) {
        if (port->netdev == netdev) {
            unsigned char rand;

            get_random_bytes(&rand, sizeof(char));

            return ((int) rand) % 100 < port->bit_corrupt_rate;
        }
    }

    return false;
}


static int __init carrier_control_init(void)
{
    int rc = create_module_dir();

    if (rc) {
        return rc;
    }

    create_ethernet_ports();

    init_hooks();

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

    destroy_hooks();
}


module_init(carrier_control_init);
module_exit(carrier_control_exit);


MODULE_LICENSE("GPL");

