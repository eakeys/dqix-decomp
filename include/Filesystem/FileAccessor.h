#pragma once

#include "FSStructs.h"

extern "C"
{
    CBool CreateFileAccessor(NitroFileAccessor* outAccessor, const char* path);

    CBool NitroVM_PrepareRead(NitroVM* vm, NitroHandle* handle, unsigned int start, unsigned int end, unsigned int capacity);

    // Sets up the appropriate registers to point to the relevant parts, so that 
    // the handle load proc can load the file
    CBool NitroVM_PrepareReadFileByID(NitroVM* vm, NitroFileAccessor accessor);

    CBool NitroVM_PrepareReadFileByPath(NitroVM* vm, const char* path);

    // I don't know what this function actually does, but I think it has 
    // some synchronisation purpose. It is often called at destruction time,
    // but it also sometimes gets called in the middle of a function (see e.g.
    // func_020ccffc in USA). 
    CBool NitroVM_MaybeCompleteTasks_020cca80(NitroVM* vm);

    // Assuming that register base_A holds the file ID, runs command 5 to output
    // the filename to the specified buffer
    void NitroVM_WriteOutFilePath(NitroVM* vm, char* buffer, unsigned int bufferLength);

    CBool NitroVM_020ccae8(NitroVM* vm);
    void NitroVM_FlagStuff_020ccba8(NitroVM* vm);

    int NitroVM_MaybeExecuteLoad_v1(NitroVM*, void* dst, int capacity);
    int NitroVM_MaybeExecuteLoad_v0(NitroVM*, void* dst, int capacity);

    // this is (probably) like libc fseek(). Assumes that base_b
    // holds the start address, base_c the end address and base_d the
    // current position, and updates base_d appropriately.
    // 0 = SEEK_SET (from start), 1 = SEEK_CUR (adjust), 2 = SEEK_END
    CBool NitroVM_Seek(NitroVM* vm, int position, int whence);

    CBool SetPrimaryFilesystemRoot(const char* path);

    void NitroHandle_020cccd4(NitroHandle*);
    int ROMFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned);
    int ROMFilesystemSaveProc(NitroHandle*, const void*, unsigned, unsigned); // doesn't do anything!

    int ROMFilesystemOpcodeOverride(NitroVM*, int opcode);

    int UnsupportedFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned);
    int UnsupportedFilesystemOpcodeOverride(NitroVM*, int opcode);

    void InitializeROMFilesystem_Internal(unsigned int unknown);

    unsigned int MaybeChangeDMAChannel_020ccf0c(unsigned int newChannel);

    // Load file tables into memory for the primary nitro handle (ROM FS),
    // provided the buffer is large enough. Returns the amount of space needed.
    unsigned int LoadPrimaryFileTables(void* where, unsigned int bufferLen);
}