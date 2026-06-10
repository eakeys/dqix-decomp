#pragma once

#include "FSStructs.h"

extern "C"
{
    void NitroHandle_ZeroInit(NitroHandle*);
    NitroHandle* NitroHandle_FindBySignature(const char* str, int len);
    int NitroHandle_AddToList(NitroHandle*, const char* sig, int len);
    void NitroHandle_RemoveFromList(NitroHandle*);

    int NitroHandle_Populate(NitroHandle*, void* image,
        unsigned int fatOffset, unsigned int fatSize,
        unsigned int fntOffset, unsigned int fntSize,
        PFNLoadFile loadProc, PFNSaveFile saveProc);

    int NitroHandle_Destroy(NitroHandle*);

    // Loads the file tables (FAT and FNT) into memory at the given
    // position. The memory should already be allocated in some way.
    // Returns the amount of space used or needed. If this exceeds the
    // capacity, nothing will be written.
    unsigned int NitroHandle_LoadFileTables(NitroHandle*, void* into, unsigned int capacity);

    // Removes all references to the file tables, then returns the pointer
    // to the allocation so that you can free the memory
    void* NitroHandle_ReleaseFileTables(NitroHandle*);

    CBool NitroHandle_UnknownDestructionFunction(NitroHandle*);
    CBool NitroHandle_OtherUnknownDestFunc(NitroHandle*);

    void NitroHandle_SetOpcodeOverride(NitroHandle*, PFNExecuteCommand, unsigned int mask);

    void NitroHandle_020cc6ac(NitroHandle*, int result);

    void InitializeCartridgeFilesystem(int unknown);

    void NitroVM_Initialize(NitroVM*);

    // Can be used with a temporary VM, and doesn't need a nitro handle
    // attached - it will be assigned by the function itself
    int GlobalSearchFileOrDirectory_020cc780(NitroVM*, const char* path,
        NitroFileAccessor* outFileData, NitroDirectoryAccessor* outDirData);

    // Returns number of bytes loaded
    int NitroVM_ExecuteLoad_020cc8c4(NitroVM*, void* dst, int capacity, CBool unknownBool);
}