#pragma once

typedef enum underlink_messagetype
{
	KEYPAIR,
	IPPACKET
}
underlink_messagetype;

typedef enum underlink_keytype
{
	PING,
	SEARCH
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
	underlink_pubkey localID;
	underlink_pubkey remoteID;
	int payloadsize;
	union
	{
		underlink_nodelist* nodes;
		underlink_keypair keypair;
		char* packetbuffer;
	};
}
underlink_message;

underlink_message* underlink_message_construct(underlink_messagetype messagetype, underlink_nodeID localID, underlink_nodeID remoteID, int payloadsize);
int underlink_message_addnode(underlink_message* packet, underlink_node* node);
int underlink_message_getkey(underlink_message* packet, void* output, int key);
int underlink_message_pack(void* out, underlink_message* packet);
int underlink_message_unpack(underlink_message* out, void* buffer, int buffersize);
void underlink_message_dump(underlink_message* packet);
