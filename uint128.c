#include "uint128.h"

uint128_t uint128_xor(uint128_t a, uint128_t b)
{
	uint128_t r;
	
	r.big = a.big ^ b.big;
	r.small = a.small ^ b.small;
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