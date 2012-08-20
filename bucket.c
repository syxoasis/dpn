#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <limits.h>
#include "node.h"
#include "main.h"

extern underlink_node thisNode;
extern underlink_node buckets[ADDR_LEN][NODES_PER_BUCKET];

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
	for (i = ADDR_LEN; i > 0; i --)
	{
		if ((check.nodeID & bits) == (thisNode.nodeID & bits))
			return ADDR_LEN - i;
		
		bits <<= 1;
	}

	return ADDR_LEN;
}

/*	
 *	long long unsigned int nodeIDComparator(const underlink_node* a, const underlink_node* b)
 *
 *	Used for qsort to determine whether the distance between
 *	the current node and the two given nodes should result in
 *	the node being moved in the working nodeset.
 */
int nodeIDComparator(const void* a, const void* b)
{	
	struct underlink_node* ia = (struct underlink_node*) &a;
	struct underlink_node* ib = (struct underlink_node*) &b;
	
	if (getDistance(thisNode, *ia) < getDistance(thisNode, *ib)) return 1;
	if (getDistance(thisNode, *ia) > getDistance(thisNode, *ib)) return -1;
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
	if (startBucket == 0 || thisNode.nodeID == check.nodeID)
		return thisNode;
		
	int lastdist = 0;
	underlink_node returnnode;
	returnnode.nodeID = 0;

	int b;
	for (b = ADDR_LEN; b > 0; b --)
	{
		// this is not efficient
		underlink_node nodes[NODES_PER_BUCKET];
		memcpy(&nodes, &buckets[b], sizeof(struct underlink_node) * NODES_PER_BUCKET);
		qsort(&nodes, NODES_PER_BUCKET, sizeof(struct underlink_node), nodeIDComparator);

		int n;
		for (n = steps; n < NODES_PER_BUCKET; n ++)
		{			
			if (nodes[n].nodeID == 0 || &nodes[n] == 0)
				continue;
				
			if (nodes[n].routermode != routermode)
				continue;

			if (getDistance(nodes[n], check) < lastdist ||
				lastdist == 0)
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
		if (buckets[b][n].nodeID == 0)
		{
			memcpy(&buckets[b][n], &newnode, sizeof(underlink_node));
			
			if (debug)
				printf("Inserted %s node %llu into bucket %i (pos %i)\n",
							newnode.routermode == ROUTER ? "router" : "direct-only",
							newnode.nodeID, b, n);
			return b;
		}
	}
	
	uint64_t closeness = 0;
	int i = 0;
	for (n = 0; n < NODES_PER_BUCKET; n ++)
	{
		int dist = getDistance(buckets[b][n], newnode);
		if (dist > closeness)
		{
			closeness = dist;
			i = n;
		}
	}
	
	if (debug)
		printf("Replacing node %llu (pos %i), with %llu in bucket %i\n",
				buckets[b][i].nodeID, i, newnode.nodeID, b);
				
	memcpy(&buckets[b][n], &newnode, sizeof(underlink_node));
	
	return b;
}
