#include "uint128.h"

#ifndef ntohll
#define ntohll(x) (((uint64_t)(ntohl((uint32_t)((x << 32) >> 32))) << 32) | ntohl(((uint32_t)(x >> 32))))
#endif

#ifndef htonll
#define htonll(x) ntohll(x)
#endif

uint128_t uint128_xor(uint128_t a, uint128_t b)
{
	uint128_t r;
	
	r.big = a.big ^ b.big;
	r.small = a.small ^ b.small;
	
	return r;
}

int uint128_compare(uint128_t a, uint128_t b)
{
	return a.big == b.big && a.small == b.small;
}

int uint128_greaterthan(uint128_t a, uint128_t b)
{
	return a.big > b.big || (a.big == b.big && a.small > b.small);
}

int uint128_lessthan(uint128_t a, uint128_t b)
{
	return a.big < b.big || (a.big == b.big && a.small < b.small);
}

int uint128_equals(uint128_t a, uint128_t b)
{
	return a.small == b.small && a.big == b.big;
}

int uint128_maskequals(uint128_t a, uint128_t ma, uint128_t b, uint128_t mb)
{
	return (a.small & ma.small) == (b.small & mb.small) &&
	       (a.big & ma.big) == (b.big & mb.big);
}

void uint128_replace(uint128_t* a, uint128_t b)
{
	a->big = b.big;
	a->small = b.small;
}

void uint128_makeBigEndian(uint128_t* a)
{
	a->big = htonll(a->big);
	a->small = htonll(a->small);
}

void uint128_makeLittleEndian(uint128_t* a)
{
	a->big = ntohll(a->big);
	a->small = ntohll(a->small);
}
