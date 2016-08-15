#include "../drivers/net/ethernet/intel/e1000/e1000.h"
#include "e1000_cc.h"
#include "cc.h"
#include <linux/netdevice.h>


void e1000_hook_before_carrier_off(struct ethernet_port *port)
{
    struct e1000_adapter *private = netdev_priv(port->netdev);
    struct delayed_work *delayed = &private->watchdog_task;

    cancel_delayed_work_sync(delayed);
}


void e1000_hook_before_carrier_on(struct ethernet_port *port)
{
    struct e1000_adapter *private = netdev_priv(port->netdev);
    struct delayed_work *delayed = &private->watchdog_task;

    INIT_DELAYED_WORK(delayed, delayed->work.func);
}

