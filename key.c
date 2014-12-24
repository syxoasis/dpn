#include <stdio.h>
#include <string.h>
#include <sodium.h>

#include "key.h"
#include "node.h"

void generateKey(underlink_pubkey* pk, underlink_seckey* sk)
{
	underlink_pubkey genpk;
	underlink_seckey gensk;
	
	while (!isPublicKeyValid(genpk))
	{
		crypto_box_curve25519xsalsa20poly1305_keypair(
			(unsigned char*) &genpk,
			(unsigned char*) &gensk
		);
	}
	
	memcpy(pk, genpk, sizeof(underlink_pubkey));
	memcpy(sk, gensk, sizeof(underlink_seckey));
}

void getNodeIDFromKey(underlink_nodeID* id, underlink_pubkey pk)
{
	uint8_t h[crypto_hash_sha512_BYTES];
	crypto_hash_sha512(h, (unsigned char*) pk, underlink_pklen);
    crypto_hash_sha512((uint8_t*) id, h, crypto_hash_sha512_BYTES);
}

int isPublicKeyValid(underlink_pubkey pk)
{
	uint8_t h[crypto_hash_sha512_BYTES];
	getNodeIDFromKey((underlink_nodeID*) &h, pk);
	
	return h[0] == 0xFD;
}
