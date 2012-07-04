#include <stdio.h>
#include "main.h"
#include "node.h"
#include "bucket.h"

underlink_node thisNode;
underlink_node buckets[ADDR_LEN][NODES_PER_BUCKET];

int main(int argc, const char** argv)
{
	if (debug)
	{
		printf("Maximum peer count: %i\n", ADDR_LEN * NODES_PER_BUCKET);
		printf("Bucket table size in memory: %lukb\n",
			(sizeof(underlink_node) * ADDR_LEN * NODES_PER_BUCKET) / 24);
	}
	
	thisNode.nodeID = 12345590;

	int i;
	for (i = 0; i < 150; i += 10)
	{
		underlink_node foo;
		foo.nodeID = 12345600 + i;
		addNodeToBuckets(foo);
	}
	
	underlink_node checkID;
	checkID.nodeID = 12345651;

	underlink_node closest = getClosestAddressFromBuckets(checkID);
	printf("found %llu\n", closest.nodeID);
}