#include <linux/types.h>
#include <linux/dma-mapping.h>
#include <linux/netdevice.h>
#include <linux/ptp_clock_kernel.h>
#include <linux/phy.h>
#include "tg3_cc.h"
#include "cc.h"
#include "../drivers/net/ethernet/broadcom/tg3.h"


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


void tg3_hook_before_carrier_off(struct ethernet_port *port)
{
    struct tg3 *tp = netdev_priv(port->netdev);

    del_timer_sync(&tp->timer);
}


void tg3_hook_before_carrier_on(struct ethernet_port *port)
{
    struct tg3 *tp = netdev_priv(port->netdev);

    tp->asf_counter = tp->asf_multiplier;
    tp->timer_counter = tp->timer_multiplier;

    tp->timer.expires = jiffies + tp->timer_offset;
    add_timer(&tp->timer);
}

