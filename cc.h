#ifndef _CC_H
#define _CC_H

#include <linux/kobject.h>

struct ethernet_port {
    struct kobject kobj;

    struct net_device *netdev;

    struct list_head list;
};

void carrier_off(struct ethernet_port *port);

void carrier_on(struct ethernet_port *port);

void create_ethernet_ports(void);

void destroy_ethernet_ports(void);

#endif
