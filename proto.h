#pragma once

#include "include/crypto_box_curve25519xsalsa20poly1305.h"
#include "include/crypto_scalarmult_curve25519.h"

#define uint64 unsigned long long
#define noncelength 16
#define nonceoffset (crypto_box_curve25519xsalsa20poly1305_NONCEBYTES - noncelength)

struct tai
{
	uint64 x;
};

struct taia
{
	struct tai sec;
	unsigned long nano;
	unsigned long atto;
};

typedef struct proto_nacl
{
	unsigned char privatekey[crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES];
	unsigned char publickey[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];
	unsigned char precomp[crypto_box_curve25519xsalsa20poly1305_BEFORENMBYTES];
	unsigned char encnonce[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	unsigned char decnonce[crypto_box_curve25519xsalsa20poly1305_NONCEBYTES];
	
	struct taia cdtaip, cdtaie;
}
proto_nacl;

void tai_pack(char *s, struct tai *t);
void tai_unpack(char *s, struct tai *t);
void taia_pack(char *s, struct taia *t);
void taia_unpack(char *s, struct taia *t);
void taia_now(struct taia *t);

int proto_encode(proto_nacl inst, unsigned char* input, unsigned char* output, unsigned int len);
int proto_decode(proto_nacl inst, unsigned char* input, unsigned char* output, unsigned int len);
int proto_init(proto_nacl inst);
