#pragma once
#include <stdint.h>

typedef struct
{
	uint64_t big;
	uint64_t small;
}
uint128_t;

uint128_t uint128_xor(uint128_t a, uint128_t b);
int uint128_compare(uint128_t a, uint128_t b);
int uint128_greaterthan(uint128_t a, uint128_t b);
int uint128_lessthan(uint128_t a, uint128_t b);
int uint128_equals(uint128_t a, uint128_t b);
void uint128_replace(uint128_t* a, uint128_t b);