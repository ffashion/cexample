#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/skbuff.h>
#include <linux/ip.h>
#include <linux/udp.h>
#include <linux/tcp.h>
#include <linux/netfilter.h>
#include <linux/netfilter_ipv4.h>

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ffashion");


unsigned int 
my_hook(void *priv,struct sk_buff *skb,const struct nf_hook_state *state);

static struct 
nf_hook_ops reg[] __read_mostly = {
    {
        .hook =  my_hook, //nf_hookfn  typedef unsigned int nf_hookfn(void *priv, struct sk_buff *skb, const struct nf_hook_state *state);
        .pf = NFPROTO_IPV4,
        .hooknum = NF_IP_LOCAL_IN,
        .priority = NF_IP_PRI_FIRST
    },
};

static int __init netfilter_hook_init(void){
    int ret;
    ret = nf_register_hooks(reg,ARRAY_SIZE(reg));
    if (ret < 0) {
        pr_warn("regiser hook func error");
        return ret;
    }
    pr_info("register hook func  ok");
    return 0;
}
static void __exit netfilter_hook_exit(void){
    nf_unregister_hooks(reg,ARRAY_SIZE(reg));
    return;
}

unsigned int 
my_hook(void *priv,struct sk_buff *skb,const struct nf_hook_state *state){
    

    return NF_ACCEPT; //这个报文继续在内核中传输
}


module_init(netfilter_hook_init);
module_exit(netfilter_hook_exit);
