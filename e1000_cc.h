#ifndef _E1000_CC
#define _E1000_CC

struct ethernet_port;

void e1000_hook_before_carrier_off(struct ethernet_port *port);

void e1000_hook_before_carrier_on(struct ethernet_port *port);

#endif
