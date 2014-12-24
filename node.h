#pragma once

#include <netinet/in.h>
#include "typedef.h"
#include "proto.h"
#include "uint128.h"

uint128_t getDistance(underlink_node one, underlink_node two);
void printNodeIPAddress(FILE* pipe, underlink_nodeID* nodeID);
