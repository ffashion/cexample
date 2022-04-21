//2 layer 
#include <linux/if.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/ethernet.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <fcntl.h>

#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data

void debug_output_mac(char *you, unsigned char *addr) {
  
    fprintf(stderr, "[%s] %02x:%02x:%02x:%02x:%02x:%02x\n",you,addr[0],
          addr[1], addr[2], addr[3], addr[4], addr[5]);
}



void hexToString(unsigned char *hex, char *str, int len, int output) {
  if (str == NULL) {
    str = malloc(2 * len) ;
  }
  for (size_t i = 0; i < len; i++) {
    sprintf(str + i * 2, "%02x", hex[i]);
  }
  if (output) {
    printf("%s\n", str);
  }
  free(str);
  return;
}

int tap_alloc(int flag) {
    struct ifreq ifr;
    int fd ;
    fd = open("/dev/net/tun", O_RDWR); //必须是这个位置
    if (fd < 0) {
        return -1;
    }
    memset(&ifr, 0, sizeof(struct ifreq));
    ifr.ifr_flags = flag;
    strcpy(ifr.ifr_name, "tap0");

    if (ioctl(fd, TUNSETIFF, &ifr)  < 0) {
        close(fd);
        return -1;
    }

    return fd;
}
// step 1: sudo ifconfig tap0 192.168.1.2/24
// step 2: ping or curl the other ips, example 192.168.1.3

int	main(int argc, char **argv) {
    int fd, n;
    char buf[1024];
    struct ethhdr *recv_ethhdr;
    struct ip *recv_iphdr;
    struct icmp *recv_icmphdr;
    //set TAP Flag, get IP data
    fd = tap_alloc(IFF_TAP | IFF_NO_PI); //no have packet infomation
    if(fd < 0) {
        perror("tap alloc");
        return -1;;
    }
    recv_ethhdr = (struct ethhdr *)buf;
    recv_iphdr = (struct ip *) (buf + ETH_HDRLEN);
    recv_icmphdr = (struct icmp *) (buf + ETH_HDRLEN + IP4_HDRLEN);

    //set ip address 
    system("sudo ifconfig tap0 192.168.1.2/24");
    for (;;) {
        //(1) user choose a ip to send data, his ip and the ip on the tap0 in the same subnet.
        //(2) user send's data which we have captured
        //(3) we can create a socket to send (1)and(2)' data, now. so we create a vpn, now.
        n = read(fd, buf, sizeof(buf)); //when some people use tap0 interface(listen, accept, connect), this have some data.
        
        debug_output_mac("source mac", recv_ethhdr->h_source);
        debug_output_mac("dest mac:", recv_ethhdr->h_dest);
        if(recv_ethhdr->h_proto == htons(ETH_P_IP)) {
            printf("this is a ip packet\n");
            printf("src ip: %s\n", inet_ntoa(recv_iphdr->ip_src));
            printf("dst ip: %s\n", inet_ntoa(recv_iphdr->ip_dst));
        }
    }
    close(fd);
    return 0;
}
