#include "cc_hook.h"
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>


unsigned int hook_func_out(const struct nf_hook_ops *ops, struct sk_buff *skb,
                           const struct net_device *in, const struct net_device *out,
                           const struct nf_hook_state *state)
{
    return NF_DROP;
}


unsigned int hook_func_in(const struct nf_hook_ops *ops, struct sk_buff *skb,
                          const struct net_device *in, const struct net_device *out,
                          const struct nf_hook_state *state)
{
    return NF_DROP;
}


unsigned int hook_func_forward(const struct nf_hook_ops *ops, struct sk_buff *skb,
                               const struct net_device *in, const struct net_device *out,
                               const struct nf_hook_state *state)
{
    return NF_DROP;
}


static struct nf_hook_ops nfho_forward;
static struct nf_hook_ops nfho_in;
static struct nf_hook_ops nfho_out;


void init_hooks(void)
{
    nfho_forward.hook = hook_func_forward;
    nfho_forward.hooknum = NF_INET_FORWARD;
    nfho_forward.pf = PF_INET;
    nfho_forward.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho_forward);

    nfho_in.hook = hook_func_in;
    nfho_in.hooknum = NF_INET_LOCAL_IN;
    nfho_in.pf = PF_INET;
    nfho_in.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho_in);

    nfho_out.hook = hook_func_out;
    nfho_out.hooknum = NF_INET_LOCAL_OUT;
    nfho_out.pf = PF_INET;
    nfho_out.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho_out);
}


void destroy_hooks(void)
{
    nf_unregister_hook(&nfho_in);
    nf_unregister_hook(&nfho_out);
    nf_unregister_hook(&nfho_forward);
}



