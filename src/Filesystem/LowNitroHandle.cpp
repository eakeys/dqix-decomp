#include "Filesystem/FSInnerDefs.h"
#include "System/Memory.h"
#include "System/Interrupts.h"
#include <globaldefs.h>

#pragma optimize_for_size off


extern "C" {
    int DefaultNitroLoadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len);
    int DefaultNitroSaveProc(NitroHandle* handle, const void* src, unsigned int offset, unsigned int len);
    int OverrideNitroLoadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len);
}

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

    NitroHandle* handle = data_02111728.handle;

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
        if (data_02111728.handle == NULL)
        {
            data_02111728.handle = nitro;
            data_02111728.primaryFSRoot.handle = nitro;
            data_02111728.primaryFSRoot.handleSubtableOffset = 0;
            data_02111728.primaryFSRoot.firstFileID = 0;
            data_02111728.primaryFSRoot.dirID = 0;
        }
        else
        {
            // At the end of this, currentEntry holds the last entry in the list
            // and nextEntry is null (points past the end)
            NitroHandle* currentEntry = data_02111728.handle;
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
        nitro->flags |= 1;
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
    nitro->flags &= ~1;

    if (data_02111728.primaryFSRoot.handle == nitro)
    {
        data_02111728.primaryFSRoot.handle = data_02111728.handle;
        data_02111728.primaryFSRoot.handleSubtableOffset = 0;
        data_02111728.primaryFSRoot.firstFileID = 0;
        data_02111728.primaryFSRoot.dirID = 0;
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

    nitro->loadFileProc_48 = (loadProc == NULL) ? &DefaultNitroLoadProc : loadProc;
    nitro->saveFileProc = (saveProc == NULL) ? &DefaultNitroSaveProc : saveProc;

    nitro->loadFileProc_50 = nitro->loadFileProc_48;
    nitro->tableRawPointer = NULL;
    nitro->flags |= (1 << 1);
    return 1;
}

extern "C" int NitroHandle_Destroy(NitroHandle* nitro)
{
    int oldState = DisableIRQInterrupts();
    
    if (GET_FLAG_BIT(nitro->flags, 1) != 0)
    {
        // Silly unused volatile read of the flags. Assembly is extra misleading,
        // can make it look like it's the 2nd argument, but this is wrong
        int destructionThingHappened = (nitro->flags, NitroHandle_UnknownDestructionFunction(nitro));
        nitro->flags |= (1 << 7);
        
        NitroVM* fs = nitro->linkToFirstVM.pNext;
        if (fs != NULL)
        {
            do {
                NitroVM* next = fs->links.pNext;
                NitroVM_UnlinkAndStoreResult(fs, 3);
                fs = next;
            } while (fs != NULL);
        }
        nitro->linkToFirstVM.pNext = NULL;
        if (destructionThingHappened)
            NitroHandle_OtherUnknownDestFunc(nitro);

        nitro->pFileImage = NULL;
        nitro->fatOffset_2C = 0;
        nitro->fatSize = 0;
        nitro->nameTableOffset_34 = 0;
        nitro->nameTableSize = 0;
        nitro->nameTableOffset_40 = 0;
        nitro->fatOffset_3C = 0;

        nitro->flags &= ~((1 << 7) | (1 << 5) | (1 << 1));
    }
    SetIRQInterruptState(oldState);
    return 1;
}

extern "C" unsigned int NitroHandle_LoadFileTables(NitroHandle* nitro, void* into, unsigned int capacity)
{
    unsigned int neededSpace = (nitro->fatSize + nitro->nameTableSize + 0x3f) & ~0x1f;

    if (neededSpace <= capacity)
    {
        void* alignedPtr = (void*)(((unsigned int)into + 0x1f) & ~0x1f);
        NitroVM vm;
        NitroVM_Initialize(&vm);
        if (NitroVM_PrepareRead(&vm, nitro, 
            nitro->fatOffset_2C,
            nitro->fatOffset_2C + nitro->fatSize, -1))
        {
            if (NitroVM_MaybeExecuteLoad_v0(&vm, alignedPtr, nitro->fatSize) < 0)
                VectorizedMemset(alignedPtr, 0, nitro->fatSize);
            NitroVM_MaybeCompleteTasks_020cca80(&vm);
        }
        nitro->fatOffset_2C = (unsigned int)alignedPtr;
        alignedPtr = (void*)((unsigned int)alignedPtr + nitro->fatSize);

        if (NitroVM_PrepareRead(&vm, nitro, 
            nitro->nameTableOffset_34,
            nitro->nameTableOffset_34 + nitro->nameTableSize, -1))
        {
            if (NitroVM_MaybeExecuteLoad_v0(&vm, alignedPtr, nitro->nameTableSize) < 0)
                VectorizedMemset(alignedPtr, 0, nitro->nameTableSize);
            NitroVM_MaybeCompleteTasks_020cca80(&vm);
        }

        nitro->nameTableOffset_34 = (unsigned int)alignedPtr;
        nitro->tableRawPointer = into;
        nitro->loadFileProc_50 = &OverrideNitroLoadProc;
        nitro->flags |= (1 << 2);
    }

    return neededSpace;
}

extern "C" void* NitroHandle_ReleaseFileTables(NitroHandle* nitro)
{
    void* oldPtr = NULL;
    if (GET_FLAG_BIT(nitro->flags, 1))
    {
        int destructionThingHappened = NitroHandle_UnknownDestructionFunction(nitro);
        if (GET_FLAG_BIT(nitro->flags, 2))
        {
            nitro->flags &= ~(1 << 2);
            oldPtr = nitro->tableRawPointer;
            nitro->tableRawPointer = NULL;
            nitro->fatOffset_2C = nitro->fatOffset_3C;
            nitro->nameTableOffset_34 = nitro->nameTableOffset_40;
            nitro->loadFileProc_50 = nitro->loadFileProc_48;
        }
        if (destructionThingHappened)
            NitroHandle_OtherUnknownDestFunc(nitro);
    }
    return oldPtr;
}

extern "C" CBool NitroHandle_UnknownDestructionFunction(NitroHandle* nitro)
{
    int oldState = DisableIRQInterrupts();
    CBool act = GET_FLAG_BIT(nitro->flags, 3) == 0;
    if (act)
    {
        int flagbit4 = GET_FLAG_BIT(nitro->flags, 4);
        int oldFlags = nitro->flags;
        if (flagbit4)
        {
            nitro->flags = oldFlags | (1 << 6);
            do {
                func_020c7898(&nitro->unknown_14);
            } while (GET_FLAG_BIT(nitro->flags, 6));
        }
        else
        {
            nitro->flags = oldFlags | (1 << 3);
        }
    }

    SetIRQInterruptState(oldState);
    return act;
}

extern "C" CBool NitroHandle_OtherUnknownDestFunc(NitroHandle* nitro)
{
    NitroVM* fs = NULL;
    int oldState = DisableIRQInterrupts();

    CBool skip = GET_FLAG_BIT(nitro->flags, 3) == 0;
    if (!skip)
    {
        nitro->flags &= ~(1 << 3);
        fs = NitroHandle_020cbc6c(nitro);
    }

    SetIRQInterruptState(oldState);
    if (fs != NULL)
        NitroVM_020cbe80(fs);
    return skip;
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
    if (GET_FLAG_BIT(nitro->flags, 8))
    {
        int previousFlags = nitro->flags;
        NitroVM* firstVM = nitro->linkToFirstVM.pNext;
        nitro->flags = previousFlags & ~(1 << 8);
        NitroVM_UnlinkAndStoreResult(firstVM, result);
        NitroVM* fs = NitroHandle_020cbc6c(nitro);
        if (fs != NULL)
            NitroVM_020cbe80(fs);
    }
    else
    {
        NitroVM* firstVM = nitro->linkToFirstVM.pNext;
        int oldState = DisableIRQInterrupts();
        firstVM->storedResult = result;
        nitro->flags &= ~(1 << 9);
        func_020c78e8(&nitro->unknown_0C);
        SetIRQInterruptState(oldState);
    }
}

extern "C" void InitializeCartridgeFilesystem(int unknown)
{
    if (data_02111738)
        return;
    data_02111738 = true;
    InitializeROMFilesystem_Internal(unknown);
}

extern "C" void NitroVM_Initialize(NitroVM* vm)
{
    vm->links.pPrev = NULL;
    vm->links.pNext = NULL;
    vm->unknown_sublist_18.pLast = NULL;
    vm->unknown_sublist_18.pFirst = NULL;
    vm->linkedHandle = NULL;
    vm->maybeScheduledCommand = 14;
    vm->flags = 0;
}

extern "C" int GlobalSearchFileOrDirectory_020cc780(NitroVM* vm, const char* inPath,
        NitroFileAccessor* outFileData, NitroDirectoryAccessor* outDirData)
{
    NitroDirectoryAccessor accessor;
    // Cast to unsigned and remove const for convenience
    unsigned char* path = (unsigned char*)inPath;
    if (path[0] == '/' || path[0] == '\\')
    {
        accessor.handle = data_0211172c.handle;
        accessor.dirID = 0;
        accessor.handleSubtableOffset = 0;
        accessor.firstFileID = 0;
        path++;
    }
    else
    {
        accessor = data_0211172c;
        int signatureIdx = 0;
        do
        {
            if (path[signatureIdx] == '\0' || path[signatureIdx] == '/' || path[signatureIdx] == '\\')
                break;
            if (path[signatureIdx] != ':')
                continue;
            
            NitroHandle* handle = NitroHandle_FindBySignature(inPath, signatureIdx);

            if (handle == NULL)
                return false;
            if (!GET_FLAG_BIT(handle->flags, 1))
                return false;

            accessor.handle = handle;
            accessor.handleSubtableOffset = 0;
            accessor.firstFileID = 0;
            accessor.dirID = 0;
            path += signatureIdx + 1;
            if (path[0] == '/' || path[0] == '\\')
                path++;
            break;
        } while (++signatureIdx <= 3);
    }

    vm->linkedHandle = accessor.handle;
    vm->regext_d.ptr = (char*)path;
    vm->regext_abc = *(FSRegisterTriple*)&accessor;
    if (outDirData != NULL)
    {
        vm->reg8.u32 = 1; // search for directory
        vm->reg9.ptr = outDirData;
    }
    else
    {
        vm->reg8.u32 = 0; // search for file
        vm->reg9.ptr = outFileData;
    }

    return NitroVM_020cbf58(vm, 4);
}

#define min(a, b) (((a) > (b)) ? (b) : (a))
#define max(a, b) (((a) < (b)) ? (b) : (a))

// This is not quite right, registers are wrong but it's logically correct
// and I'm moving on for now.
/*extern "C" int NitroVM_ExecuteLoad_020cc8c4(NitroVM* vm, void* dst, int capacity, int unknownBool)
{ 
    // base_d is like seek / tell index, adjusted by load commands
    int srcStart = vm->regbase_d.s32;
    int srcLength = vm->regbase_abc.c.s32 - vm->regbase_d.s32;

    unsigned int lengthToCopy;
    lengthToCopy = capacity;
    if ((int)lengthToCopy > srcLength)
        lengthToCopy = srcLength;
    
    if ((int)lengthToCopy < 0)
        lengthToCopy = 0;

    vm->regext_abc.a.ptr = dst;
    vm->regext_abc.b.s32 = capacity;
    vm->regext_abc.c.u32 = lengthToCopy;

    if (!unknownBool)
        vm->flags |= (1 << 2);

    NitroVM_020cbf58(vm, 0);

    if (!unknownBool)
    {
        if (func_020ccae8(vm))
            lengthToCopy = vm->regbase_d.u32 - srcStart;
        else
            lengthToCopy = -1;
    }

    return lengthToCopy;
}*/