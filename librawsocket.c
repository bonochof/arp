#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <linux/if.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include "librawsocket.h"

/**
 * Initialize RawSocket
 *
 * @param device network device interface name (e.g. eth0)
 * 
 * @return socket number, or -1 if the initialization is failed.
 */
int init_raw_socket(char *device)
{
	struct ifreq ifreq;
	int soc = -1;

	// create socket with RAW type
	if ((soc = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL))) < 0)
	{
		perror("socket");
		return (-1);
	}

	// check existance of the device by getting device index
	memset(&ifreq, 0, sizeof(struct ifreq));
	strncpy(ifreq.ifr_name, device, sizeof(ifreq.ifr_name) - 1);
	if (ioctl(soc, SIOCGIFINDEX, &ifreq) < 0)
	{
		perror("ioctl");
		close(soc);
		return (-1);
	}

	// set promiscuous mode
	struct packet_mreq mreq;
	memset(&mreq, 0, sizeof(mreq));
	mreq.mr_type = PACKET_MR_PROMISC;
	mreq.mr_ifindex = ifreq.ifr_ifindex;
	if ((setsockopt(soc, SOL_PACKET, PACKET_ADD_MEMBERSHIP, (void *)&mreq, sizeof(mreq))) < 0)
	{
		perror("setsockopt");
		return (-1);
	}

	// bind socket
	struct sockaddr_ll sa;
	sa.sll_family = PF_PACKET;
	sa.sll_protocol = htons(ETH_P_ALL);
	sa.sll_ifindex = ifreq.ifr_ifindex;
	if (bind(soc, (struct sockaddr *)&sa, sizeof(sa)) < 0)
	{
		perror("bind");
		close(soc);
		return (-1);
	}
	return (soc);
}

