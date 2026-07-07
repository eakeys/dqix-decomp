#pragma once

/*
    To do: there is some trickery with volatile stuff. I'm trying to keep it as minimal
    as possible, short of casting everything to volatile exclusively for the places
    where it makes a difference, but some of this could definitely be wrong.

    As it currently stands:
    * reg9 in NitroVM needs to be volatile so that writing to the address it holds
      in FS72_Command_GetFileByName happens in the right order. At least the pointer
      member needs to be volatile.
      
    
*/

#include "System/ProcessorContext.h"

struct FSLinkedListHeader
{
    void* pPrev;
    void* pNext;
};

struct FSLinkedListChildSet
{
    void* pFirst;
    void* pLast;
};

template<class T>
struct FSListHeader
{
    T* pPrev;
    T* pNext;
};

struct NitroHandle;
struct NitroVM;

typedef int CBool;

// Signature: (this, dst, imageOffset, copyLength)
typedef int(*PFNLoadFile)(NitroHandle*, void*, unsigned int, unsigned int);
// Signature: (this, src, imageOffset, copyLength)
typedef int(*PFNSaveFile)(NitroHandle*, const void*, unsigned int, unsigned int);
//
typedef int(*PFNExecuteCommand)(NitroVM*, int);

// The FS72 struct seems to hold a bunch of values that are used
// for various different purposes by different commands. It seems they're
// essentially treated as registers for some kind of virtual machine
union FSRegister
{
    void* ptr;
    int s32;
    unsigned int u32;
    struct {
        unsigned short low;
        unsigned short high;
    } u16;
    struct {
        short low;
        short high;
    } s16;
};

// There are a few operations that act on multiple registers at once
// via ldm / stm commands, which is consistent with copy-assigning an array
union FSRegisterTriple
{
    struct {
        FSRegister a;
        FSRegister b;
        FSRegister c;
    };
};

#define NITROVM_FLAG_0 0
#define NITROVM_FLAG_1 1
#define NITROVM_FLAG_2 2
#define NITROVM_FLAG_3 3
#define NITROVM_FLAG_4 4
// might have other uses but this is the only decomped one so far
#define NITROVM_FLAG_SEARCHING_DIRECTORY 5
// some kind of sync / interrupt loop thing again
#define NITROVM_FLAG_6 6
#define NITROVM_FLAG_7 7
#define NITROVM_FLAG_8 8
#define NITROVM_FLAG_9 9
#define NITROVM_FLAG_10 10

#define NITROVM_OPCODE_LOAD 0
#define NITROVM_OPCODE_SAVE 1
#define NITROVM_OPCODE_GET_DIRECTORY_DATA 2
#define NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_NAME_DATA 3
#define NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_BY_NAME 4
#define NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_PATH 5
#define NITROVM_OPCODE_GET_FAT_ENTRY 6
#define NITROVM_OPCODE_COPY_REGISTERS 7
#define NITROVM_OPCODE_8 8
#define NITROVM_OPCODE_ACQUIRE_NDS_BUS 9
#define NITROVM_OPCODE_RELEASE_NDS_BUS 10
#define NITROVM_OPCODE_MAYBE_INVALID 14

#define NITRO_RESULT_SUCCESS 0
#define NITRO_RESULT_FAILURE 1
#define NITRO_RESULT_2 2
#define NITRO_RESULT_MAYBE_INVALID_HANDLE 3
#define NITRO_RESULT_4 4
#define NITRO_RESULT_5 5
// not sure about this one. The load opcode for the ROM filesystem returns this, and
// ExecuteCommand() does a special loop based on nitroHandle flag 9 if the
// instruction returns this value.
#define NITRO_RESULT_REQUIRE_SYNC_MAYBE 6
#define NITRO_RESULT_FALLBACK_TO_DEFAULT 7
#define NITRO_RESULT_OPCODE_NOT_IMPLEMENTED 8

// This is the 72-byte struct we see in a bunch of places
struct NitroVM
{
    FSListHeader<NitroVM> links;
    NitroHandle* linkedHandle;
    // Initially we were always marking the struct as volatile. But this
    // seems to work better. Uses are not properly known, but for now:
    // * bit 5: directory (set) or file (clear). Used in command #5
    volatile unsigned int flags;
    // func_020cbf14 uses this as the opcode in a call to ExecuteCommand
    int maybeScheduledCommand;
    int storedResult;
    BlockedContextList unknown_sublist_18;
    FSRegisterTriple regbase_abc;
    FSRegister regbase_d;
    FSRegisterTriple regext_abc;
    FSRegister regext_d;
    FSRegister reg8;
    volatile FSRegister reg9;
};

struct FSReadHandle
{
    NitroHandle* nitroHandle;
    unsigned int offset;
};

