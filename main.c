#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip6.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/select.h>
#include "main.h"
#include "node.h"
#include "bucket.h"
#include "message.h"

#define ntohll(x) (((uint64_t)(ntohl((uint32_t)((x << 32) >> 32))) << 32) | ntohl(((uint32_t)(x >> 32))))                                        
#define htonll(x) ntohll(x)

underlink_node thisNode;
underlink_node buckets[ADDR_LEN][NODES_PER_BUCKET];

int max(int a, int b)
{
	return a > b ? a : b;
}

int main(int argc, char* argv[])
{
	int portnumber = 3456;
	int opt;
	int sockfd, tuntapfd;
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
	thisNode.nodeID = lrand48();
	thisNode.endpoint.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &thisNode.endpoint.sin_addr);
	thisNode.endpoint.sin_port = htons(portnumber);
	
	int i;
	for (i = 0; i < 15; i ++)
	{
		underlink_node n;
		n.nodeID = lrand48();
		addNodeToBuckets(n);
	}
	
	printf("My node ID: %llu\n", thisNode.nodeID);
	
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (sockfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
	
	if (nodename)
		strcpy(nodename, "/dev/tun0");
		
	#ifdef __linux__
		struct ifreq ifr;
		memset(&ifr, 0, sizeof(ifr));
	
		if ((tuntapfd = open("/dev/net/tun", O_RDWR)) < 0)
		{
			fprintf(stderr, "Unable to find /dev/net/tun\n");
			return -1;
		}
	
		strcpy(ifr.ifr_name, nodename);
		ifr.ifr_flags = IFF_TUN;
		ifr.ifr_flags |= IFF_NO_PI;
	
		if (ioctl(tuntapfd, TUNSETIFF, (void *) &ifr) < 0)
		{
			fprintf(stderr, "Unable to configure tuntap device\n");
			return -1;
		}
	#else
		if ((tuntapfd = open(nodename, O_RDWR)) < 0)
		{
			fprintf(stderr, "Unable to open tuntap device '%s'\n", nodename);
			return -1;
		}
	#endif
	
	while (1)
	{
		FD_ZERO(&selectlist);
		FD_SET(tuntapfd, &selectlist);
		FD_SET(sockfd, &selectlist);
		
		int nfds = max(tuntapfd, sockfd);
		nfds++;
		
		int len = select(nfds, &selectlist, NULL, NULL, 0);

		if (len < 0)
		{
			perror("select");
			return -1;
		}
		
		if (FD_ISSET(tuntapfd, &selectlist) != 0)
		{
			char buffer[1500];
			struct ip6_hdr* headers = (struct ip6_hdr*) &buffer;
			
			long readvalue = read(tuntapfd, &buffer, 1500);
			
			if (readvalue < 0)
			{
				perror("read");
				return -1;
			}
			
			struct in6_addr* src_addr = &headers->ip6_src;
			struct in6_addr* dst_addr = &headers->ip6_dst;
			
			if (dst_addr->s6_addr[0] != 0xFD || dst_addr->s6_addr[1] != 0xFD) continue;
			if (src_addr->s6_addr[0] != 0xFD || src_addr->s6_addr[1] != 0xFD) continue;
				
			char src[128], dst[128];
			inet_ntop(AF_INET6, &headers->ip6_src, src, 128);
			inet_ntop(AF_INET6, &headers->ip6_dst, dst, 128);
			printf("Source: %s, destination: %s\n", src, dst);
				
			struct underlink_node source, destination, closest;
			memset(&source, 0, sizeof(char) * 16);
			memset(&destination, 0, sizeof(char) * 16);
			memset(&closest, 0, sizeof(char) * 16);
			memcpy((void*) &source.nodeID + 2, (void*) &src_addr->s6_addr + 2, sizeof(char) * 8);
			memcpy((void*) &destination.nodeID + 2, (void*) &dst_addr->s6_addr + 2, sizeof(char) * 8);			
			source.nodeID = ntohll(source.nodeID);
			destination.nodeID = ntohll(destination.nodeID);
			
			closest = getClosestAddressFromBuckets(destination, 0);
			
			printf("\tXOR difference: %llx\n", (source.nodeID ^ destination.nodeID));
			printf("\tMatch ID: \t%llx\n", destination.nodeID);
			printf("\tClosest ID: \t%llx\n", closest.nodeID);
		}
	}
}