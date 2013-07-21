#pragma once
#include "main.h"

typedef enum underlink_messagetype
{
	CONTROL,
	IPPACKET
}
underlink_messagetype;

typedef enum underlink_keytype
{
	ROUTE,				// Please route this packet for me
	FORWARDED_REFER,	// Packet forwarded, please use this next hop next time
	NOT_FORWARDED,		// Packet not forwarded, please try another hop
	VERIFY,				// I want to add you to my node buckets
	VERIFY_SUCCESS,		// Yes, add me to your node buckets
	UNSPEC_ERROR		// An unspecified unrecoverable error occured
}
underlink_keytype;

typedef struct underlink_keypair
{
	underlink_keytype key;
	char value[32];
}
underlink_keypair;

typedef struct underlink_message
{
	underlink_messagetype message;
	underlink_nodeID localID;
	underlink_nodeID remoteID;
	int payloadsize;
	union
	{
		underlink_nodelist* nodes;
		underlink_keypair keypair;
		char packetbuffer[MTU];
	};
}
underlink_message;

int underlink_message_addnode(underlink_message* packet, underlink_node* node);
int underlink_message_getkey(underlink_message* packet, void* output, int key);
int underlink_message_pack(void* out, underlink_message* packet);
int underlink_message_unpack(underlink_message* out, void* buffer, int buffersize);
void underlink_message_dump(underlink_message* packet);
