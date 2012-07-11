#include <netinet/in.h>

typedef struct underlink_node
{
	uint64_t				nodeID;
	struct sockaddr_in		endpoint;
	char					publickey[64];
	int						lastused;
} underlink_node;

uint64_t getDistance(underlink_node one, underlink_node two);