
// https://www.pdbuchan.com/rawsock/rawsock.html
#include <asm-generic/errno-base.h>
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
char icmp_echo_bytes[] = {
    0x00,0x00, 00 , 00 ,00 ,00 ,00 ,00, 00, 00, 00 ,00 ,0x08 ,00 , 0x45 , 0x00 ,
    00, 0x1c, 0x81, 0x42, 0x40, 00,0x40,0x01,0xbb, 0x9c,  0x7f,  0x00, 0x00,0x01, 
    0x7f, 0x00 ,0x00 , 0x01,0x08, 00,0xf7 , 0xff,  0x00 , 0x00 ,0x00 , 0x00
};



char *setup_ethhdr(char *buf);
char *setup_iphdr(char *buf);
char * setup_icmphdr(char *buf);

#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data

u_int16_t checksum(unsigned short *buf, int size) {
	unsigned long sum = 0;
	while (size > 1) {
		sum += *buf;
		buf++;
		size -= 2;
	}
	if (size == 1)
		sum += *(unsigned char *)buf;
	sum = (sum & 0xffff) + (sum >> 16);
	sum = (sum & 0xffff) + (sum >> 16);
	return ~sum;
}

// struct ifreq 
// sockaddr_ll
int make_raw_socket(int protocol_to_sniff) {
    int fd;
	if((fd = socket(PF_PACKET, SOCK_RAW, htons(protocol_to_sniff)))== -1) {
		perror("Error creating raw socket: ");
		exit(-1);
	}
	return fd;
}


int bind_raw_socket_to_interface(char *device, int rawsock, int protocol)
{
	struct sockaddr_ll sll;
	struct ifreq ifr;

	memset(&sll, 0, sizeof(sll));
	memset(&ifr, 0, sizeof(ifr));
	/* First Get the Interface Index  */
	strncpy((char *)ifr.ifr_name, device, IFNAMSIZ);
	if((ioctl(rawsock, SIOCGIFINDEX, &ifr)) == -1) {
		printf("Error getting Interface index !\n");
		exit(-1);
	}
	/* Bind our raw socket to this interface */
	sll.sll_family = AF_PACKET;
	sll.sll_ifindex = ifr.ifr_ifindex;
	sll.sll_protocol = htons(protocol);
	if((bind(rawsock, (struct sockaddr *)&sll, sizeof(sll)))== -1) {
		perror("Error binding raw socket to interface\n");
		exit(-1);
	}
	return 1;
}
int send_raw_packet(int rawsock, char *pkt, int pkt_len)
{
	int sent= 0;
 	/* A simple write on the socket ..thats all it takes ! */
 	if((sent = write(rawsock, pkt, pkt_len)) != pkt_len) {
		return 0;
	}
	return 1;
}


int send_ping_echo(int fd, char *packet) {
    char *buff;

    buff = setup_ethhdr(packet);
    buff = setup_iphdr(buff);
    buff = setup_icmphdr(buff);

    if (send_raw_packet(fd, packet, buff-packet) < 0) {
        perror("Error sending packet");
    }
    return 0;
}


char *setup_ethhdr(char *buf) {
    static char h_source[ETH_ALEN]  = {0x52, 0x54, 0x00, 0x05, 0x58, 0xc3};
    static char h_dest[ETH_ALEN]  = {0xff, 0xff ,0xff ,0xff , 0xff ,0xff};
    

    // static char h_source[ETH_ALEN]  = {0x00};
    // static char h_dest[ETH_ALEN]  = {0x00};


    struct ethhdr *ethhdr;
    ethhdr = (struct ethhdr *)buf;
    memcpy(ethhdr->h_source, h_source, ETH_ALEN);
    memcpy(ethhdr->h_dest, h_dest, ETH_ALEN);
    ethhdr->h_proto = htons(ETH_P_IP);
    
    
    return buf + sizeof(struct ethhdr);
}

