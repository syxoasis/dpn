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
	
	underlink_message msg;
	struct sockaddr_in remote;
	remote.sin_family = AF_INET;
	socklen_t addrlen = sizeof(remote);
	
	memset(&msg, 0, sizeof(underlink_message));
	memset(&buckets, 0, sizeof(underlink_node) * sizeof(underlink_nodeID) * NODES_PER_BUCKET);
	memset(&thisNode, 0, sizeof(underlink_node));
	
	generateKey(&localPublicKey, &localSecretKey);
	getNodeIDFromKey(&thisNode.nodeID, localPublicKey);
		
	thisNode.endpoint.sin_family = AF_INET;
	inet_pton(AF_INET, "0.0.0.0", &thisNode.endpoint.sin_addr);
	thisNode.endpoint.sin_port = htons(portnumber);
	
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
	
	addNodeToBuckets(thisNode);
	
	while ((opt = getopt(argc, argv, "b:")) != -1)
	{
		switch (opt)
		{
			case 'b':
				msg.message = VERIFY;
				msg.localID = thisNode.nodeID;
				msg.payloadsize = sizeof(underlink_node);
				memcpy(&msg.node, &thisNode, msg.payloadsize);
				underlink_message_dump(&msg);
				
				int p = inet_pton(PF_INET, optarg, &remote.sin_addr);
				if (p < 0)
					perror("inet_pton");
				
				remote.sin_family = AF_INET;
				remote.sin_port = htons(portnumber);
				
				int sentlen = sendto(sockfd, (char*) &msg, msg.payloadsize + sizeof(underlink_message), 0, (struct sockaddr*) &remote, addrlen);
				
				char foo[INET_ADDRSTRLEN];
				inet_ntop(AF_INET, &remote.sin_addr, &foo, 128);
				
				if (sentlen <= 0)
				{
					fprintf(stderr, "Socket error when attempting to send bootstrap message to %s:\n -> ", foo);
					perror("sendto");
					break;
				}

				printf("Sent bootstrap packet to %s\n", foo);
				break;
				
			default:
				fprintf(stderr, "Usage: %s [-b bootstrap ip]\n", argv[0]);
				exit(EXIT_FAILURE);
		}
	}
	
	if (debug)
	{
		printf("Using UDP port %i\n", portnumber);
		printf("Maximum peer count: %lu\n", sizeof(underlink_nodeID) * NODES_PER_BUCKET);
		printf("%lu-bit peer address space\n", sizeof(underlink_nodeID) * 8);
		printf("Bucket table size in memory: %lukb\n",
			(sizeof(underlink_node) * sizeof(underlink_nodeID) * NODES_PER_BUCKET) / 24);
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
		struct ifreq ifr;
		struct sockaddr_in6 sai;
		int conffd;
		struct in6_ifreq ifr6;
		memset(&ifr, 0, sizeof(ifr));
	
		if ((tuntapfd = open("/dev/net/tun", O_RDWR)) < 0)
		{
			fprintf(stderr, "Unable to find /dev/net/tun\n");
			return -1;
		}
	
		ifr.ifr_flags = IFF_TUN;
		ifr.ifr_flags |= IFF_NO_PI;
	
		if (ioctl(tuntapfd, TUNSETIFF, (void *) &ifr) < 0)
		{
			fprintf(stderr, "Unable to configure tuntap device\n");
			return -1;
		}
		
		conffd = socket(AF_INET6, SOCK_DGRAM, IPPROTO_IP);
	
		if (conffd < 0)
		{
			perror("socket");
			return -1;
		}

		memset(&sai, 0, sizeof(struct sockaddr));
		sai.sin6_family = AF_INET6;
		sai.sin6_port = 0;
	
		if (inet_pton(AF_INET6, prefix, (void*) &sai.sin6_addr) < 0)
		{
			perror("inet_pton");
			return -1;
		}
	
		memcpy((char*) &ifr6.ifr6_addr, (char*) &prefix, sizeof(struct in6_addr));
	
		if (ioctl(conffd, SIOGIFINDEX, &ifr) < 0)
			perror("SIOGIFINDEX");
	
		ifr6.ifr6_ifindex = ifr.ifr_ifindex;
		ifr6.ifr6_prefixlen = 8;
	
		if (ioctl(conffd, SIOCSIFADDR, &ifr6) < 0)
			perror("SIOCSIFADDR");
	
		ifr.ifr_flags |= IFF_UP | IFF_RUNNING;
	
		int ret = ioctl(conffd, SIOCSIFFLAGS, &ifr);
	
		close(conffd);
	#else
		if ((tuntapfd = open(nodename, O_RDWR)) < 0)
		{
			fprintf(stderr, "Unable to open tuntap device '%s'\n", nodename);
			return -1;
		}

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
			memcpy((void*) &source.nodeID, src_addr, sizeof(underlink_nodeID));
			memcpy((void*) &destination.nodeID, dst_addr, sizeof(underlink_nodeID));
			
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
						
			sendIPPacket(buffer, readvalue, source.nodeID, destination.nodeID);
			continue;
		}
		
		if (FD_ISSET(sockfd, &selectlist) != 0)
		{
			underlink_message message;
			memset(&msg, 0, sizeof(underlink_message));
			long readvalue = recvfrom(sockfd, &message, MTU, 0, (void*) &remote, &addrlen);
			underlink_message_dump(&message);
			
			switch (message.message)
			{
				case IPPACKET:
					if (uint128_equals(message.remoteID, thisNode.nodeID))
					{
						if (write(tuntapfd, message.packetbuffer, message.payloadsize) <= 0)
						{
							fprintf(stderr, "TUN/TAP error when attempting to write to adapter: ");
							perror("write");
							fprintf(stderr, "\n");
						}
					}
						else
					{
						if (uint128_equals(thisNode.nodeID, message.localID))
							break;
						
						if (message.remoteID.big == 0 && message.remoteID.small == 0)
							break;
						
						if (message.localID.big == 0 && message.localID.small == 0)
							break;
						
						sendIPPacket(message.packetbuffer, message.payloadsize, message.localID, message.remoteID);
					}
					break;
				
				case FORWARDED_REFER:
					msg.message = VERIFY;
					msg.localID = thisNode.nodeID;
					msg.remoteID = message.node.nodeID;
					msg.payloadsize = 0;
					break;
					
				case VERIFY:
					message.node.endpoint = remote;
					addNodeToBuckets(message.node);
					msg.message = VERIFY_SUCCESS;
					msg.localID = thisNode.nodeID;
					msg.remoteID = message.localID;					
					msg.payloadsize = sizeof(underlink_node);
					memcpy(&msg.node, &thisNode, msg.payloadsize);
					break;
					
				case VERIFY_SUCCESS:
					memcpy(&message.node.endpoint, &remote, sizeof(struct sockaddr_in));
					addNodeToBuckets(message.node);
					break;
					
				case NOT_FORWARDED:
					break;
			}
			
			if (msg.localID.big == 0 && msg.localID.small == 0)
				continue;
			
			int sentlen = sendto(sockfd, (char*) &msg, msg.payloadsize + sizeof(underlink_message), 0, (struct sockaddr*) &remote, addrlen);
			
			if (sentlen <= 0)
			{
				fprintf(stderr, "Socket error when attempting to send to ");
				printNodeIPAddress(stderr, &msg.remoteID);
				fprintf(stderr, ":\n -> ");
				perror("sendto");
			}
		}
	}
}

