#pragma once
#include "typedef.h"
#include "main.h"

int dpn_message_pack(void* out, dpn_message* packet);
int dpn_message_unpack(dpn_message* out, void* buffer, int buffersize);
void dpn_message_dump(dpn_message* packet);
