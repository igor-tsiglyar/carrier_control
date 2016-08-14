#ifndef _CC_H
#define _CC_H

#include <linux/kobject.h>

struct net_device;
struct ethernet_port;

struct ethernet_port_fns {
    void (*init_variant_opaque)(struct ethernet_port*);

    void (*hook_before_carrier_off)(struct ethernet_port*);
    void (*hook_before_carrier_on)(struct ethernet_port*);
};

struct ethernet_port {
    struct kobject kobj;
    struct net_device *netdev;

    int has_carrier;
    int irq;

    struct list_head list;

    void *variant_opaque;
    struct ethernet_port_fns *fns;
};

void carrier_off(struct ethernet_port *port);

void carrier_on(struct ethernet_port *port);

void create_ethernet_ports(void);

void destroy_ethernet_ports(void);

#endif
