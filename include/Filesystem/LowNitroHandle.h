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

    // Returns the amount of space used or needed. If this exceeds the
    // capacity, nothing will be written.
    unsigned int NitroHandle_GetFileTables(NitroHandle*, void* into, unsigned int capacity);

    // Removes all references to the file tables, then returns the pointer
    // to the allocation so that you can free the memory
    void* NitroHandle_ReleaseFileTables(NitroHandle*);

    int NitroHandle_UnknownDestructionFunction(NitroHandle*);
    int NitroHandle_OtherUnknownDestFunc(NitroHandle*);

    void NitroHandle_SetOpcodeOverride(NitroHandle*, PFNExecuteCommand, unsigned int mask);

    void NitroHandle_020cc6ac(NitroHandle*, int result);
}