int sendIPPacket(char buffer[MTU], long length, underlink_nodeID source, underlink_nodeID destination)
{
	underlink_node closest;
	memset(&closest, 0, sizeof(char) * 16);

	underlink_node dst;
	dst.nodeID = destination;
	closest = getClosestAddressFromBuckets(dst, 0);

	if (closest.nodeID.big == 0 && closest.nodeID.small == 0)
	{
		fprintf(stderr, "Remote node ");
		printNodeIPAddress(stderr, &destination);
		fprintf(stderr, " is not accessible; no intermediate router known\n");
		return -1;
	}

	if (closest.endpoint.sin_addr.s_addr == 0)
	{
		fprintf(stderr, "Packet discarded: node ");
		printNodeIPAddress(stderr, &destination);
		fprintf(stderr, " has no remote endpoint\n");
		return -1;
	}
	
	underlink_message msg;
	msg.message = IPPACKET;
	uint128_replace(&msg.localID, thisNode.nodeID);
	uint128_replace(&msg.remoteID, destination);
	msg.payloadsize = length;
	memcpy(msg.packetbuffer, buffer, length);
	
	if (debug)
	{
		fprintf(stderr, "Sending %lu bytes to node ", length);
		printNodeIPAddress(stderr, &destination);
		fprintf(stderr, " via ");
		printNodeIPAddress(stderr, &closest.nodeID);
		fprintf(stderr, "\n");
	}
		
	int sentlen = sendto(sockfd, (char*) &msg, length + sizeof(underlink_message), 0, (struct sockaddr*) &closest.endpoint, sizeof(closest.endpoint));
	
	if (sentlen <= 0)
	{
		fprintf(stderr, "Socket error when attempting to send to ");
		printNodeIPAddress(stderr, &closest.nodeID);
		fprintf(stderr, ":\n -> ");
		perror("sendto");
	}
}
