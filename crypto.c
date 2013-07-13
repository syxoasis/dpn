#include "crypto.h"

uint64_t getKeyFromID(unsigned char* key)
{
	int i, r;
	for (i = 0; i < 32; i ++)
		r |= key[i] << (8 * i);
	
	return r;
}

int generateNewKey(unsigned char* key)
{
	int r;
	while (!isKeyValid(key))
		r = RAND_bytes(key, sizeof(*key));

	return r;
}

int isKeyValid(unsigned char* key)
{
	return key[0] == 0xFC;
}