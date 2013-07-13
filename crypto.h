#pragma GCC diagnostic ignored "-Wdeprecated-declarations"

#include <stdint.h>
#include <openssl/aes.h>
#include <openssl/rand.h>

uint64_t getKeyFromID(unsigned char* key);
int generateNewKey(unsigned char* key);
int isKeyValid(unsigned char* key);