#include <stdint.h>
#include "include/crypto_box_curve25519xsalsa20poly1305.h"

#define underlink_pklen crypto_box_curve25519xsalsa20poly1305_PUBLICKEYBYTES
#define underlink_sklen crypto_box_curve25519xsalsa20poly1305_SECRETKEYBYTES

typedef unsigned char underlink_pubkey[underlink_pklen];
typedef unsigned char underlink_seckey[underlink_sklen];