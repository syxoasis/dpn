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
        #define __IOCTL_OPERATION SIOCSIFADDR

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
#include "proto.h"

underlink_node buckets[sizeof(underlink_nodeID) * 8][NODES_PER_BUCKET];
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
	
	memset(&buckets, 0, sizeof(underlink_node) * sizeof(underlink_nodeID) * NODES_PER_BUCKET);
	memset(&thisNode, 0, sizeof(underlink_node));
	
	generateKey(&localPublicKey, &localSecretKey);
	getNodeIDFromKey(&thisNode.nodeID, localPublicKey);
		
	thisNode.endpoint.sin_family = AF_INET;
	inet_pton(AF_INET, "0.0.0.0", &thisNode.endpoint.sin_addr);
	thisNode.endpoint.sin_port = htons(portnumber);
	
	while ((opt = getopt(argc, argv, "p:odr")) != -1)
	{
		switch (opt)
		{
			case 'p':
				if (atoi(optarg) <= 65535)
				{
					portnumber = atoi(optarg);
					thisNode.endpoint.sin_port = htons(portnumber);
				}
				break;
				
			default:
				fprintf(stderr, "Usage: %s [-p port] [-o] [-d] [-r]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	addNodeToBuckets(thisNode);
	
	if (debug)
	{
		printf("Using UDP port %i\n", portnumber);
		printf("Maximum peer count: %lu\n", sizeof(underlink_nodeID) * NODES_PER_BUCKET);
		printf("%lu-bit peer address space\n", sizeof(underlink_nodeID) * 8);
		printf("Bucket table size in memory: %lukb\n",
			(sizeof(underlink_node) * sizeof(underlink_nodeID) * NODES_PER_BUCKET) / 24);
	}
	
	srand(time(NULL));
	
	int i;
	for (i = 0; i < 10; i ++)
	{
		underlink_node n;
		underlink_pubkey pk;
		underlink_seckey sk;
		
		generateKey(&pk, &sk);
		getNodeIDFromKey(&n.nodeID, pk);
		
		n.endpoint.sin_family = AF_INET;
		inet_pton(AF_INET, "127.0.0.1", &n.endpoint.sin_addr);
		n.endpoint.sin_port = htons(3456);
		proto_init(n.crypto);
		addNodeToBuckets(n);
	}
	
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
	
	inet_pton(AF_INET, "127.0.0.1", &thisNode.endpoint.sin_addr);
	
	if (nodename)
		strcpy(nodename, "/dev/tun0");
		
	char prefix[128];
	memcpy((void*) &prefix, (void*) &thisNode.nodeID, sizeof(underlink_nodeID));
	
	printf("Interface prefix: ");
	printNodeIPAddress(stdout, &thisNode.nodeID);
	printf("/8\n");

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
		
	#ifdef __linux__
		fprintf(stderr, "Please set interface prefix manually using ip -6 addr add\n");
	#else
		struct in6_aliasreq addreq6;
		memset(&addreq6, 0, sizeof(addreq6));
		sprintf(addreq6.ifra_name, "tun0");

		addreq6.ifra_addr.sin6_family = AF_INET6;
		addreq6.ifra_addr.sin6_len = sizeof(struct sockaddr_in6);
		memcpy(&addreq6.ifra_addr.sin6_addr, prefix, sizeof(struct in6_addr));

		addreq6.ifra_prefixmask.sin6_family = AF_INET6;
		addreq6.ifra_prefixmask.sin6_len = sizeof(struct sockaddr_in6);
		memset(&addreq6.ifra_prefixmask.sin6_addr, 0xFF, 1);
		
		addreq6.ifra_lifetime.ia6t_pltime = 0xFFFFFFFFL;
		addreq6.ifra_lifetime.ia6t_vltime = 0xFFFFFFFFL;

		int sockfd6 = socket(AF_INET6, SOCK_DGRAM, 0);
		if (sockfd6 < 0)
			perror("socket(AF_INET6)");

		if (ioctl(sockfd6, __IOCTL_OPERATION, &addreq6) == -1)
			perror("SIOCAIFADDR_IN6");
	#endif
	
	char buffer[MTU];
	struct ip6_hdr* headers = (struct ip6_hdr*) &buffer;
	struct in6_addr* src_addr = &headers->ip6_src;
	struct in6_addr* dst_addr = &headers->ip6_dst;
	
	while (1)
	{		
		FD_ZERO(&selectlist);
		FD_SET(tuntapfd, &selectlist);
		FD_SET(sockfd, &selectlist);
		
		int nfds;
		nfds = max(tuntapfd, sockfd) + 1;
		
		int len = select(nfds, &selectlist, NULL, NULL, 0);

		if (len <= 0)
		{
			perror("select");
			return -1;
		}
		
		if (FD_ISSET(tuntapfd, &selectlist) != 0)
		{
			memset(&buffer, 0, MTU);
			long readvalue = read(tuntapfd, &buffer, MTU);
			
			if (readvalue < 0)
			{
				perror("read");
				return -1;
			}
			
			if (dst_addr->s6_addr[0] != 0xFD ||
				src_addr->s6_addr[0] != 0xFD)
				continue;
		
			struct underlink_node source, destination;
			memcpy((void*) &source.nodeID, src_addr->s6_addr, sizeof(underlink_nodeID));
			memcpy((void*) &destination.nodeID, dst_addr->s6_addr, sizeof(underlink_nodeID));
			
			if (memcmp(&src_addr->s6_addr, &thisNode.nodeID, sizeof(underlink_nodeID)) != 0)
			{
				if (debug)
				{
					fprintf(stderr, "Packet discarded by filter: invalid source ");
					printNodeIPAddress(stderr, &source.nodeID);
					fprintf(stderr, "\n");
				}
				
				continue;
			}
						
			sendIPPacket(buffer, readvalue, source, destination, headers);
		}
		
		if (FD_ISSET(sockfd, &selectlist) != 0)
		{
			struct underlink_message* message = (underlink_message*) &buffer;
			
			memset(&buffer, 0, MTU);
			long readvalue = read(sockfd, &buffer, MTU);
			
			printf("Received %li bytes from UDP\n", readvalue);
			underlink_message_dump(message);
			
			if (memcmp(&message->remoteID, &thisNode.nodeID, sizeof(underlink_nodeID)) == 0)
			{
				if (write(tuntapfd, message->packetbuffer, message->payloadsize) <= 0)
				{
					fprintf(stderr, "TUN/TAP error when attempting to write to adapter: ");
					perror("write");
					fprintf(stderr, "\n");
				}
					else
				printf("Written to TUN/TAP\n");
			}
				else
			{
				//sendIPPacket(message->packetbuffer, message->payloadsize, message->remoteID, message->localID, 0);
			}
		}
	}
}

int sendIPPacket(char buffer[MTU], long length, underlink_node source, underlink_node destination, struct ip6_hdr* headers)
{
	underlink_node closest;
	memset(&closest, 0, sizeof(char) * 16);

	closest = getClosestAddressFromBuckets(destination, 0);

	if (closest.nodeID.big == 0 && closest.nodeID.small)
	{
		fprintf(stderr, "Remote node ");
		printNodeIPAddress(stderr, &destination.nodeID);
		fprintf(stderr, " is not accessible; no intermediate router known\n");
		return -1;
	}

	if (closest.endpoint.sin_addr.s_addr == 0)
	{
		fprintf(stderr, "Packet discarded: node ");
		printNodeIPAddress(stderr, &destination.nodeID);
		fprintf(stderr, " has no remote endpoint\n");
		return -1;
	}
		
	struct underlink_message* msg = underlink_message_construct(IPPACKET, thisNode.nodeID, closest.nodeID, length);
	char* sendbuffer = calloc(1, MTU);
	memcpy(&msg->packetbuffer, &buffer, MTU);
	int sendsize = underlink_message_pack(sendbuffer, msg);
	
	printf("sizeof(underlink_message): %i\n", sizeof(underlink_message));
	printf("length: %i\n", length);
	printf("sendsize: %i\n", sendsize);
	printf("sendlength: %i\n", length + sizeof(underlink_message));
	
	if (debug)
	{
		fprintf(stderr, "Sending %lu bytes to node ", length);
		printNodeIPAddress(stderr, &destination.nodeID);
		fprintf(stderr, " via ");
		printNodeIPAddress(stderr, &closest.nodeID);
		fprintf(stderr, "\n");
	}
		
	if (sendto(sockfd, sendbuffer, sendsize, 0, (struct sockaddr*) &closest.endpoint, sizeof(closest.endpoint)) <= 0)
	{
		fprintf(stderr, "Socket error when attempting to send to ");
		printNodeIPAddress(stderr, &closest.nodeID);
		fprintf(stderr, ":\n -> ");
		perror("sendto");
	}
	
	free(sendbuffer);
	free(msg);
}
