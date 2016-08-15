#include "../drivers/net/ethernet/broadcom/tg3.h"
#include "tg3_cc.h"
#include "cc.h"


bool tg3_action_filter(void *action_dev_id, struct ethernet_port* port)
{
    struct tg3 *tp = netdev_priv(port->netdev);
    int i;

    for (i = 0; i < tp->irq_cnt; i++) {
        struct tg3_napi *tnapi = &tp->napi[i];

        if (action_dev_id == tnapi) {
            return true;
        }
    }

    return false;
}

