#include "cc_hook.h"
#include "cc.h"
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>


static unsigned int
packet_loss_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
                 const struct net_device *in, const struct net_device *out,
                 const struct nf_hook_state *state)
{
    return should_drop_packet(out) ? NF_DROP : NF_ACCEPT;
}


static unsigned int
packet_corrupt_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
                    const struct net_device *in, const struct net_device *out,
                    const struct nf_hook_state *state)
{
    struct iphdr *ip_header = (struct iphdr *) skb_network_header(skb);

    if (ip_header->protocol == 17 && skb_make_writable(skb, skb->len)) {
        int byte_idx, bit_idx;

        for (byte_idx = 0; byte_idx < skb->len; ++byte_idx) {
            for (bit_idx = 0; bit_idx < 8; ++bit_idx) {
                if (should_corrupt_bit(in)) {
                    skb->data[byte_idx] ^= 1 << bit_idx;
                }
            }
        }
    }

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

