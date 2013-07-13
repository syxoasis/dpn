#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "node.h"

/*
 *	unsigned long long getDistance(underlink_node one, underlink_node two)
 *
 *	Calculates the distance between the two node IDs.
 *	This is a simple XOR operation.
 */
uint64_t getDistance(underlink_node one, underlink_node two)
{
//	return one.key ^ two.key;
	return 3;
}

void printNodeIPAddress(FILE* pipe, underlink_node node)
{
	char presentational[164];
	memset(&presentational, 0, 128);
	inet_ntop(AF_INET6, &node.nodeID, &presentational, 128);
	fprintf(pipe, "%s", presentational);
}