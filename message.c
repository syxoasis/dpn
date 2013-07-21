#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <netinet/ip.h>
#include "node.h"
#include "message.h"

int underlink_message_pack(void* out, underlink_message* packet)
{
	int size = sizeof(underlink_message) + packet->payloadsize;
	
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
	printf("\tLocal ");
	printNodeIPAddress(stdout, &packet->localID);
	printf(" -> Remote ");
	printNodeIPAddress(stdout, &packet->remoteID);
	printf("\n");

	if (packet->message == IPPACKET)
	{
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
		
		return;
	}
}

