#include <pcap/pcap.h>
/* base */
#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
/* netinet*/
#include <netinet/in.h>
#include <netinet/ip_icmp.h>	
#include <netinet/udp.h>	
#include <netinet/tcp.h>
#include <netinet/ip.h>
#include <net/ethernet.h>
#include <arpa/inet.h>
#include <netdb.h>
/* linux*/
#include <linux/if_packet.h>
#include <linux/if_tun.h>
#include <linux/if_ether.h>
#include <linux/if.h>

/*sys*/
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>


#define ETH_HDRLEN 14  // Ethernet header length
#define IP4_HDRLEN 20  // IPv4 header length
#define ICMP_HDRLEN 8  // ICMP header length for echo request, excludes data

static int pcap_try_reopen(pcap_t *handle) {
    int pcap_activate_r = pcap_activate(handle);
    if (pcap_activate_r != 0) {
        return pcap_activate_r;
    }
    return 0;
}

void read_packet(u_char *user, const struct pcap_pkthdr *header, const u_char *packet) {
    struct ethhdr *ethhdr;
    struct ip *iph;
    struct icmp *icmphdr;
    struct tcphdr *tcphdr;

    ethhdr = (struct ethhdr *)packet;
    iph = (struct ip *) (packet + ETH_HDRLEN);
    icmphdr = (struct icmp *) (packet + ETH_HDRLEN + IP4_HDRLEN);
    tcphdr = (struct tcphdr *) (packet + ETH_HDRLEN + IP4_HDRLEN);
    
    if (ethhdr->h_proto == htons(ETH_P_IP)) {
        switch (iph->ip_p) {
            case IPPROTO_ICMP:
                printf("icmp: caplen %d, \n", header->caplen);
                break;
            case IPPROTO_IGMP:
                printf("igmp: caplen %d, \n", header->caplen);
                break;
            case IPPROTO_TCP:
                printf("tcp:, caplen %d,\n", header->caplen);
                printf("tcp dst port %d \n", htons(tcphdr->th_dport));
                break;
            case IPPROTO_UDP:
                printf("udp: caplen %d\n", header->caplen);
                break;
            default:
                break;
	    }
    }
}

int main(int argc, char *argv[]) {
	pcap_t *handle;			/* Session handle */
	char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
	struct bpf_program fp;		/* The compiled filter */
    //focus on: if you use vpn, you can't filter this pcaket
	char filter_exp[] = "tcp dst port 8081";	/* The filter expression */
	bpf_u_int32 mask;		/* Our netmask */
	bpf_u_int32 net;		/* Our IP */
	struct pcap_pkthdr header;	/* The header that pcap gives us */
	const u_char *packet;		/* The actual packet */
    pcap_if_t *devs, *device;
    char *ifname;
    int packet_q_len, rc;

    /* Init var */
    ifname = "ens3";
    packet_q_len = 64;

	/* Define the device */
	if (pcap_findalldevs(&devs, errbuf) < 0) {
        printf("%s\n", errbuf);
        return -1;
    }
    device = devs;
    //find interface
    while(device) {
        if (strlen(ifname) == strlen(device->name)) {
            if(strcmp(ifname, device->name) == 0) {
                break;
            }
        }
        device = device->next;
    }
    if (!device) {
        printf("warnning: nofound interface %s\n", ifname);
        return -1;
    }
    
	/* Find the properties for the device */
	if (pcap_lookupnet(device->name, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n", device->name, errbuf);
		net = 0;
		mask = 0;
	}
	/* Open the session in promiscuous mode */
	handle = pcap_open_live(device->name, BUFSIZ, 1, 1000, errbuf); //call pcap_create()
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", device->name, errbuf);
		return(2);
	}
    /* Compile and apply the filter */
    if (pcap_compile(handle, &fp ,filter_exp , 0, net)  == -1) {
        fprintf(stderr, "Couldn't parse filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
    }

    if (pcap_setfilter(handle, &fp) == -1) {
		fprintf(stderr, "Couldn't install filter %s: %s\n", filter_exp, pcap_geterr(handle));
		return(2);
	}

	/* Grab a packet */
    for (;;) {
        rc = pcap_dispatch(handle, packet_q_len, read_packet, NULL);
        if (rc == 0 || rc == PCAP_ERROR_BREAK) {
            if (rc == PCAP_ERROR_BREAK) {
                return -1;
            }
            //this packet process timeout, you should process it

        }else if (rc < 0) {
            printf("the interface is down, we try to reopen it \n");
            int times = 1;
            //the interface is down, you should reopen it
            do {
                usleep(50000);
                rc = pcap_try_reopen(handle);
                printf("try times, %d\r", times++);
                fflush(NULL);
            } while (rc < 0);
        }
    }

	/* And close the session */
	pcap_close(handle);
	return(0);
}