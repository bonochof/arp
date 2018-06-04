#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <net/ethernet.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include "../librawsocket.h"
#include "../libnet.h"

/**
 * send arp reply
 * @param soc raw socket
 * @param target_ip
 * @param target_mac
 * @param source_ip
 * @param source_mac
 * 
 * @return true if sending arp request is success, otherwise false.
 **/
bool send_arp_reply(int soc, in_addr_t target_ip, u_char target_mac[6], in_addr_t source_ip, u_char source_mac[6])
{
	struct ether_header eh;
	struct ether_arp arp;
	union {
		unsigned long l;
		u_char c[4];
	} ip_conv;

	// create ether header
	for (int i = 0; i < 6; i++)
	{
		// set source/target mac address
		eh.ether_dhost[i] = target_mac[i];
		eh.ether_shost[i] = source_mac[i];
	}
	eh.ether_type = htons(ETHERTYPE_ARP);

	// create arp request packet
	arp.arp_hrd = htons(ARPHRD_ETHER);
	arp.arp_pro = htons(ETHERTYPE_IP);
	arp.arp_hln = 6;
	arp.arp_pln = 4;
	arp.arp_op = htons(ARPOP_REPLY);

	for (int i = 0; i < 6; i++)
	{
		// set source/target mac address
		arp.arp_sha[i] = source_mac[i];
		arp.arp_tha[i] = target_mac[i];
	}

	ip_conv.l = source_ip;
	for (int i = 0; i < 4; i++)
	{
		// set source ip address
		arp.arp_spa[i] = ip_conv.c[i];
	}
	ip_conv.l = target_ip;
	for (int i = 0; i < 4; i++)
	{
		// set target ip address
		arp.arp_tpa[i] = ip_conv.c[i];
	}

	// send arp request packet
	u_char buf[sizeof(struct ether_header) + sizeof(struct ether_arp)];
	memset(buf, 0, sizeof(buf));
	u_char *p = buf;
	memcpy(p, &eh, sizeof(struct ether_header));
	p = p + sizeof(struct ether_header);
	memcpy(p, &arp, sizeof(struct ether_arp));
	p = p + sizeof(struct ether_arp);
	int size = p - buf;
	write(soc, buf, size);
	return true;
}

int main(int argc, char *argv[])
{
	int soc = -1;

	if (argc <= 8)
	{
		fprintf(stderr, "Usage: arpreply device_name ip_addr mac1 mac2 mac3 mac4 mac5 mac6\n");
		return (1);
	}

	// initialize raw socket
	if ((soc = init_raw_socket(argv[1])) == -1)
	{
		fprintf(stderr, "ERROR: cannot initialize device: %s\n", argv[1]);
		return (-1);
	}

	// send arp reply
	in_addr_t source_ip;
	get_device_ip_address(soc, argv[1], &source_ip);
	in_addr_t target_ip;
	get_ip_address(argv[2], &target_ip);
	u_char source_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	get_device_mac_address(soc, argv[1], source_mac);
	u_char target_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
	for ( int i=0; i<6; i++ ) {
		target_mac[i] = strtol(argv[3+i], NULL, 16);
	}
	send_arp_reply(soc, target_ip, target_mac, source_ip, source_mac);

	close(soc);
	return (0);
}

