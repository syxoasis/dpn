#pragma once

#include <sodium.h>
#include "uint128.h"

#define MTU 1400
#define NODES_PER_BUCKET 24

#define underlink_pklen crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES
#define underlink_sklen crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES

typedef unsigned char underlink_pubkey[underlink_pklen];
typedef unsigned char underlink_seckey[underlink_sklen];

typedef uint128_t underlink_nodeID;

typedef struct underlink_node
{
        underlink_nodeID                nodeID;
        underlink_pubkey                publickey;
        struct sockaddr_in              endpoint;
//      proto_nacl                              crypto;
} underlink_node;

typedef struct underlink_nodelist
{
        underlink_node* node;
        void* next;
}
underlink_nodelist;

typedef enum underlink_messagetype
{
        IPPACKET,                       // Please route this packet for me
        FORWARDED_REFER,        // Packet forwarded, please use this next hop next time
        NOT_FORWARDED,          // Packet not forwarded, please try another hop
        VERIFY,                         // I want to add you to my node buckets
        VERIFY_SUCCESS,         // Yes, add me to your node buckets
        UNSPEC_ERROR            // An unspecified unrecoverable error occured
}
underlink_messagetype;

typedef struct underlink_message
{
        underlink_messagetype message;
        int payloadsize;
        underlink_nodeID localID;
        underlink_nodeID remoteID;
        int ttl;
        int flags;
        union
        {
                char packetbuffer[MTU];
                underlink_node node;
        };
}
underlink_message;

#ifndef ntohll
#define ntohll(x) (((uint64_t)(ntohl((uint32_t)((x << 32) >> 32))) << 32) | ntohl(((uint32_t)(x >> 32))))
#endif

#ifndef htonll
#define htonll(x) ntohll(x)
#endif
