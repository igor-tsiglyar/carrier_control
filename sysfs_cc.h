#ifndef _SYSFS_CC_H
#define _SYSFS_CC_H

struct ethernet_port;

int create_module_dir(void);

void destroy_module_dir(void);

int create_ethernet_port_dir(struct ethernet_port *port);

void destroy_ethernet_port_dir(struct ethernet_port *port);

#endif
