#include "cc_hook.h"
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>


static unsigned int
packet_loss_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
                 const struct net_device *in, const struct net_device *out,
                 const struct nf_hook_state *state)
{
    return NF_ACCEPT;
}


static unsigned int
packet_corrupt_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
                    const struct net_device *in, const struct net_device *out,
                    const struct nf_hook_state *state)
{
    return NF_ACCEPT;
}


static struct nf_hook_ops nfho_in;
static struct nf_hook_ops nfho_out;


void init_hooks(void)
{
    nfho_in.hook = packet_corrupt_hook;
    nfho_in.hooknum = NF_INET_LOCAL_IN;
    nfho_in.pf = PF_INET;
    nfho_in.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho_in);

    nfho_out.hook = packet_loss_hook;
    nfho_out.hooknum = NF_INET_LOCAL_OUT;
    nfho_out.pf = PF_INET;
    nfho_out.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho_out);
}


void destroy_hooks(void)
{
    nf_unregister_hook(&nfho_in);
    nf_unregister_hook(&nfho_out);
}

