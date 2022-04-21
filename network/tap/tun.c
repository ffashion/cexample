// 3 layer 
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

#define IP4_HDRLEN 20  // IPv4 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data



void debug_output_mac(unsigned char *addr) {
  
    fprintf(stderr, "%02x:%02x:%02x:%02x:%02x:%02x\n",addr[0],
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
    strcpy(ifr.ifr_name, "tun0");

    if (ioctl(fd, TUNSETIFF, &ifr)  < 0) {
        close(fd);
        return -1;
    }

    return fd;
}
// step 1: sudo ifconfig tun0 192.168.1.2/24
// step 2: ping or curl the other ips, but 192.168.1.2 , example 192.168.1.3

int	main(int argc, char **argv) {
    int fd, n;
    char buf[48];
    struct ethhdr *recv_ethhdr;
    struct ip *recv_iphdr;
    struct icmp *recv_icmphdr;

    fd = tap_alloc(IFF_TUN | IFF_NO_PI);
    if(fd < 0) {
        perror("tap alloc");
        return -1;;
    }
    recv_iphdr = (struct ip *) buf;
    recv_icmphdr = (struct icmp *) (buf + IP4_HDRLEN);

    //set ip address 
    system("sudo ifconfig tun0 192.168.1.2/24");
    for (;;) {
        n = read(fd, buf, sizeof(buf));

        printf("src ip: %s\n", inet_ntoa(recv_iphdr->ip_src));
        printf("dst ip: %s\n", inet_ntoa(recv_iphdr->ip_dst));
    }
    close(fd);
    return 0;
}
