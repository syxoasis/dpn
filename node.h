#include <netinet/in.h>
#include "proto.h"
#include "key.h"

typedef enum underlink_routermode
{
	DIRECT_ONLY, 
	ROUTER 
}
underlink_routermode;

typedef struct underlink_node
{
	underlink_pubkey		key;
	struct sockaddr_in		endpoint;
	int						lastused;
	underlink_routermode	routermode;
	proto_nacl				crypto;
} underlink_node;

typedef struct underlink_nodelist
{
	underlink_node*	node;
	void* next;
}
underlink_nodelist;

uint64_t getDistance(underlink_node one, underlink_node two);
