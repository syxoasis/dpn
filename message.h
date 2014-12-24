#pragma once
#include "typedef.h"
#include "main.h"

int underlink_message_pack(void* out, underlink_message* packet);
int underlink_message_unpack(underlink_message* out, void* buffer, int buffersize);
void underlink_message_dump(underlink_message* packet);
