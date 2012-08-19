#include <netinet/in.h>

typedef enum underlink_routermode
{
	DIRECT_ONLY, 
	ROUTER 
}
underlink_routermode;

typedef struct underlink_node
{
	uint64_t				nodeID;
	struct sockaddr_in		endpoint;
	char					publickey[64];
	int						lastused;
	underlink_routermode	routermode;
} underlink_node;

typedef struct underlink_nodelist
{
	underlink_node*	node;
	void* next;
}
underlink_nodelist;

uint64_t getDistance(underlink_node one, underlink_node two);