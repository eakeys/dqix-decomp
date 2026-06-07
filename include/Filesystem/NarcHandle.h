#pragma once

#include "FSStructs.h"

bool IsFileValidNarc(const unsigned char* buffer);

// sizeof(NarcHandle) == 104.
class NarcHandle
{
public:
    NitroHandle initial;
    const void* pNarcFile; // points to where the narc itself is loaded in memory
    const void* pFATBSection; // points to the 'FATB' header
    const void* pFileDataStart; // points past the 8-byte 'FIMG' header

    bool Initialize(const char* signatureString, const unsigned char* buffer);
    bool Destroy();

    const void* GetFileByIndex(unsigned int idx) const;
};

const void* GetFileFromNARCInMemory(const char* filename);
bool PrepareReadFileInNARCByID(NitroVM* vm, NarcHandle* handle, unsigned int id);