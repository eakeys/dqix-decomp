#pragma once

/*
    To do: there is some trickery with volatile stuff. I'm trying to keep it as minimal
    as possible, short of casting everything to volatile exclusively for the places
    where it makes a difference, but some of this could definitely be wrong.

    As it currently stands:
    * reg9 in FSStruct72 needs to be volatile so that writing to the address it holds
      in FS72_Command_GetFileByName happens in the right order. At least the pointer
      member needs to be volatile.
      
    
*/

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

struct NarcHandleInitialPart;
struct FSStruct72;

// Signature: (this, dst, imageOffset, copyLength)
typedef int(*PFNLoadFile)(NarcHandleInitialPart*, void*, unsigned int, unsigned int);
// Signature: (this, src, imageOffset, copyLength)
typedef int(*PFNSaveFile)(NarcHandleInitialPart*, const void*, unsigned int, unsigned int);
//
typedef int(*PFNExecuteCommand)(FSStruct72*, int);

// sizeof(NarcHandleInitialPart) == 92.
// Used much more broadly but idk where atm
struct NarcHandleInitialPart
{
    unsigned int signature; // 'rom' or 'arc'
    char unknown_04[8];
    int unknown_0C;
    int unknown_10;
    int unknown_14;
    int unknown_18;
    unsigned int flags_1C; // might need this volatile
    char unknown_20[4];
    FSStruct72* fs_24;
    void* pFileImage;
    int fatOffset_2C;
    unsigned int fatSize;
    int nameTableOffset_34;
    unsigned int nameTableSize;
    int fatOffset_3C;
    int nameTableOffset_40;
    int unknown_44;
    PFNLoadFile loadFileProc_48;
    PFNSaveFile saveFileProc;
    // From some lua testing, this is always either 020cbc18 or 020cbc54.
    // The former copies from (pFileImage + imageOffset), the latter treats
    // imageOffset as a true pointer.
    PFNLoadFile loadFileProc_50;
    // Seems to be some kind of opcode override. Signature (FSStruct72*, int opcode).
    // The only non-null one I've found is func_020ccd48 overriding opcodes 1,
    // 9 and 10 (with the latter two being unimplemented by default)
    PFNExecuteCommand instructionOverride;
    // Indicates whether to use the opcode override for a given opcode
    unsigned int overrideOpcodeFlags;
};

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
union FSRegisterSet
{
    struct { FSRegister vals[4]; } abcd;
    struct { FSRegister vals[3]; } abc;
    struct { FSRegister vals[2]; } ab;

    struct {
        FSRegister a;
        FSRegister b;
        FSRegister c;
        FSRegister d;
    };
};

// func_020cbb90 
struct FSStruct72
{
    FSStruct72* pPrev;
    FSStruct72* pNext;
    NarcHandleInitialPart* unknown_8;
    // Initially we were always marking the struct as volatile. But this
    // seems to work better. Uses are not properly known, but for now:
    // * bit 5: directory (set) or file (clear). Used in command #5
    volatile unsigned int flags;
    // func_020cbf14 uses this as the opcode in a call to ExecuteCommand
    char unknown_10[4];
    int storedResult;
    FSLinkedListChildSet unknown_sublist_18;
    FSRegisterSet regbase;
    FSRegisterSet regext;
    FSRegister reg8;
    volatile FSRegister reg9;
};

struct FSReadHandle
{
    NarcHandleInitialPart* nitroHandle;
    unsigned int offset;
};

extern "C"
{
    void FS72_PopAndUpdateResult(FSStruct72* fs, int result);
    int FS72_ExecuteCommand(FSStruct72* fs, int opcode);
    int CaseInsensitiveStrncmp(const unsigned char* first, const unsigned char* second, unsigned int len);
    int FS_ReadBytes(FSReadHandle* handle, void* dst, unsigned int len);

    // Pass the index of the directory relative to the beginning of directory entries.
    // That is, for the directory F000, pass 0. For FFFF, pass FFF.
    // The FS struct will end up containing the following values.
    // value_20: NarcHandleInitialPart* (fs->unknown_8)
    // value_24.low = input dirIndex
    // value_24.high = id of first file in this directory
    // value_28 = offset of subtable relative to start of data section for NarcHandleInitial
    // value_2C = (int)id of parent directory
    int FS72_LoadDirectoryDataByIndex(FSStruct72* fs, unsigned int dirIndex);

    // Default command for FS72_ExecuteCommand with opcode 0.
    // Expects as inputs:
    // value_2C = offset from beginning of the nitro data to start copying from
    // value_30 = destination to load data to
    // value_38 = number of bytes to load.
    // Outputs:
    // The value_2C is adjusted by the number of bytes loaded.
    int FS72_Command_Load(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 1.
    // Very likely completely unused since nitro files are read-only.
    // Expects as inputs:
    // value_2C = offset from beginning of the nitro data to start copying to
    // value_30 = source data to copy
    // value_38 = number of bytes to copy
    // Outputs:
    // value_2C is adjusted by the number of bytes saved.
    int FS72_Command_Save(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 2.
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
    int FS72_Command_GetDirectoryData(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 3.
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
    int FS72_Command_GetFileOrDirectoryNameData(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 4.
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
    // Into reg9 we write the following data.
    // * If we found a file: { nitro handle (ptr), file id (u32) }
    // * If we found a directory: { 
    //       nitro handle (ptr);
    //       dir id (u16);
    //       first file id (u16);
    //       subtable offset in handle (u32)
    //   }
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
    int FS72_Command_GetFileOrDirectoryByName(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 5.
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
    int FS72_Command_GetPath(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 6.
    // Loads data from the file allocation table.
    // Inputs:
    // ext_b: file id / index in the table
    // Outputs:
    // base_a and ext_c: file id
    // base_b, base_d and ext_a: beginning of allocation
    // base_c and ext_b: end of allocation
    int FS72_Command_GetFATEntry(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 7.
    // Copies data from the extended registers into the base registers.
    // Specifically:
    // ext_a is copied into base_b and base_d
    // ext_b is copied into base_c
    // ext_c is copied into base_a
    int FS72_Command_CopyExtendedRegisters(FSStruct72* fs);

    // Default command for FS72_ExecuteCommand with opcode 8.
    // A simple nop.
    int FS72_Command_Nop(FSStruct72* fs);
}

struct FSStruct76
{
    unsigned short unknown_0;
    unsigned short unknown_2;
    FSStruct72 inner;
};

