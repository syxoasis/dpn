#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include "node.h"
#include "message.h"

underlink_message* underlink_message_construct(underlink_messagetype messagetype,
		unsigned long long localID, unsigned long long remoteID, int payloadsize)
{
	int payloadbytes;
	if (messagetype == SEARCH)
		payloadbytes = sizeof(underlink_nodelist) + (sizeof(underlink_nodelist) * payloadsize);
	else
		payloadbytes = sizeof(underlink_nodelist) + payloadsize;
	
	underlink_message* out = calloc(1, sizeof(underlink_message) + payloadbytes);

	out->message = messagetype;
	out->payloadsize = 0;
	out->localID = localID;
	out->remoteID = remoteID;

	return out;
}

int underlink_message_addnode(underlink_message* packet, underlink_node* node)
{
	if (packet == 0)
		return -1;

	underlink_nodelist* kp;
	if (&packet->payloadsize != 0)
		kp = (underlink_nodelist*) &packet->nodes;

	int i;
	for (i = 0; i < packet->payloadsize; i ++)
		kp = (underlink_nodelist*) &kp->next;

	memcpy(&kp->node, node, sizeof(struct underlink_node));
	
	packet->payloadsize ++;

	return 0;
}

int underlink_message_getkey(underlink_message* packet, void* output, int key)
{
	underlink_nodelist* kp;
	if (packet->payloadsize == 0)
		return;

	kp = (underlink_nodelist*) &packet->payloadsize;

	int i;
	for (i = 0; i < packet->payloadsize; i ++)
	{
		if (i == key)
		{
			memcpy(output, kp->node, sizeof(kp->node));
			return sizeof(kp->node);
		}

		if (&kp->next != 0)
			kp = (underlink_nodelist*) &kp->next;
		else   
			break;
	}

	return 0;
}

int underlink_message_pack(void* out, underlink_message* packet)
{
	int size;
	if (packet->message == SEARCH)
		size = sizeof(underlink_message) + (sizeof(underlink_nodelist) * packet->payloadsize);
	else
		size = sizeof(underlink_message) + packet->payloadsize;
	
	memcpy(out, packet, size);
	return size;
}

int underlink_message_unpack(underlink_message* out, void* buffer, int buffersize)
{
	memcpy(out, buffer, buffersize);
	return buffersize;
}

void underlink_message_dump(underlink_message* packet)
{
	printf("Message ID: 0x%X, payload size: %i\n", packet->message, packet->payloadsize);
	printf("\tLocal %llu -> Remote %llu\n", packet->localID, packet->remoteID);

	underlink_nodelist* kp;
	if (&packet->nodes == 0)
		return;

	kp = (underlink_nodelist*) &packet->nodes;

	int i;
	for (i = 0; i < packet->payloadsize; i ++)
	{
		if (&kp->node != 0)
		{
			underlink_node* node = (struct underlink_node*) &kp->node;
			char addr[64];
			inet_ntop(AF_INET, &node->endpoint.sin_addr, addr, 64);
			
			printf("\tNode %i: %llu (%s:%i)\n", i + 1, node->nodeID, addr,
					ntohs(node->endpoint.sin_port));
		}
		else
			printf("\tNode %i: (undefined)\n", i + 1);

		if (&kp->next != 0)
			kp = (underlink_nodelist*) &kp->next;
		else
			break;
	}
}
	