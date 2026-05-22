#include "Filesystem/FSStructs.h"
#include "System/Memory.h"
#include "System/Interrupts.h"
#include "Filesystem/LowNitroHandle.h"
#include <globaldefs.h>

#pragma optimize_for_size off

#define GET_FLAG_BIT(what, idx) (((what) & (1 << (idx))) ? 1 : 0)

extern "C" {
int func_020cbc18(NitroHandle*, void*, unsigned, unsigned);
int func_020cbc34(NitroHandle*, const void*, unsigned, unsigned);
int func_020cc61c(NitroHandle*);
void func_020cc758(NitroVM*);
bool func_020cc980(NitroVM*, NitroHandle*, unsigned int start, unsigned int end, int);

int func_020ccc00(NitroVM*, void* dst, unsigned int len);
bool func_020cca80(NitroVM*);

int func_020cbc54(NitroHandle*, void*, unsigned int, unsigned int);

void func_020c7898(void*);
void func_020c78e8(void*);

    NitroVM* func_020cbc6c(NitroHandle*);
    void func_020cbe80(NitroVM*);
}

// sizeof == 0x2C == 44 probably.
struct Struct_02111728
{
    NitroHandle* first;
    NitroHandle* lastMaybe;
    unsigned short unknown_8;
    unsigned short unknown_A;
    unsigned int unknown_C;
};

extern Struct_02111728 data_02111728;
extern int data_0211172c;

extern "C" void NitroHandle_ZeroInit(NitroHandle* nitro)
{
    VectorizedMemset(nitro, 0, sizeof(NitroHandle));
    nitro->unknown_10 = 0;
    nitro->unknown_0C = 0;
    nitro->unknown_18 = 0;
    nitro->unknown_14 = 0;
}

extern "C" NitroHandle* NitroHandle_FindBySignature(const char* str, int len)
{
    unsigned int targetSig = Nitro_CalculateSignature(str, len);
    int oldState = DisableIRQInterrupts();

    NitroHandle* handle = data_02111728.first;

    while (handle != NULL && handle->signature != targetSig)
    {
        handle = handle->pNeighbor4;
    }

    SetIRQInterruptState(oldState);
    return handle;
}

extern "C" int NitroHandle_AddToList(NitroHandle* nitro, const char* str, int len)
{
    int result = false;
    int oldState = DisableIRQInterrupts();
    if (NitroHandle_FindBySignature(str, len) == NULL)
    {
        if (data_02111728.first == NULL)
        {
            data_02111728.first = nitro;
            data_02111728.lastMaybe = nitro;
            data_02111728.unknown_C = 0;
            data_02111728.unknown_A = 0;
            data_02111728.unknown_8 = 0;
        }
        else
        {
            // At the end of this, currentEntry holds the last entry in the list
            // and nextEntry is null (points past the end)
            NitroHandle* currentEntry = data_02111728.first;
            NitroHandle* nextEntry = currentEntry->pNeighbor4;
            if (nextEntry != NULL)
            {
                do
                {
                    currentEntry = nextEntry;
                    nextEntry = nextEntry->pNeighbor4;
                } while (nextEntry != NULL);
            }

            currentEntry->pNeighbor4 = nitro;
            nitro->pNeighbor8 = currentEntry;
        }
        nitro->signature = Nitro_CalculateSignature(str, len);
        result = true;
        nitro->flags_1C |= 1;
    }

    SetIRQInterruptState(oldState);
    return result;
}

extern "C" void NitroHandle_RemoveFromList(NitroHandle* nitro)
{
    if (nitro->signature == 0)
        return;

    int oldState = DisableIRQInterrupts();

    if (nitro->pNeighbor4 != NULL)
        nitro->pNeighbor4->pNeighbor8 = nitro->pNeighbor8;

    if (nitro->pNeighbor8 != NULL)
        nitro->pNeighbor8->pNeighbor4 = nitro->pNeighbor4;

    nitro->signature = 0;
    nitro->pNeighbor8 = NULL;
    nitro->pNeighbor4 = NULL;
    nitro->flags_1C &= ~1;

    if (data_02111728.lastMaybe == nitro)
    {
        data_02111728.lastMaybe = data_02111728.first;
        data_02111728.unknown_C = 0;
        data_02111728.unknown_A = 0;
        data_02111728.unknown_8 = 0;
    }

    SetIRQInterruptState(oldState);
}

