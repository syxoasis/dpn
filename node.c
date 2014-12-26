#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include "main.h"
#include "node.h"
#include "uint128.h"

uint128_t getDistance(dpn_node one, dpn_node two)
{
	return uint128_xor(one.nodeID, two.nodeID);
}

void printNodeIPAddress(FILE* pipe, dpn_nodeID* nodeID)
{
	char presentational[164];
	memset(&presentational, 0, 164);
	inet_ntop(AF_INET6, nodeID, presentational, 164);
	fprintf(pipe, "%s", presentational);
}