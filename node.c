#include "main.h"
#include "node.h"

/*
 *	unsigned long long getDistance(underlink_node one, underlink_node two)
 *
 *	Calculates the distance between the two node IDs.
 *	This is a simple XOR operation.
 */
unsigned long long getDistance(underlink_node one, underlink_node two)
{
	return one.nodeID ^ two.nodeID;
}