#ifndef _CC_H
#define _CC_H

#include <linux/kobject.h>
#include <linux/interrupt.h>

struct ethernet_port;

struct ethernet_port_fns {
    bool (*action_filter)(void*, struct ethernet_port*);

    void (*hook_before_carrier_off)(struct ethernet_port*);
    void (*hook_before_carrier_on)(struct ethernet_port*);
};

struct irq_context {
    unsigned int irq;
    irq_handler_t handler;
    unsigned long flags;
    char *dev_name;
    void *dev_id;

    struct list_head list;
};

struct ethernet_port {
    struct kobject kobj;

    struct net_device *netdev;
    struct irq_context context;

    struct ethernet_port_fns *fns;

    struct list_head list;
};

void carrier_off(struct ethernet_port *port);

void carrier_on(struct ethernet_port *port);

void create_ethernet_ports(void);

void destroy_ethernet_ports(void);

#endif
