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
		if ((n+1) % 8 == 0) printf(" ");
		printf("%c", (b & 1ULL << n) ? '1' : '0');
	}
	printf("\n");
}

/*
 * 	int getBucketID(underlink_node check)
 * 
 * 	Returns the bucket ID for the given node. This is
 *	calculated by the number of common bits at the
 *	beginning of the node IDs.
 */
int getBucketID(underlink_node check)
{
	uint64_t bits = ~0ULL >> 16;

	int i;
	for (i = sizeof(underlink_nodeID); i > 0; i --)
	{
	//	if ((check.nodeID[i] & bits) == (thisNode.nodeID[i] & bits))
	//		return sizeof(underlink_nodeID) - i;
		
		bits <<= 1;
	}

	return sizeof(underlink_nodeID);
}

/*	
 *	long long unsigned int keyComparator(const underlink_node* a, const underlink_node* b)
 *
 *	Used for qsort to determine whether the distance between
 *	the current node and the two given nodes should result in
 *	the node being moved in the working nodeset.
 */
int keyComparator(const void* a, const void* b)
{	
	struct underlink_node* ia = (struct underlink_node*) &a;
	struct underlink_node* ib = (struct underlink_node*) &b;
	
	if (uint128_lessthan(getDistance(thisNode, *ia), getDistance(thisNode, *ib))) return 1;
	if (uint128_greaterthan(getDistance(thisNode, *ia), getDistance(thisNode, *ib))) return -1;
	return 0;
}

/*
 * 	underlink_node getClosestAddressFromBuckets(underlink_node check)
 * 
 * 	Searches the bucket list for the closest destination
 *	node to the node given. This will start at the most 
 *	appropriate bucket, working upwards until the closest
 *	address is found. 
 */
underlink_node getClosestAddressFromBuckets(underlink_node check, int steps, underlink_routermode routermode)
{
	int startBucket = getBucketID(check);	
	if (startBucket == 0 || uint128_equals(thisNode.nodeID, check.nodeID))
		return thisNode;
		
	uint128_t lastdist;
	underlink_node returnnode;
	memset(&returnnode.nodeID, 0, sizeof(underlink_pubkey));

	int b;
	for (b = sizeof(underlink_nodeID); b > 0; b --)
	{
		// this is not efficient
		underlink_node nodes[NODES_PER_BUCKET];
		memcpy(&nodes, &buckets[b], sizeof(struct underlink_node) * NODES_PER_BUCKET);
		qsort(&nodes, NODES_PER_BUCKET, sizeof(struct underlink_node), keyComparator);

		int n;
		for (n = steps; n < NODES_PER_BUCKET; n ++)
		{			
			if ((nodes[n].nodeID.big == 0 && nodes[n].nodeID.small == 0) || &nodes[n] == 0)
				continue;
				
			if (nodes[n].routermode != routermode)
				continue;

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

/*
 * 	int addNodeToBuckets(underlink_node newnode)
 * 
 * 	Adds the given node to the bucket list. The bucket
 *	is determined automatically, and the number returned.
 */
int addNodeToBuckets(underlink_node newnode)
{
	int b = getBucketID(newnode);
	
	int n;
	for (n = 0; n < NODES_PER_BUCKET; n ++)
	{
		if (buckets[b][n].nodeID.big == 0 && buckets[b][n].nodeID.small == 0)
		{
			memcpy(&buckets[b][n], &newnode, sizeof(underlink_node));
			
			if (debug)
			{
				printf("Inserted %s node ", newnode.routermode == ROUTER ? "router" : "direct-only");
				printNodeIPAddress(stdout, newnode);
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
	
	//if (debug)
	//	printf("Replacing node 0x%08llX (pos %i), with 0x%08llX in bucket %i\n",
	//			ntohll(buckets[b][i].nodeID), i, ntohll(newnode.nodeID), b);
				
	memcpy(&buckets[b][n], &newnode, sizeof(underlink_node));
	
	return b;
}
