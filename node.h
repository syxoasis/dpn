typedef struct underlink_node
{
	unsigned long long			nodeID;
	struct sockaddr_storage*	endpoint;
	char						publickey[64];
} underlink_node;

unsigned long long getDistance(underlink_node one, underlink_node two);