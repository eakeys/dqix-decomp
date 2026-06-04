#pragma once

#include "FSStructs.h"
#include "LowNitroHandle.h"
#include "FileAccessor.h"

extern "C"
{
    void func_020c7898(void*);
    void func_020c78e8(void*);

    CBool func_020d1198();
}

#define GET_FLAG_BIT(what, idx) (((what) & (1 << (idx))) ? 1 : 0)

struct Struct_02111728
{
    NitroHandle* handle;
    NitroDirectoryAccessor primaryFSRoot;
};

struct Struct_0211173c
{
    unsigned int unknown_0;
    unsigned int maybeDMAChannel;
    // These are referenced as 8 byte things somewhere, probably a smaller struct

    struct ArmData
    {
        unsigned int unknown[2];
    };

    ArmData unknown_8;
    ArmData unknown_10;
};

// These overlap, but the 72c is accessed from the 728 as well as on its own.
// It should be possible to just take a reference to mountedDir directly, e.g.
// if you copy assign from it you'll get .word data_02111728+0x4 (which is
// the same assembly as .word data_0211172c), but it doesn't always seem to work
extern Struct_02111728 data_02111728;
extern NitroDirectoryAccessor data_0211172c;

extern int data_02111738; // stores whether the rom fs is initialised
extern Struct_0211173c data_0211173c;
extern NitroHandle data_02111754;