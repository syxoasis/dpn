#define debug 1

#include "typedef.h"
#include "node.h"

int sendIPPacket(char buffer[MTU], long length, underlink_nodeID source, underlink_nodeID destination, int ttl);
