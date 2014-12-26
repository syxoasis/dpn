#pragma once

#include <stdint.h>
#include <sodium.h>
#include "typedef.h"
#include "node.h"

int isPublicKeyValid(dpn_pubkey pk);
void getNodeIDFromKey(dpn_nodeID* id, dpn_pubkey pk);
void generateKey(dpn_pubkey* pk, dpn_seckey* sk);

