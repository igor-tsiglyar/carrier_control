#ifndef _CC_H
#define _CC_H

#include <linux/kobject.h>

struct ethernet_port {
    struct kobject kobj;

    struct net_device *netdev;

    int udp;

    int tcp;

    struct list_head list;
};

void create_ethernet_ports(void);

void destroy_ethernet_ports(void);

bool should_forbid_udp_packet(const struct net_device *netdev, uint16_t udp_port);

bool should_forbid_tcp_packet(const struct net_device *netdev, uint16_t tcp_port);

#endif
