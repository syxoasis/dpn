#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "node.h"
#include "main.h"

extern underlink_node thisNode;
extern underlink_node buckets[sizeof(underlink_nodeID) * 8][NODES_PER_BUCKET];

void printbits(unsigned long long b, int n)
{
	for (--n; n >= 0; --n)
	{
		printf("%c", (b & 1ULL << n) ? '1' : '0');
		if ((n) % 8 == 0) printf("");
	}
	printf(" ");
}

int getBucketID(underlink_node check)
{
	uint128_t bits;
	memset(&bits, ~0, sizeof(uint128_t));
	check.nodeID.big = ntohll(check.nodeID.big);
	check.nodeID.small = ntohll(check.nodeID.small);
	
	uint128_t nhThisNode;
	nhThisNode.big = ntohll(thisNode.nodeID.big);
	nhThisNode.small = ntohll(thisNode.nodeID.small);

	int i;
	for (i = 0; i < sizeof(underlink_nodeID) * 8; i ++)
	{
		if (i < 64)
			bits.small <<= 1;
		else
			bits.big <<= 1;
		
		if (uint128_maskequals(check.nodeID, bits, nhThisNode, bits))
			return (sizeof(underlink_nodeID) * 8) - i;
	}

	return sizeof(underlink_nodeID) * 8;
}

int keyComparator(const void* a, const void* b)
{
	uint128_t distFromA = getDistance(thisNode, *(struct underlink_node*) &a);
	uint128_t distFromB = getDistance(thisNode, *(struct underlink_node*) &b);
	
	if (uint128_lessthan(distFromA, distFromB)) return 1;
	if (uint128_greaterthan(distFromA, distFromB)) return -1;
	
	return 0;
}

underlink_node getClosestAddressFromBuckets(underlink_node check, int steps)
{
	int startBucket = getBucketID(check);
	if (startBucket == 0 || uint128_equals(thisNode.nodeID, check.nodeID))
		return thisNode;
		
	uint128_t lastdist;
	memset(&lastdist, 0, sizeof(uint128_t));
	
	underlink_node returnnode;
	memset(&returnnode.nodeID, 0, sizeof(underlink_nodeID));

	int b;
	for (b = 0; b < sizeof(underlink_nodeID) * 8; b ++)
	{
		underlink_node nodes[NODES_PER_BUCKET];
		memcpy(&nodes, &buckets[b], sizeof(struct underlink_node) * NODES_PER_BUCKET);
		qsort(&nodes, NODES_PER_BUCKET, sizeof(struct underlink_node), keyComparator);

		int n;
		for (n = steps; n < NODES_PER_BUCKET; n ++)
		{			
			if ((nodes[n].nodeID.big == 0 && nodes[n].nodeID.small == 0) || &nodes[n] == 0)
				continue;
			
			if (uint128_equals(check.nodeID, nodes[n].nodeID))
				return nodes[n];
			
			if (uint128_lessthan(getDistance(nodes[n], check), lastdist) ||
				(lastdist.big == 0 && lastdist.small == 0))
			{
				returnnode = nodes[n];
				lastdist = getDistance(nodes[n], check);
			}
		}
	}
	
	return returnnode;
}

int addNodeToBuckets(underlink_node newnode)
{
	int b = getBucketID(newnode);
	int n;
	
	for (n = 0; n < NODES_PER_BUCKET; n ++)
	{
		if (uint128_equals(buckets[b][n].nodeID, thisNode.nodeID))
			return b;
	}
	
	for (n = 0; n < NODES_PER_BUCKET; n ++)
	{
		if (buckets[b][n].nodeID.big == 0 && buckets[b][n].nodeID.small == 0)
		{
			memcpy(&buckets[b][n], &newnode, sizeof(underlink_node));
			
			if (debug)
			{
				printf("Inserted node ");
				printNodeIPAddress(stdout, &newnode.nodeID);
				printf(" into bucket %i (pos %i)\n", b, n);
			}
			
			return b;
		}
	}
	
	uint128_t closeness;
	int i = 0;
	for (n = 0; n < NODES_PER_BUCKET; n ++)
	{
		uint128_t dist = getDistance(buckets[b][n], newnode);
		
		if (uint128_greaterthan(dist, closeness))
		{
			uint128_replace(&closeness, dist);
			i = n;
		}
	}
	
	if (debug)
	{
		printf("Replacing node ");
		printNodeIPAddress(stdout, &buckets[b][i].nodeID);
		printf(" (pos %i) with ", i);
		printNodeIPAddress(stdout, &newnode.nodeID);
		printf(" in bucket %i\n", b);
	}
				
	memcpy(&buckets[b][n], &newnode, sizeof(underlink_node));
	
	return b;
}
