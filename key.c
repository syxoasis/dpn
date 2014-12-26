#include <stdio.h>
#include <string.h>
#include <sodium.h>

#include "key.h"
#include "node.h"

void generateKey(dpn_pubkey* pk, dpn_seckey* sk)
{
	dpn_pubkey genpk;
	dpn_seckey gensk;
	
	while (!isPublicKeyValid(genpk))
	{
		crypto_box_curve25519xsalsa20poly1305_keypair(
			(unsigned char*) &genpk,
			(unsigned char*) &gensk
		);
	}
	
	memcpy(pk, genpk, sizeof(dpn_pubkey));
	memcpy(sk, gensk, sizeof(dpn_seckey));
}

void getNodeIDFromKey(dpn_nodeID* id, dpn_pubkey pk)
{
	uint8_t h[crypto_hash_sha512_BYTES];
	crypto_hash_sha512(h, (unsigned char*) pk, dpn_pklen);
    crypto_hash_sha512((uint8_t*) id, h, crypto_hash_sha512_BYTES);
}

int isPublicKeyValid(dpn_pubkey pk)
{
	uint8_t h[crypto_hash_sha512_BYTES];
	getNodeIDFromKey((dpn_nodeID*) &h, pk);
	
	return h[0] == 0xFD;
}
