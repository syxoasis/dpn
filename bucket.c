#include <stdio.h>
#include "node.h"
#include "main.h"

extern underlink_node thisNode;
extern underlink_node buckets[ADDR_LEN][NODES_PER_BUCKET];

/*
 * 	int getBucketID(underlink_node check)
 * 
 * 	Returns the bucket ID for the given node. This is
 *	calculated by the number of common bits at the
 *	beginning of the node IDs.
 */
int getBucketID(underlink_node check)
{
	int i;
	for (i = ADDR_LEN; i > 0; i --)
	{
		unsigned long long bits = (1ULL << (i - 1ULL));
		
		if ((check.nodeID & bits) != (thisNode.nodeID & bits))
			return ADDR_LEN - i;
	}
	
	return ADDR_LEN;
}

/*
 * 	underlink_node getClosestAddressFromBuckets(underlink_node check)
 * 
 * 	Searches the bucket list for the closest destination
 *	node to the node given. This will start at the most 
 *	appropriate bucket, working upwards until the closest
 *	address is found. 
 */
underlink_node getClosestAddressFromBuckets(underlink_node check)
{
	int startBucket = getBucketID(check);
	
	if (startBucket == 0)
		return thisNode;
	
	unsigned long long closeness;
	underlink_node closest;
	
	int b;
	for (b = startBucket; b > 0; b --)
	{
		int n;
		for (n = 0; n < NODES_PER_BUCKET; n ++)
		{			
			if (buckets[b][n].nodeID == 0)
				continue;
		
			int dist = getDistance(buckets[b][n], check);
			if (dist < closeness)
			{
				closeness = dist;
				closest = buckets[b][n];
			}
		}
	}
	
	return closest;
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
			buckets[b][n] = newnode;
			if (debug)
				printf("Inserted node %llu into bucket %i (pos %i)\n", newnode.nodeID, b, n);
			return b;
		}
	}
	
	unsigned long long closeness = 0;
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
	buckets[b][i] = newnode;
	
	return b;
}
