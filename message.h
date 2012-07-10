typedef struct underlink_nodelist
{
	underlink_node*	node;
	void* next;
}
underlink_nodelist;

typedef enum underlink_messagetype
{
	PING, SEARCH, FORWARD
}
underlink_messagetype;

typedef struct underlink_message
{
	underlink_messagetype message;
	unsigned long long localID;
	unsigned long long remoteID;
	int payloadsize;
	union
	{
		underlink_nodelist* nodes;
		char* packetbuffer;
	};
}
underlink_message;

underlink_message* underlink_message_construct(underlink_messagetype messagetype,
unsigned long long localID, unsigned long long remoteID, int payloadsize);
int underlink_message_addnode(underlink_message* packet, underlink_node* node);
int underlink_message_getkey(underlink_message* packet, void* output, int key);
int underlink_message_pack(void* out, underlink_message* packet);
int underlink_message_unpack(underlink_message* out, void* buffer, int buffersize);
void underlink_message_dump(underlink_message* packet);