#pragma once

#include "FSStructs.h"
#include "LowNitroHandle.h"
#include "FileAccessor.h"
#include "FileIOPorts.h"
#include "System/ProcessorContext.h"

#if defined(jpn)
#define func_020d1198 func_020d2c64

#define data_02111304 data_02110fa4

#define data_02111728 data_021113c8
#define data_0211172c data_021113cc
#define data_02111738 data_021113d8
#define data_0211173c data_021113dc
#define data_02111754 data_021113f4

#define data_021118e0 data_02111580
#define data_02111f00 data_02111ba0
#endif

extern "C"
{
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

    // For each overlay we have 0x20 bytes of metadata in the ROM.
    // The ARM9 overlays are all grouped together in a table, and the ARM7 overlays
    // are grouped together in another table. (In DQIX there are no arm7 overlays)
    // In theory, one or both of these tables could be loaded into memory,
    // in which case this would point to it. In practice, I don't think it ever is
    struct ArmOverlayDataTableData
    {
        void* start;
        unsigned int size;
    };
    ArmOverlayDataTableData arm9Data;
    ArmOverlayDataTableData arm7Data;
};

// sizeof == 0x60
struct Struct_02111880
{
    int unknown_0;
    int unknown_4[23];
};

// sizeof <= 0x620
#define CARTRIDGE_READ_CONTEXT_FLAG_0 0
#define CARTRIDGE_READ_CONTEXT_FLAG_2 2
#define CARTRIDGE_READ_CONTEXT_FLAG_TASK_PENDING 3
#define CARTRIDGE_READ_CONTEXT_FLAG_4 4
#define CARTRIDGE_READ_CONTEXT_FLAG_5 5
#define CARTRIDGE_READ_CONTEXT_FLAG_6 6

struct Struct_021118e0
{
    typedef void (*PFNCartridgeRead)(Struct_021118e0*);

    Struct_02111880* pUnknown_0;
    int unknown_4, unknown_8;
    int lockCount_C;
    BlockedContextList blockedList_10; // unblocked by func_020cfd7c (from opcode 10), blocked by func_020cfcf8 (from opcode 9)
    int unknown_18;
    unsigned int cartridgeReadOffset;
    unsigned char* writeDst;
    unsigned int writeLength;
    unsigned int dmaChannel;
    int unknown_2c[3];
    PFNNitroCleanup maybeCleanupProc_38;
    NitroHandle* handle_3c;
    PFNCartridgeRead cartridgeReadProc;
    // populated as a call within func_020cfe08
    ProcessorContext cartridgeReadContext;
    ProcessorContext* pContext_104;
    // Set to 4 and 8 in different places
    unsigned int contextPriority_108;
    BlockedContextList list_10C;
    // bit 3: set when there is a task to be done by the read context
    volatile unsigned int flags_114;
    // see func_020d0a5c
    unsigned int maybeInstructionCacheLimit_118;
    unsigned int maybeDataCacheLimit_11c;

    // Start of this seems to have game title, but can't find where it gets
    // copied in
    char unknown[0x500];
};

struct Struct_02111f20
{
    typedef void (*PFNRead)(Struct_02111f20*);

    PFNRead readProc;
    unsigned int control_4;
    unsigned int* alignedWrite;
    unsigned int unknown_C[5];
    // 512 = 0x200 bytes of temporary space for unaligned writes
    unsigned char scratchBuffer[512];
};

struct Struct_02111f00
{
    unsigned int number;
    unsigned int unknown_4[7];
    Struct_02111f20 innerStruct;
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

extern Struct_021118e0 data_021118e0;
// Some kind of offset for reading from the ROM. In practice it's always 0
extern Struct_02111f00 data_02111f00;
//extern Struct_02111f20 data_02111f20;