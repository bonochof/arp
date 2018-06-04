#include <stdio.h>
#include <string.h>
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
#include "libnet.h"

bool get_ip_address(char* addr, in_addr_t* res)
{
	if (inet_pton(PF_INET, addr, res) != 1)
	{
		perror("inet_pton");
		return false;
	}
	return true;
}

bool get_device_ip_address(int soc, char* device, in_addr_t* res)
{
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = PF_INET;
	strncpy(ifr.ifr_name, device, IFNAMSIZ - 1);

	// get ip address for the network interface
	if ( ioctl(soc, SIOCGIFADDR, &ifr) == -1 ) {
		perror("ioctl: SIOCGIFADDR");
		return false;
	}
	*res = ((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr.s_addr;
	// printf("%s\n", inet_ntoa(((struct sockaddr_in *)&(ifr.ifr_addr))->sin_addr));
	return true;
}

// get mac address for the network interface
bool get_device_mac_address(int soc, char* device, u_char res[6])
{
	struct ifreq ifr;
	ifr.ifr_addr.sa_family = PF_INET;
	strncpy(ifr.ifr_name, device, IFNAMSIZ - 1);
	if ( ioctl(soc, SIOCGIFHWADDR, &ifr) == -1 ) {
		perror("ioctl: SIOCGIFHWADDR");
		return false;
	}
	for (int i = 0; i < 6; i++)
	{
		res[i] = ifr.ifr_hwaddr.sa_data[i];
	}
	return true;
}
