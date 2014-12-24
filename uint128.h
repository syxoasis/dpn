#pragma once
#include <stdint.h>
#include <arpa/inet.h>

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
int uint128_maskequals(uint128_t a, uint128_t ma, uint128_t b, uint128_t mb);
void uint128_replace(uint128_t* a, uint128_t b);
void uint128_makeBigEndian(uint128_t* a);
void uint128_makeLittleEndian(uint128_t* a);