#define NITROHANDLE_FLAG_IS_IN_MAIN_LIST 0
#define NITROHANDLE_FLAG_IS_POPULATED 1
#define NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY 2
#define NITROHANDLE_FLAG_3 3
#define NITROHANDLE_FLAG_NDS_BUS_HELD 4
#define NITROHANDLE_FLAG_5 5
// used for some sort of interrupt / sync loop thing
#define NITROHANDLE_FLAG_6 6
#define NITROHANDLE_FLAG_MAYBE_DESTRUCTION_UNDERWAY 7
#define NITROHANDLE_FLAG_8 8
#define NITROHANDLE_FLAG_9 9

// sizeof(NitroHandle) == 92 == 0x5C.
// Used much more broadly but idk where atm
struct NitroHandle
{
    unsigned int signature; // 'rom' or 'arc'
    // Weird, but I think this is next and previous in that order
    NitroHandle* pNeighbor4;
    NitroHandle* pNeighbor8;
    BlockedContextList unknown_0C;
    BlockedContextList unknown_14;
    volatile unsigned int flags;
    // Creates a fake first entry in a list of VMs. The previous pointer
    // in here is unused (probably null) and next points to the first actual VM
    FSListHeader<NitroVM> linkToFirstVM;
    void* pFileImage;
    int fatOffset_2C;
    unsigned int fatSize;
    int nameTableOffset_34;
    unsigned int nameTableSize;
    int fatOffset_3C;
    int nameTableOffset_40;
    // When the tables are loaded into memory, this is the pointer that was
    // passed (though the start of the tables might be a bit after this)
    void* tableRawPointer;
    PFNLoadFile loadFileProc_48;
    PFNSaveFile saveFileProc;
    // From some lua testing, this is always either 020cbc18 or 020cbc54.
    // The former copies from (pFileImage + imageOffset), the latter treats
    // imageOffset as a true pointer.
    PFNLoadFile loadFileProc_50;
    // Seems to be some kind of opcode override. Signature (NitroVM*, int opcode).
    // The only non-null one I've found is func_020ccd48 overriding opcodes 1,
    // 9 and 10 (with the latter two being unimplemented by default)
    PFNExecuteCommand instructionOverride;
    // Indicates whether to use the opcode override for a given opcode
    unsigned int overrideOpcodeFlags;
};

struct NitroFileAccessor
{
    NitroHandle* handle;
    unsigned int fileID;
};

struct NitroDirectoryAccessor
{
    // unnamed struct to make this trivially copyable (using ldm, stm commands)
    struct {
        NitroHandle* handle;
        unsigned short dirID;
        unsigned short firstFileID;
        unsigned int handleSubtableOffset;
    };
};

