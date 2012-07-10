#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "main.h"
#include "node.h"
#include "bucket.h"
#include "message.h"

underlink_node thisNode;
underlink_node buckets[ADDR_LEN][NODES_PER_BUCKET];

int main(int argc, char* argv[])
{
	int portnumber = 3456;
	int opt;
	int sockfd;
	
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
	thisNode.nodeID = rand();
	thisNode.endpoint.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &thisNode.endpoint.sin_addr);
	thisNode.endpoint.sin_port = htons(portnumber);
	
	int i;
	for (i = 0; i < ADDR_LEN * NODES_PER_BUCKET; i += 1)
	{
		underlink_node foo;
		foo.nodeID = rand();
		foo.endpoint.sin_family = AF_INET;
		inet_pton(AF_INET, "127.0.0.1", &foo.endpoint.sin_addr);
		foo.endpoint.sin_port = htons(i);
		addNodeToBuckets(foo);
	}
	
	underlink_node checkID;
	checkID.nodeID = rand();

	printf("Looking for %llu... ", checkID.nodeID);
	underlink_node closest = getClosestAddressFromBuckets(checkID, 0);
	printf("closest match: %llu\n", closest.nodeID);
	
	underlink_message* packet = underlink_message_construct(SEARCH, thisNode.nodeID, checkID.nodeID, 5);
	
	for (i = 0; i < 5; i ++)
	{
		closest = getClosestAddressFromBuckets(checkID, i);
		underlink_message_addnode(packet, &closest);
	}

	underlink_message_dump(packet);
	
	sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	
	if (sockfd < 0)
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}
}