#include "key.h"
#include "include/crypto_hash_sha512.h"

void generateKey(underlink_pubkey* pk, underlink_seckey* sk)
{
	do
	{
		crypto_box_curve25519xsalsa20poly1305_keypair(
			(unsigned char*) pk,
			(unsigned char*) sk
		);
	}
	while (!isPublicKeyValid(pk));
}

int isPublicKeyValid(underlink_pubkey* pk)
{
	uint8_t h[crypto_hash_sha512_BYTES];
	crypto_hash_sha512(h, (unsigned char*) pk, underlink_pklen);
    crypto_hash_sha512(h, h, crypto_hash_sha512_BYTES);
	
	return h[0] == 0xFD;
}

