#include <linux/init.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/errno.h>
#include <linux/netdevice.h>
#include <linux/etherdevice.h>
#include <linux/kernel.h>
#include <linux/types.h>
#include <linux/fcntl.h>
#include <linux/interrupt.h>
#include <linux/ioport.h>
#include <linux/in.h>
#include <linux/skbuff.h>
#include <linux/slab.h>
#include <linux/spinlock.h>
#include <linux/string.h>
#include <linux/init.h>
#include <linux/bitops.h>
#include <linux/delay.h>
#include <linux/ip.h>
#include <asm/io.h>
#include <asm/irq.h>
#include <asm/dma.h>
//sudo ifconfig vdev0  3.3.3.3
static struct net_device *vdev;
static unsigned char dev_addr[ETH_ALEN] = {
  0x08, 0x89, 0x89, 0x89, 0x89, 0x11
};


static void 
ping_replay_rx_packet(struct sk_buff *skb,struct net_device *dev);

static netdev_tx_t
loopback_start_xmit(struct sk_buff *skb, struct net_device *dev);

static void 
vdev_setup(struct net_device *dev);

int swap_n(char *p, char *q, int n);

static const struct net_device_ops vdev_netdev_ops = {
    .ndo_start_xmit = loopback_start_xmit
};

int swap_n(char *p, char *q, int n) {

    char *__tmp = (char *)kmalloc(n, GFP_KERNEL);
    if (__tmp == NULL) {
        return -1;
    }
    memcpy(__tmp, q, n);
    memcpy(q, p, n);
    memcpy(p, __tmp, n);

    kfree(__tmp);
    return 0;
}

static void vdev_setup(struct net_device *dev) {
    ether_setup(dev);
}
static void 
ping_replay_rx_packet(struct sk_buff *skb,struct net_device *dev) {

    unsigned char *type;
    struct iphdr  *ih;
    struct ethhdr *ethhdr;
    struct sk_buff *rx_skb;

    //build a skb
    rx_skb = dev_alloc_skb(skb->len + 2);

    ethhdr = (struct ethhdr *)skb->data;

    //set mac layer 
    swap_n(ethhdr->h_dest, ethhdr->h_source, ETH_ALEN);

    //set ip later 

    ih = (struct iphdr *)(skb->data + sizeof(struct ethhdr));

    swap(ih->saddr, ih->daddr);
    
    //set icmp header 
    type = skb->data + sizeof(struct ethhdr) + sizeof(struct iphdr);
    //修改类型，原来0x8表示ping
    *type = 0;        /* 0表示reply */


    ih->check = 0;     /* and rebuild the checksum (ip need it) */
    ih->check = ip_fast_csum((unsigned char *)ih,ih->ihl);
  
    
    skb_reserve(rx_skb, 2);  // for kernel protocal stack 
    

    memcpy(skb_put(rx_skb, skb->len), skb->data,skb->len);/*使用memcpy()将之前修改好的sk_buff->data复制到新的sk_buff里*/
    // skb_put():来动态扩大sk_buff结构体里中的数据区，避免溢出
    
    rx_skb->dev = dev;
    rx_skb->protocol = eth_type_trans(rx_skb, dev);
    rx_skb->ip_summed = CHECKSUM_UNNECESSARY;        /* don't check it */
    
      /* 更新接收统计信息,并使用netif_rx( )来 传递sk_fuffer收包 */
    dev->stats.rx_packets++;                     
    dev->stats.rx_bytes += skb->len;
    // dev->last_rx= jiffies;                       //收包时间戳
  
    netif_rx(rx_skb); //通知内核接收数据了，数据已经准备好了。
 
}

//send data
static netdev_tx_t
loopback_start_xmit(struct sk_buff *skb, struct net_device *dev) {
    // the praprm skb is input skb. we should free it.

    static int count;
    printk("vdev_start_xmit count %d\n", ++count);

    netif_stop_queue(dev); //让协议栈不要再发送数据了, 也就是停止调用这个函数

    //get a skb from hardware, but this fake package created by software
    
    ping_replay_rx_packet(skb, dev);
  
    dev_kfree_skb(skb);

    dev->stats.tx_packets++;
    dev->stats.tx_bytes += skb->len;

    netif_wake_queue(dev); //让协议栈继续发送数据，也就是让协议栈继续调用这个函数
    
    return NETDEV_TX_OK;
}


static int loopback_init_module(void){

    vdev = alloc_netdev(0, "vdev0", NET_NAME_UNKNOWN, vdev_setup);
    if (!vdev)
		return -ENOMEM;
    
    vdev->netdev_ops = &vdev_netdev_ops;
    
    memcpy(vdev->dev_addr, dev_addr, sizeof(dev_addr));

    vdev->flags    |= IFF_NOARP;
    // vdev->features |= NETIF_F_NO_CSUM;
  
    register_netdev(vdev);
    
    printk("net_vdev_init_module\n");
    
    return 0;
}
static void loopback_exit_module(void){
    printk("net_vdev_exit_module\n");
    
    unregister_netdev(vdev);
    free_netdev(vdev);
    return;
}


module_init(loopback_init_module);

module_exit(loopback_exit_module);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ffashion");

