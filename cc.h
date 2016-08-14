#ifndef _CC_H
#define _CC_H

#include <linux/kobject.h>

struct net_device;

struct ethernet_port {
    struct kobject kobj;
    struct net_device *netdev;

    int has_carrier;

    int irq;

    struct list_head list;

    void *variant_opaque;
    void (*hook_before_off)(struct ethernet_port*);
    void (*hook_before_on)(struct ethernet_port*);
};

void carrier_off(struct ethernet_port *port);

void carrier_on(struct ethernet_port *port);

int create_ethernet_ports(void);

void destroy_ethernet_ports(void);

#endif
