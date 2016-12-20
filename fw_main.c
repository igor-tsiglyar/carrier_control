#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include "fw_sysfs.h"
#include "fw_hook.h"
#include "fw.h"


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
        destroy_ethernet_port_dir(port);

        list_del(&port->list);
    }
}


bool should_forbid_udp_packet(const struct net_device *netdev, uint16_t udp_port)
{
    struct ethernet_port *port;

    list_for_each_entry(port, &ethernet_ports.list, list) {
        if (port->netdev == netdev) {
            return htons(port->udp) == udp_port;
        }
    }

    return false;
}


bool should_forbid_tcp_packet(const struct net_device *netdev, uint16_t tcp_port)
{
    struct ethernet_port *port;

    list_for_each_entry(port, &ethernet_ports.list, list) {
        if (port->netdev == netdev) {
            return htons(port->tcp) == tcp_port;
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
    destroy_ethernet_ports();

    destroy_module_dir();

    destroy_hooks();
}


module_init(carrier_control_init);
module_exit(carrier_control_exit);


MODULE_LICENSE("GPL");

