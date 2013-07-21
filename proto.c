#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include "main.h"
#include "proto.h"

void tai_pack(char *s, struct tai *t)
{
	uint64 x;
	x = t->x;
	s[7] = x & 255; x >>= 8;
	s[6] = x & 255; x >>= 8;
	s[5] = x & 255; x >>= 8;
	s[4] = x & 255; x >>= 8;
	s[3] = x & 255; x >>= 8;
	s[2] = x & 255; x >>= 8;
	s[1] = x & 255; x >>= 8;
	s[0] = x;
}

void tai_unpack(char *s, struct tai *t)
{
	uint64 x;
	x = (unsigned char) s[0];
	x <<= 8; x += (unsigned char) s[1];
	x <<= 8; x += (unsigned char) s[2];
	x <<= 8; x += (unsigned char) s[3];
	x <<= 8; x += (unsigned char) s[4];
	x <<= 8; x += (unsigned char) s[5];
	x <<= 8; x += (unsigned char) s[6];
	x <<= 8; x += (unsigned char) s[7];
	t->x = x;
}

void taia_pack(char *s, struct taia *t)
{
	unsigned long x;
	tai_pack(s, &t->sec);
	s += 8;
	x = t->atto;
	s[7] = x & 255; x >>= 8;
	s[6] = x & 255; x >>= 8;
	s[5] = x & 255; x >>= 8;
	s[4] = x;
	x = t->nano;
	s[3] = x & 255; x >>= 8;
	s[2] = x & 255; x >>= 8;
	s[1] = x & 255; x >>= 8;
	s[0] = x;
} 

void taia_unpack(char *s, struct taia *t)
{
	unsigned long x;
	tai_unpack(s, &t->sec);
	s += 8;
	x = (unsigned char) s[4];
	x <<= 8; x += (unsigned char) s[5];
	x <<= 8; x += (unsigned char) s[6];
	x <<= 8; x += (unsigned char) s[7];
	t->atto = x;
	x = (unsigned char) s[0];
	x <<= 8; x += (unsigned char) s[1];
	x <<= 8; x += (unsigned char) s[2];
	x <<= 8; x += (unsigned char) s[3];
	t->nano = x;
}

void taia_now(struct taia *t)
{
	struct timeval now;
	gettimeofday(&now, (struct timezone *) 0);
	t->sec.x = 4611686018427387914ULL + (uint64) now.tv_sec;
	t->nano = 1000 * now.tv_usec + 500;
	t->atto++;
}

int proto_encode(proto_nacl inst, unsigned char* input, unsigned char* output, unsigned int len)
{
	if (len + noncelength + crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES > MTU)
	{
		fprintf(stderr, "Encryption failed (packet length %i is above MTU %i)\n", len, MTU);
		return -1;
	}
		
	unsigned char tempbufferinput[len + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES];
	
	memset(tempbufferinput, 0, crypto_box_curve25519xsalsa20poly1305_ZEROBYTES);
	memcpy(tempbufferinput + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES, input, len);
	
	len += crypto_box_curve25519xsalsa20poly1305_ZEROBYTES;
	
	taia_now(&inst.cdtaie);
	taia_pack(inst.encnonce + nonceoffset, &(inst.cdtaie));

	int result = crypto_box_curve25519xsalsa20poly1305_afternm(
		output,
		tempbufferinput,
		len,
		inst.encnonce,
		inst.precomp
	);
	
	memcpy(output, inst.encnonce + nonceoffset, noncelength);
	
	if (result)
	{
		fprintf(stderr, "Encryption failed (length %i, given result %i)\n", len, result);
		return -1;
	}
	
	return len;
}

int proto_decode(proto_nacl inst, unsigned char* input, unsigned char* output, unsigned int len)
{
	if (len - crypto_box_curve25519xsalsa20poly1305_ZEROBYTES > MTU)
	{
		fprintf(stderr, "Decryption failed (packet length %i is above MTU %i)\n", len, MTU);
		return 0;
	}
	
	if (len < crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES + noncelength)
	{
		fprintf(stderr, "Short packet received: %d\n", len);
		return 0;
	}

	struct taia cdtaic;
	unsigned char tempbufferout[len];
	
	taia_unpack((char*) input, &cdtaic);

	memcpy(inst.decnonce + nonceoffset, input, noncelength);
	memset(input, 0, crypto_box_curve25519xsalsa20poly1305_BOXZEROBYTES);
	
	int result = crypto_box_curve25519xsalsa20poly1305_open_afternm(
		tempbufferout,
		input,
		len,
		inst.decnonce,
		inst.precomp
	);
	
	inst.cdtaip = cdtaic;
	
	if (result)
	{
		fprintf(stderr, "Decryption failed (length %i, given result %i)\n", len, result);
		return 0;
	}
	
	len -= crypto_box_curve25519xsalsa20poly1305_ZEROBYTES;
	memcpy(output, tempbufferout + crypto_box_curve25519xsalsa20poly1305_ZEROBYTES, len);
	
	return len;
}

int proto_init(proto_nacl inst)
{
	unsigned char taipublickey[crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES];
	
	crypto_box_curve25519xsalsa20poly1305_beforenm(
		inst.precomp,
		inst.publickey,
		inst.privatekey
	);
	
	memset(inst.encnonce, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	memset(inst.decnonce, 0, crypto_box_curve25519xsalsa20poly1305_NONCEBYTES);
	
	crypto_scalarmult_curve25519_base(taipublickey, inst.privatekey);
	
	inst.encnonce[nonceoffset - 1] = memcmp(taipublickey, inst.publickey, crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES) > 0 ? 1 : 0;
	inst.decnonce[nonceoffset - 1] = inst.encnonce[nonceoffset - 1] ? 0 : 1;
	
	return 0;
}
