#pragma once

#include <sodium.h>
#include "uint128.h"

#define MTU 1400
#define NODES_PER_BUCKET 24

#define dpn_pklen crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES
#define dpn_sklen crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES

typedef unsigned char dpn_pubkey[dpn_pklen];
typedef unsigned char dpn_seckey[dpn_sklen];

typedef uint128_t dpn_nodeID;

typedef struct dpn_node
{
        dpn_nodeID                nodeID;
        dpn_pubkey                publickey;
        struct sockaddr_in              endpoint;
//      proto_nacl                              crypto;
} dpn_node;

typedef struct dpn_nodelist
{
        dpn_node* node;
        void* next;
}
dpn_nodelist;

typedef enum dpn_messagetype
{
        IPPACKET,                       // Please route this packet for me
        FORWARDED_REFER,        // Packet forwarded, please use this next hop next time
        NOT_FORWARDED,          // Packet not forwarded, please try another hop
        VERIFY,                         // I want to add you to my node buckets
        VERIFY_SUCCESS,         // Yes, add me to your node buckets
        UNSPEC_ERROR            // An unspecified unrecoverable error occured
}
dpn_messagetype;

typedef struct dpn_message
{
        dpn_messagetype message;
        int payloadsize;
        dpn_nodeID localID;
        dpn_nodeID remoteID;
        int ttl;
        int flags;
        union
        {
                char packetbuffer[MTU];
                dpn_node node;
        };
}
dpn_message;

#ifndef ntohll
#define ntohll(x) (((uint64_t)(ntohl((uint32_t)((x << 32) >> 32))) << 32) | ntohl(((uint32_t)(x >> 32))))
#endif

#ifndef htonll
#define htonll(x) ntohll(x)
#endif
