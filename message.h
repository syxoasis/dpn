#pragma once
#include "main.h"

typedef enum underlink_messagetype
{
	IPPACKET,			// Please route this packet for me
	FORWARDED_REFER,	// Packet forwarded, please use this next hop next time
	NOT_FORWARDED,		// Packet not forwarded, please try another hop
	VERIFY,				// I want to add you to my node buckets
	VERIFY_SUCCESS,		// Yes, add me to your node buckets
	UNSPEC_ERROR		// An unspecified unrecoverable error occured
}
underlink_messagetype;

typedef struct underlink_message
{
	underlink_messagetype message;
	int payloadsize;
	underlink_nodeID localID;
	underlink_nodeID remoteID;
	int ttl;
	union
	{
		char packetbuffer[MTU];
		underlink_node node;
	};
}
underlink_message;

int underlink_message_pack(void* out, underlink_message* packet);
int underlink_message_unpack(underlink_message* out, void* buffer, int buffersize);
void underlink_message_dump(underlink_message* packet);
