#define debug 1

#include "typedef.h"
#include "node.h"

int sendIPPacket(char buffer[MTU], long length, dpn_nodeID source, dpn_nodeID destination, int ttl);
