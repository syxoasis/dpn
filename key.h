#pragma once

#include <stdint.h>
#include <sodium.h>
#include "typedef.h"
#include "node.h"

int isPublicKeyValid(underlink_pubkey pk);
void getNodeIDFromKey(underlink_nodeID* id, underlink_pubkey pk);
void generateKey(underlink_pubkey* pk, underlink_seckey* sk);