char *setup_iphdr(char *buf) {
    struct iphdr *iphdr;
    iphdr = (struct iphdr *) buf;
    // 4bits version
    iphdr->version = 4;
    
    //4 bits ip header len
    iphdr->ihl = sizeof(struct iphdr) / sizeof(u_int32_t); //if no ip options , then equals 5

    // 8 bits tos
    iphdr->tos = 0;

    //16 bits total length
    iphdr->tot_len = htons (IP4_HDRLEN + ICMP_HDRLEN);

    //16 bits identification 
    iphdr->id = htons(getpid());
    


    //3 bits flags + 13bits fragment offset
    iphdr->frag_off  = 0; //all zero

    // 8 bits ttl
    iphdr->ttl = 64;

    // 8bits protocol
    iphdr->protocol = IPPROTO_ICMP;

    // 16bits checksum
    iphdr->check = 0;

    // 32bits saddr
    iphdr->saddr = inet_addr("10.10.22.65");

    // 32bits daddr
    iphdr->daddr = inet_addr("10.10.22.60");


    //reset checksum
    iphdr->check = checksum((unsigned short *)iphdr, sizeof(struct iphdr));
    
    return buf + sizeof(struct iphdr);

}

char *setup_icmphdr(char *buf) {
    struct icmphdr *icmphdr;
    icmphdr = (struct icmphdr *) buf;

    icmphdr->type = ICMP_ECHO;
    icmphdr->code = 0;
    //set zero, must set it
    icmphdr->checksum = 0;
    
    icmphdr->un.echo.id = htons(0);
    icmphdr->un.echo.sequence = htons(0);


    //reset checksum
    icmphdr->checksum = checksum((unsigned short *)icmphdr, sizeof(struct icmphdr));
    
    
    return buf + sizeof(struct icmphdr);
}

int	main(int argc, char **argv) {
    int fd, n, err;
    char *interface = "ens3", rec_ip[INET_ADDRSTRLEN];
    struct ifreq 		ifr;
    struct sockaddr_ll	device;
    char packet[1024];
    char recv_ether_frame[IP_MAXPACKET];
    struct ethhdr *recv_ethhdr;
    struct ip *recv_iphdr;
    struct icmp *recv_icmphdr;

    fd = make_raw_socket(ETH_P_ALL);
    if (fd < 0) {
        perror("make socket");
    }
    bind_raw_socket_to_interface(interface, fd, ETH_P_ALL);
#if 0
    send_raw_packet(fd, icmp_echo_bytes, sizeof(icmp_echo_bytes));
#endif

    send_ping_echo(fd, packet);

    recv_ethhdr = ( struct ethhdr *)recv_ether_frame;
    recv_iphdr = (struct ip *) (recv_ether_frame + ETH_HDRLEN);
    recv_icmphdr = (struct icmp *) (recv_ether_frame + ETH_HDRLEN + IP4_HDRLEN);

    for(;;) {
        int flag = 0;
        
        n = recv(fd, recv_ether_frame, sizeof(recv_ether_frame), 0);
        if (n < 0) {
            err = errno;
            if (err == EAGAIN) {
                printf("no replay\n");
                continue;
            } else if (err == EINTR) {
                continue;// Something weird happened, but let's keep listening.
            } else {
                perror ("recvfrom() failed ");
                exit (EXIT_FAILURE);
            }
        }
        
        if ((recv_ethhdr->h_proto == htons(ETH_P_IP)) && (recv_iphdr->ip_p == IPPROTO_ICMP) && 
        (recv_icmphdr->icmp_type == ICMP_ECHOREPLY) && (recv_icmphdr->icmp_code == 0)) {
            inet_ntop (AF_INET, &(recv_iphdr->ip_src.s_addr), rec_ip, INET_ADDRSTRLEN);
            printf ("recived from %s and %d bytes received\n", rec_ip, n);
            break;
        }
        
    }
   



    return 0;
}
