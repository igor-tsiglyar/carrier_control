#include "../drivers/net/ethernet/broadcom/bnx2x/bnx2x.h"
#include "bnx2x_cc.h"
#include "cc.h"


bool bnx2x_action_filter(void *action_dev_id, struct ethernet_port* port)
{
    if (action_dev_id == port->netdev) {
        return true;
    } else {
        struct bnx2x *bp = netdev_priv(port->netdev);
        int i;

        for_each_eth_queue(bp, i) {
            struct bnx2x_fastpath *fp = &bp->fp[i];

            if (action_dev_id == fp) {
                return true;
            }
        }
    }

    return false;
}