extern "C"
{
    void NitroVM_UnlinkAndStoreResult(NitroVM* fs, int result);
    int NitroVM_ExecuteCommand(NitroVM* fs, int opcode);
    int CaseInsensitiveStrncmp(const unsigned char* first, const unsigned char* second, unsigned int len);
    int FS_ReadBytes(FSReadHandle* handle, void* dst, unsigned int len);

    // Pass the index of the directory relative to the beginning of directory entries.
    // That is, for the directory F000, pass 0. For FFFF, pass FFF.
    // The FS struct will end up containing the following values.
    // base_A: NitroHandle* (fs->unknown_8)
    // base_B.low = input dirIndex
    // base_B.high = id of first file in this directory
    // base_C = offset of subtable relative to start of data section for NarcHandleInitial
    // base_D = (int)id of parent directory
    int NitroVM_LoadDirectoryDataByIndex(NitroVM* fs, unsigned int dirIndex);

    // Default implementation for NitroVM opcode 0.
    // Expects as inputs:
    // base_D = offset from beginning of the nitro data to start copying from
    // ext_A = destination to load data to
    // ext_C = number of bytes to load.
    // Outputs:
    // base_D is adjusted by the number of bytes loaded.
    int NitroVM_DefaultCommand_Load(NitroVM* fs);

    // Default implementation for NitroVM opcode 1.
    // Very likely completely unused since nitro files are read-only.
    // Expects as inputs:
    // base_D = offset from beginning of the nitro data to start copying to
    // ext_A = source data to copy
    // ext_C = number of bytes to copy
    // Outputs:
    // value_2C is adjusted by the number of bytes saved.
    int NitroVM_DefaultCommand_Save(NitroVM* fs);

    // Default implementation for NitroVM opcode 2.
    // Expects as inputs:
    // ext_B_low = directory index (omit the 0xF000)
    // Outputs:
    // base_A = input ext_A
    // base_B_low = input directory index
    // base_D = parent directory index, or num directories if this is the root
    // Provided that ext_C = 0 and ext_B_high = 0:
    // base_B_high is overwritten by the first file in the referenced directory
    // base_C is the offset to the subtable for files/subdirs in said directory
    // (relative to the nitro handle)
    // If this precondition is not met, the values from ext_b_high and ext_c
    // respectively will just be copied in
    int NitroVM_DefaultCommand_GetDirectoryData(NitroVM* fs);

    // Default implementation for NitroVM opcode 3.
    // Loads data from a FNT subtable within the nitro filesystem.
    // Inputs:
    // base_C = offset to start reading from (relative to the nitro handle)
    // ext_b = (int as bool) 1 if string should *NOT* be saved, 0 otherwise
    // ext_a = pointer to destination for extra data here
    // base_b_high = probably: index of current file to process (if this entry is a file)
    // Outputs:
    // The address in ext_a is filled as a FileDataStore (see the cpp file)
    // base_C is updated to the new offset after this entry
    // base_b_high is incremented if we found a file (not a directory)
    // Returns 0 on success, 1 if at end of table and other values if inner
    // processes go wrong for whatever reason
    int NitroVM_DefaultCommand_GetFileOrDirectoryNameData(NitroVM* fs);

    // Default implementation for NitroVM opcode 4.
    // Gets the ID and related data of a file / directory based on the name.
    // Inputs:
    // [technicality] ext_a = should equal nitro handle if passing empty file string, unused otherwise
    // ext_b_low = id of root directory to start the search, minus 0xF000. Zero means search everywhere
    // ext_b_high = should be zero to avoid weird behaviour
    // ext_c = should be zero to avoid weird behaviour
    // ext_d = pointer to buffer holding file path
    // reg8 = 1 if searching for a directory, 0 if searching for a file
    // reg9 = storage pointer
    // Outputs:
    // At the address given by the reg9 input, we populate either a
    // NitroFileMetadata or a NitroDirectoryMetadata depending on what we
    // found.
    // The other registers are probably unused, but their contents will be:
    // base_a = nitro handle
    // base_b_low = id either of the directory we found, or of the directory
    //              containing the file we found
    // base_b_high = first file of directory, or 1+(id of found file)
    // base_c = offset to FNT subtable for found directory, or pointer past
    //          entry in the relevant FNT subtable for the file we found
    // base_d = parent directory id of either the found directory or the directory
    //          containing the file we found (i.e. 2 levels up)
    // ext_a = nitro handle if got a directory, garbage stack pointer otherwise
    // ext_b = directory id if got a directory, 0 otherwise
    // ext_c = 0
    // ext_d, reg8, reg9 unchanged
    int NitroVM_DefaultCommand_GetFileOrDirectoryByName(NitroVM* fs);

    // Default implementation for NitroVM opcode 5.
    // Computes the full path of a file or directory given its ID.
    // Inputs:
    // flag bit 5 (0x20): set if specifying a directory, clear if specifying a file
    // base_a: id of file to get data of (unused if handling a directory)
    // base_b_low: id of dir to get data of (unused for files)
    // ext_a: pointer to output the filename. If null, nothing will be written
    //        and routine will succeed, providing only the other outputs
    // ext_b: output buffer capacity
    // ext_c_low: should be 0. If nonzero, this and ext_c_high will be treated
    //            as outputs from a previous run of this function (with only
    //            perhaps a different value of ext_a)
    // Outputs:
    // the full path, as a null-terminated string in the format
    // "sig:/path/to/file.ext" (in practice, sig is either 'arc' or 'rom')
    // is written to the address in ext_a
    // ext_c_low: number of bytes written, including the null-terminator
    // ext_c_high: directory id (either input dir id if you specified a dir, or
    // the id of the container directory if you specified a file)
    int NitroVM_DefaultCommand_GetPath(NitroVM* fs);

    // Default implementation for NitroVM opcode 6.
    // Loads data from the file allocation table.
    // Inputs:
    // ext_b: file id / index in the table
    // Outputs:
    // base_a and ext_c: file id
    // base_b, base_d and ext_a: beginning of allocation
    // base_c and ext_b: end of allocation
    int NitroVM_DefaultCommand_GetFATEntry(NitroVM* fs);

    // Default implementation for NitroVM opcode 7.
    // Copies data from the extended registers into the base registers.
    // Specifically:
    // ext_a is copied into base_b and base_d
    // ext_b is copied into base_c
    // ext_c is copied into base_a
    int NitroVM_DefaultCommand_CopyRegisters(NitroVM* fs);

    // Default implementation for NitroVM opcode 8.
    // Does nothing, but might be intended as a sort of shutdown/destructor.
    // (See e.g. func_020cca80 in USA, which is used at the end of functions
    // that create temporary VMs and invokes command 8).
    int NitroVM_DefaultCommand_8_Nop(NitroVM* fs);

    unsigned int Nitro_CalculateSignature(const char* str, int len);

    NitroVM* NitroHandle_020cbc6c(NitroHandle* handle);
    void NitroVM_020cbe80(NitroVM* vm);
    CBool NitroVM_ExecuteAndUnlink_020cbf14(NitroVM* vm);
    // Seems like it executes the command and
    // does some stuff to ensure it's 'fully' executed. (might be waiting
    // for peripheral stuff e.g. the gamecard bus, or because of queued 
    // commands / stored results etc)
    CBool NitroVM_020cbf58(NitroVM* vm, int opcode);
}

struct FSStruct76
{
    unsigned short unknown_0;
    unsigned short unknown_2;
    NitroVM inner;
};

