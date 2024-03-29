#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include "node.h"
#include "message.h"

int dpn_message_pack(void* out, dpn_message* packet)
{
	int size = sizeof(dpn_message) + packet->payloadsize;
	memcpy(out, packet, size);
	return size;
}

int dpn_message_unpack(dpn_message* out, void* buffer, int buffersize)
{
	memcpy(out, buffer, buffersize);
	return buffersize;
}

void dpn_message_makeLittleEndian(dpn_message* msg)
{
	msg->message = ntohl(msg->message);
	uint128_makeLittleEndian(&msg->localID);
	uint128_makeLittleEndian(&msg->remoteID);
	msg->payloadsize = ntohl(msg->payloadsize);
}

void dpn_message_makeBigEndian(dpn_message* msg)
{
	msg->message = htonl(msg->message);
	uint128_makeBigEndian(&msg->localID);
	uint128_makeBigEndian(&msg->remoteID);
	msg->payloadsize = htonl(msg->payloadsize);
}

void dpn_message_dump(dpn_message* packet)
{
	printf("Message ID: 0x%X, payload size: %i, time-to-live: %i\n", packet->message, packet->payloadsize, packet->ttl);
	printf("\tLocal ");
	printNodeIPAddress(stdout, &packet->localID);
	printf(" -> Remote ");
	printNodeIPAddress(stdout, &packet->remoteID);
	printf("\n");
	
	printf("\tPacket type: ");
	switch (packet->message)
	{
		case IPPACKET:			printf("Packet forward request\n"); break;
		case FORWARDED_REFER:	printf("Packet forwarded, referral provided\n"); break;
		case NOT_FORWARDED:		printf("Not forwarded\n"); break;
		case VERIFY:			printf("Verify request\n"); break;
		case VERIFY_SUCCESS:	printf("Verify success\n"); break;
		case UNSPEC_ERROR:		printf("Unspecified error\n"); break;
	}

	if (packet->payloadsize == 0)
		return;
	
	printf("\tPacket contents:\n\t");
		
	int s;
	for (s = 0; s < packet->payloadsize; s ++)
	{
		if (s % 8 == 0)
			printf("%03i-%03i: ", s, s + 7);
			
		printf("%08x ", packet->packetbuffer[s]);

		if (s % 8 == 7)
		 	printf("\n\t");
	}
		
	printf("\n");
}

