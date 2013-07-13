#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>

#ifdef __linux__
        #include <linux/if_tun.h>
        #define __IOCTL_OPERATION SIOCAIFADDR

        struct in6_ifreq
        {
            struct in6_addr ifr6_addr;
            uint32_t ifr6_prefixlen;
            unsigned int ifr6_ifindex;
        };
#else
        #include <netinet6/in6_var.h>
        #define __IOCTL_OPERATION SIOCAIFADDR_IN6
#endif

#include "main.h"
#include "node.h"
#include "bucket.h"
#include "message.h"

underlink_node buckets[ADDR_LEN][NODES_PER_BUCKET];
underlink_node thisNode;

int sockfd, tuntapfd;

underlink_pubkey localPublicKey;
underlink_seckey localSecretKey;

int max(int a, int b)
{
	return a > b ? a : b;
}

int main(int argc, char* argv[])
{
	int portnumber = 3456;
	int opt;
	char nodename[16];
	fd_set selectlist;
	
	memset(&buckets, 0, sizeof(underlink_node) * ADDR_LEN * NODES_PER_BUCKET);
	memset(&thisNode, 0, sizeof(underlink_node));
	
	while ((opt = getopt(argc, argv, "p:")) != -1)
	{
		switch (opt)
		{
			case 'p':
				if (atoi(optarg) <= 65535)
					portnumber = atoi(optarg);
				break;
				
			default:
				fprintf(stderr, "Usage: %s [-p port]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	if (debug)
	{
		printf("Using UDP port %i\n", portnumber);
		printf("Maximum peer count: %i\n", ADDR_LEN * NODES_PER_BUCKET);
		printf("%i-bit peer address space\n", ADDR_LEN);
		printf("Bucket table size in memory: %lukb\n",
			(sizeof(underlink_node) * ADDR_LEN * NODES_PER_BUCKET) / 24);
	}

	srand(time(NULL));
	
	int i;
	for (i = 0; i < 15; i ++)
	{
		underlink_node n;
		underlink_pubkey pk;
		underlink_seckey sk;
		
		key_generate(pk, sk);
		n.endpoint.sin_family = AF_INET;
		inet_pton(AF_INET, "127.0.0.1", &n.endpoint.sin_addr);
		n.endpoint.sin_port = htons(3456);
		n.routermode = ROUTER;
		proto_init(n.crypto);
		addNodeToBuckets(n);
	}
	
	crypto_box_curve25519xsalsa20poly1305_keypair(localPublicKey, localSecretKey);
	thisNode.endpoint.sin_family = AF_INET;
	thisNode.routermode = DIRECT_ONLY;
	inet_pton(AF_INET, "127.0.0.1", &thisNode.endpoint.sin_addr);
	thisNode.endpoint.sin_port = htons(portnumber);
	addNodeToBuckets(thisNode);
	
	//printf("My Node ID: 0x%08llX\n", thisNode.key);
	
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (sockfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if (bind(sockfd, (struct sockaddr*) &thisNode.endpoint, sizeof(struct sockaddr_in)) < 0)
	{
		perror("bind");
		exit(EXIT_FAILURE);
	}
	
	if (nodename)
		strcpy(nodename, "/dev/tun0");
		
	char prefix[128];
	uint64_t network = htonll(0xFD974C4E9D261F01);
	memcpy((void*) &prefix, &network, ADDR_LEN);
	memcpy((void*) &prefix + 8, (void*) &thisNode.key, ADDR_LEN);
	
	char presentational[128];
	inet_ntop(AF_INET6, &prefix, &presentational, 128);
	printf("Interface prefix: %s/64\n", presentational);

	#ifdef __linux__	
		if ((tuntapfd = open("/dev/net/tun", O_RDWR)) < 0)
		{
			fprintf(stderr, "Unable to find /dev/net/tun\n");
			return -1;
		}
	#else
		if ((tuntapfd = open(nodename, O_RDWR)) < 0)
		{
			fprintf(stderr, "Unable to open tuntap device '%s'\n", nodename);
			return -1;
		}
	#endif
	
	struct ifreq ifr;
	ioctl(sockfd, SIOCSIFADDR, &ifr);
	ioctl(sockfd, SIOCGIFFLAGS, &ifr);
	ifr.ifr_flags &= ~IFF_MULTICAST;
	ioctl(tuntapfd, SIOCSIFFLAGS, &ifr);

	struct in6_aliasreq addreq6;
	memset(&addreq6, 0, sizeof(addreq6));
	sprintf(addreq6.ifra_name, "tun0");

	addreq6.ifra_addr.sin6_family = AF_INET6;
    addreq6.ifra_addr.sin6_len = sizeof(struct sockaddr_in6);
    memcpy(&addreq6.ifra_addr.sin6_addr, prefix, sizeof(struct in6_addr));

    addreq6.ifra_prefixmask.sin6_family = AF_INET6;
    addreq6.ifra_prefixmask.sin6_len = sizeof(struct sockaddr_in6);
    memset(&addreq6.ifra_prefixmask.sin6_addr, 0xFF, sizeof(addreq6.ifra_prefixmask.sin6_addr) / 4);
    
    addreq6.ifra_lifetime.ia6t_pltime = 0xFFFFFFFFL;
    addreq6.ifra_lifetime.ia6t_vltime = 0xFFFFFFFFL;

	int sockfd6 = socket(AF_INET6, SOCK_DGRAM, 0);
	if (sockfd6 < 0)
		perror("socket(AF_INET6)");

	if (ioctl(sockfd6, __IOCTL_OPERATION, &addreq6) == -1)
		perror("SIOCAIFADDR_IN6");
	
	while (1)
	{
		FD_ZERO(&selectlist);
		FD_SET(tuntapfd, &selectlist);
		FD_SET(sockfd, &selectlist);
		
		int nfds = max(tuntapfd, sockfd);
		nfds ++;
		
		int len = select(nfds, &selectlist, NULL, NULL, 0);

		if (len < 0)
		{
			perror("select");
			return -1;
		}
		
		if (FD_ISSET(tuntapfd, &selectlist) != 0)
		{
			char buffer[MTU];
			struct ip6_hdr* headers = (struct ip6_hdr*) &buffer;
			
			long readvalue = read(tuntapfd, &buffer, MTU);
			
			if (readvalue < 0)
			{
				perror("read");
				return -1;
			}
			
			struct in6_addr* src_addr = &headers->ip6_src;
			struct in6_addr* dst_addr = &headers->ip6_dst;
			
			if (dst_addr->s6_addr[0] == 0xFF) continue;
			if (memcmp(&src_addr->s6_addr, &network, sizeof(network)) != 0) continue;
			if (memcmp(&dst_addr->s6_addr, &network, sizeof(network)) != 0) continue;
		
			struct underlink_node source, destination;
			memcpy((void*) &source.key, (void*) &src_addr->s6_addr + 8, sizeof(char) * 8);
			memcpy((void*) &destination.key, (void*) &dst_addr->s6_addr + 8, sizeof(char) * 8);
			
			if (source.key != thisNode.key)
			{
			//	fprintf(stderr, "Packet discarded by filter: invalid source node ID 0x%08llX\n", source.key);
				continue;
			}
							
			sendIPPacket(buffer, readvalue, source, destination, headers);
		}
		
		if (FD_ISSET(sockfd, &selectlist) != 0)
		{
			char buffer[MTU];
			struct underlink_message* message = (underlink_message*) &buffer;
			
			long readvalue = read(sockfd, &buffer, MTU);
			
			if (message->remoteID == thisNode.key)
			{
				if (write(tuntapfd, message->packetbuffer, message->payloadsize) < 0)
				{
					fprintf(stderr, "TUN/TAP error when attempting to write to adapter: ");
					perror("write");
					fprintf(stderr, "\n");
				}
			}
				else
			{
				if (thisNode.routermode == ROUTER || message->localID == thisNode.key)
					sendIPPacket(message->packetbuffer, message->payloadsize, message->remoteID, message->localID, 0);
				//else
				//	fprintf(stderr, "Packet discarded: illegal attempt to route for node %llu\n", message->remoteID);
			}
		}
	}
}

int sendIPPacket(char buffer[MTU], long length, underlink_node source, underlink_node destination, struct ip6_hdr* headers)
{		
	underlink_node closest;
	memset(&closest, 0, sizeof(char) * 16);

	closest = getClosestAddressFromBuckets(destination, 0, ROUTER);

	if (closest.key == 0)
	{
		//fprintf(stderr, "Remote node %llu is not accessible; no intermediate router known\n", destination.key);
		return -1;
	}

	if (closest.endpoint.sin_addr.s_addr == 0)
	{
		//fprintf(stderr, "Packet discarded: node %llu has no remote endpoint\n", closest.key);
		return -1;
	}
		
	struct underlink_message* msg = underlink_message_construct(IPPACKET, thisNode.key, closest.key, length);
	char* sendbuffer = calloc(1, MTU);
	memcpy(&msg->packetbuffer, &buffer, length);
	int sendsize = underlink_message_pack(sendbuffer, msg);
	
	//if (debug)
	//	printf("Sending %lu bytes to node 0x%08llX via 0x%08llX\n", length, htonll(destination.key), htonll(closest.key));

	if (sendto(sockfd, sendbuffer, sendsize, 0, (struct sockaddr*) &closest.endpoint, sizeof(closest.endpoint)) == -1)
	{
		//fprintf(stderr, "Socket error when attempting to send to %llu: ", closest.key);
		perror("sendto");
		//fprintf(stderr, "\n");
	}
	
	free(sendbuffer);
	free(msg);
}
