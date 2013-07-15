#pragma once

#include <netinet/in.h>
#include "proto.h"
#include "key.h"
#include "uint128.h"

typedef enum underlink_routermode
{
	DIRECT_ONLY, 
	ROUTER,
	ORCHESTRATOR
}
underlink_routermode;

typedef uint128_t underlink_nodeID;

typedef struct underlink_node
{
	underlink_nodeID		nodeID;
	underlink_pubkey		publickey;
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

uint128_t getDistance(underlink_node one, underlink_node two);
