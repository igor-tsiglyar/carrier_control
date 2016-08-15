#ifndef _TG3_CC_H
#define _TG3_CC_H

struct ethernet_port;

bool tg3_action_filter(void *action_dev_id, struct ethernet_port* port);

#endif

