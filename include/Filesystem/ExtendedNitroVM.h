#pragma once

#include "FSStructs.h"

// sizeof == 76 == 0x4c
class ExtendedNitroVM
{
public:
    unsigned short unknown_0;
    unsigned short unknown_2;
    NitroVM machine;

    void ZeroInitialize();

    bool CheckUnknownFlagBit4();
    unsigned int GetFileSize();
    bool Seek(unsigned int where);
    bool MaybeReset();

    // Not sure what the idea behind the bool is, but it's always false.
    // If set to true, most of the function gets skipped
    bool PrepareRead(const char* filePath, bool skip);
    // Returns number of bytes copied
    unsigned int LoadToBuffer(void* into, unsigned int capacity);

    bool DoFlagStuff();
};

