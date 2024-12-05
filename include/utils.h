#pragma once
#include <WString.h>

String getChipId();
void hexDump(const uint8_t* pData, uint32_t length);
void * decompress_emplace(void * compressed_data, uint32_t src_size, uint32_t decomp_size);