extern "C" int NitroHandle_Populate(NitroHandle* nitro, void* image,
        unsigned int fatOffset, unsigned int fatSize,
        unsigned int fntOffset, unsigned int fntSize,
        PFNLoadFile loadProc, PFNSaveFile saveProc)
{
    nitro->pFileImage = image;
    nitro->fatSize = fatSize;
    nitro->fatOffset_2C = nitro->fatOffset_3C = fatOffset;
    nitro->nameTableSize = fntSize;
    nitro->nameTableOffset_40 = fntOffset;
    nitro->nameTableOffset_34 = fntOffset;

    nitro->loadFileProc_48 = (loadProc == NULL) ? &func_020cbc18 : loadProc;
    nitro->saveFileProc = (saveProc == NULL) ? &func_020cbc34 : saveProc;

    nitro->loadFileProc_50 = nitro->loadFileProc_48;
    nitro->maybeTablePtr = 0;
    nitro->flags_1C |= (1 << 1);
    return 1;
}

extern "C" int NitroHandle_Destroy(NitroHandle* nitro)
{
    int oldState = DisableIRQInterrupts();
    
    if (GET_FLAG_BIT(nitro->flags_1C, 1) != 0)
    {
        // Silly unused volatile read of the flags. Assembly is extra misleading,
        // can make it look like it's the 2nd argument, but this is wrong
        int destructionThingHappened = (nitro->flags_1C, NitroHandle_UnknownDestructionFunction(nitro));
        nitro->flags_1C |= (1 << 7);
        
        NitroVM* fs = nitro->fs_24;
        if (fs != NULL)
        {
            do {
                NitroVM* next = fs->pNext;
                FS72_PopAndUpdateResult(fs, 3);
                fs = next;
            } while (fs != NULL);
        }
        nitro->fs_24 = NULL;
        if (destructionThingHappened)
            NitroHandle_OtherUnknownDestFunc(nitro);

        nitro->pFileImage = NULL;
        nitro->fatOffset_2C = 0;
        nitro->fatSize = 0;
        nitro->nameTableOffset_34 = 0;
        nitro->nameTableSize = 0;
        nitro->nameTableOffset_40 = 0;
        nitro->fatOffset_3C = 0;

        nitro->flags_1C &= ~((1 << 7) | (1 << 5) | (1 << 1));
    }
    SetIRQInterruptState(oldState);
    return 1;
}

extern "C" unsigned int NitroHandle_GetFileTables(NitroHandle* nitro, void* into, unsigned int capacity)
{
    unsigned int neededSpace = (nitro->fatSize + nitro->nameTableSize + 0x3f) & ~0x1f;

    if (neededSpace <= capacity)
    {
        void* alignedPtr = (void*)(((unsigned int)into + 0x1f) & ~0x1f);
        NitroVM fs;
        func_020cc758(&fs);
        if (func_020cc980(&fs, nitro, 
            nitro->fatOffset_2C,
            nitro->fatOffset_2C + nitro->fatSize, -1))
        {
            if (func_020ccc00(&fs, alignedPtr, nitro->fatSize) < 0)
                VectorizedMemset(alignedPtr, 0, nitro->fatSize);
            func_020cca80(&fs);
        }
        nitro->fatOffset_2C = (unsigned int)alignedPtr;
        alignedPtr = (void*)((unsigned int)alignedPtr + nitro->fatSize);

        if (func_020cc980(&fs, nitro, 
            nitro->nameTableOffset_34,
            nitro->nameTableOffset_34 + nitro->nameTableSize, -1))
        {
            if (func_020ccc00(&fs, alignedPtr, nitro->nameTableSize) < 0)
                VectorizedMemset(alignedPtr, 0, nitro->nameTableSize);
            func_020cca80(&fs);
        }

        nitro->nameTableOffset_34 = (unsigned int)alignedPtr;
        nitro->maybeTablePtr = into;
        nitro->loadFileProc_50 = &func_020cbc54;
        nitro->flags_1C |= (1 << 2);
    }

    return neededSpace;
}

