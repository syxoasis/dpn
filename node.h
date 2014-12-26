#pragma once

#include <netinet/in.h>
#include "typedef.h"
#include "proto.h"
#include "uint128.h"

uint128_t getDistance(dpn_node one, dpn_node two);
void printNodeIPAddress(FILE* pipe, dpn_nodeID* nodeID);
