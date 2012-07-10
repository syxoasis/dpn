#include <netinet/in.h>

typedef struct underlink_node
{
	unsigned long long		nodeID;
	struct sockaddr_in		endpoint;
	char					publickey[64];
	int						lastused;
} underlink_node;

unsigned long long getDistance(underlink_node one, underlink_node two);