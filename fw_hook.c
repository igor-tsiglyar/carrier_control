#include "fw_hook.h"
#include "fw.h"
#include <linux/netdevice.h>
#include <linux/netfilter_ipv4.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>


static unsigned int
udp_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
                 const struct net_device *in, const struct net_device *out,
                 const struct nf_hook_state *state)
{
    if (skb->protocol == htons(ETH_P_IP)) {
        struct iphdr *ip_header = ip_hdr(skb);

        if (ip_header->version == 4 && ip_header->protocol == IPPROTO_UDP) {
            struct udphdr *udp_header = udp_hdr(skb);

            if (should_forbid_udp_packet(out, udp_header->source)) {
                return NF_DROP;
            }
        }
    }


    return NF_ACCEPT;
}


static unsigned int
tcp_hook(const struct nf_hook_ops *ops, struct sk_buff *skb,
                    const struct net_device *in, const struct net_device *out,
                    const struct nf_hook_state *state)
{
    if (skb->protocol == htons(ETH_P_IP)) {
        struct iphdr *ip_header = ip_hdr(skb);

        if (ip_header->version == 4 && ip_header->protocol == IPPROTO_TCP) {
            struct tcphdr *tcp_header = tcp_hdr(skb);

            if (should_forbid_tcp_packet(in, tcp_header->dest)) {
                return NF_DROP;
            }
        }
    }


    return NF_ACCEPT;
}


static struct nf_hook_ops nfho_in;
static struct nf_hook_ops nfho_out;


void init_hooks(void)
{
    nfho_in.hook = tcp_hook;
    nfho_in.hooknum = NF_INET_LOCAL_IN;
    nfho_in.pf = PF_INET;
    nfho_in.priority = NF_IP_PRI_FIRST;
    nf_register_hook(&nfho_in);

    nfho_out.hook = udp_hook;
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