extern "C" void* NitroHandle_ReleaseFileTables(NitroHandle* nitro)
{
    void* oldPtr = NULL;
    if (GET_FLAG_BIT(nitro->flags_1C, 1))
    {
        int destructionThingHappened = NitroHandle_UnknownDestructionFunction(nitro);
        if (GET_FLAG_BIT(nitro->flags_1C, 2))
        {
            nitro->flags_1C &= ~(1 << 2);
            oldPtr = nitro->maybeTablePtr;
            nitro->maybeTablePtr = NULL;
            nitro->fatOffset_2C = nitro->fatOffset_3C;
            nitro->nameTableOffset_34 = nitro->nameTableOffset_40;
            nitro->loadFileProc_50 = nitro->loadFileProc_48;
        }
        if (destructionThingHappened)
            NitroHandle_OtherUnknownDestFunc(nitro);
    }
    return oldPtr;
}

extern "C" int NitroHandle_UnknownDestructionFunction(NitroHandle* nitro)
{
    int oldState = DisableIRQInterrupts();
    int act = GET_FLAG_BIT(nitro->flags_1C, 3) == 0;
    if (act)
    {
        int flagbit4 = GET_FLAG_BIT(nitro->flags_1C, 4);
        int oldFlags = nitro->flags_1C;
        if (flagbit4)
        {
            nitro->flags_1C = oldFlags | (1 << 6);
            do {
                func_020c7898(&nitro->unknown_14);
            } while (GET_FLAG_BIT(nitro->flags_1C, 6));
        }
        else
        {
            nitro->flags_1C = oldFlags | (1 << 3);
        }
    }

    SetIRQInterruptState(oldState);
    return act;
}

extern "C" int NitroHandle_OtherUnknownDestFunc(NitroHandle* nitro)
{
    NitroVM* fs = NULL;
    int oldState = DisableIRQInterrupts();

    int act = GET_FLAG_BIT(nitro->flags_1C, 3) == 0;
    if (!act)
    {
        nitro->flags_1C &= ~(1 << 3);
        fs = func_020cbc6c(nitro);
    }

    SetIRQInterruptState(oldState);
    if (fs != NULL)
        func_020cbe80(fs);
    return act;
}

extern "C" void NitroHandle_SetOpcodeOverride(NitroHandle* nitro, PFNExecuteCommand fnOverride, unsigned int mask)
{
    if (mask == 0)
        fnOverride = NULL;
    else if (fnOverride == NULL)
        mask = 0;
    
    nitro->instructionOverride = fnOverride;
    nitro->overrideOpcodeFlags = mask;
}

extern "C" void NitroHandle_020cc6ac(NitroHandle* nitro, int result)
{
    if (GET_FLAG_BIT(nitro->flags_1C, 8))
    {
        int previousFlags = nitro->flags_1C;
        NitroVM* yes = nitro->fs_24;
        nitro->flags_1C = previousFlags & ~(1 << 8);
        FS72_PopAndUpdateResult(yes, result);
        NitroVM* fs = func_020cbc6c(nitro);
        if (fs != NULL)
            func_020cbe80(fs);
    }
    else
    {
        NitroVM* fs = nitro->fs_24;
        int oldState = DisableIRQInterrupts();
        fs->storedResult = result;
        nitro->flags_1C &= ~(1 << 9);
        func_020c78e8(&nitro->unknown_0C);
        SetIRQInterruptState(oldState);
    }
}