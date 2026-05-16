#pragma once

void InvalidateDataCache();
void CleanDataCache();
void CleanInvalidateDataCache();
void InvalidateDataCacheRange(void* where, unsigned int len);
void CleanCacheRange(void* where, unsigned int len);
void CleanInvalidateCacheRange(void* where, unsigned int len);
void DrainWriteBuffer();
void InvalidateInstructionCache();
void InvalidateInstructionCacheRange(void* where, unsigned int len);