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

extern "C" void NitroHandle_ZeroInit(NitroHandle* handle)
{
    VectorizedMemset(handle, 0, sizeof(NitroHandle));
    handle->unknown_10 = 0;
    handle->unknown_0C = 0;
    handle->unknown_18 = 0;
    handle->unknown_14 = 0;
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

extern "C" int NitroHandle_AddToList(NitroHandle* handle, const char* str, int len)
{
    int result = false;
    int oldState = DisableIRQInterrupts();
    if (NitroHandle_FindBySignature(str, len) == NULL)
    {
        if (data_02111728.handle == NULL)
        {
            data_02111728.handle = handle;
            data_02111728.primaryFSRoot.handle = handle;
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

            currentEntry->pNeighbor4 = handle;
            handle->pNeighbor8 = currentEntry;
        }
        handle->signature = Nitro_CalculateSignature(str, len);
        result = true;
        handle->flags |= (1 << NITROHANDLE_FLAG_IS_IN_MAIN_LIST);
    }

    SetIRQInterruptState(oldState);
    return result;
}

extern "C" void NitroHandle_RemoveFromList(NitroHandle* handle)
{
    if (handle->signature == 0)
        return;

    int oldState = DisableIRQInterrupts();

    if (handle->pNeighbor4 != NULL)
        handle->pNeighbor4->pNeighbor8 = handle->pNeighbor8;

    if (handle->pNeighbor8 != NULL)
        handle->pNeighbor8->pNeighbor4 = handle->pNeighbor4;

    handle->signature = 0;
    handle->pNeighbor8 = NULL;
    handle->pNeighbor4 = NULL;
    handle->flags &= ~(1 << NITROHANDLE_FLAG_IS_IN_MAIN_LIST);

    if (data_02111728.primaryFSRoot.handle == handle)
    {
        data_02111728.primaryFSRoot.handle = data_02111728.handle;
        data_02111728.primaryFSRoot.handleSubtableOffset = 0;
        data_02111728.primaryFSRoot.firstFileID = 0;
        data_02111728.primaryFSRoot.dirID = 0;
    }

    SetIRQInterruptState(oldState);
}

extern "C" CBool NitroHandle_Populate(NitroHandle* handle, void* image,
        unsigned int fatOffset, unsigned int fatSize,
        unsigned int fntOffset, unsigned int fntSize,
        PFNLoadFile loadProc, PFNSaveFile saveProc)
{
    handle->pFileImage = image;
    handle->fatSize = fatSize;
    handle->fatOffset_2C = handle->fatOffset_3C = fatOffset;
    handle->nameTableSize = fntSize;
    handle->nameTableOffset_40 = fntOffset;
    handle->nameTableOffset_34 = fntOffset;

    handle->loadFileProc_48 = (loadProc == NULL) ? &DefaultNitroLoadProc : loadProc;
    handle->saveFileProc = (saveProc == NULL) ? &DefaultNitroSaveProc : saveProc;

    handle->loadFileProc_50 = handle->loadFileProc_48;
    handle->tableRawPointer = NULL;
    handle->flags |= (1 << NITROHANDLE_FLAG_IS_POPULATED);
    return true;
}

extern "C" CBool NitroHandle_Destroy(NitroHandle* handle)
{
    int oldState = DisableIRQInterrupts();
    
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_IS_POPULATED) != 0)
    {
        // Silly unused volatile read of the flags. Assembly is extra misleading,
        // can make it look like it's the 2nd argument, but this is wrong
        CBool destructionThingHappened = (handle->flags, NitroHandle_UnknownDestructionFunction(handle));
        handle->flags |= (1 << NITROHANDLE_FLAG_7);
        
        NitroVM* machine = handle->linkToFirstVM.pNext;
        if (machine != NULL)
        {
            do {
                NitroVM* next = machine->links.pNext;
                NitroVM_UnlinkAndStoreResult(machine, NITRO_RESULT_MAYBE_INVALID_HANDLE);
                machine = next;
            } while (machine != NULL);
        }
        handle->linkToFirstVM.pNext = NULL;
        if (destructionThingHappened)
            NitroHandle_OtherUnknownDestFunc(handle);

        handle->pFileImage = NULL;
        handle->fatOffset_2C = 0;
        handle->fatSize = 0;
        handle->nameTableOffset_34 = 0;
        handle->nameTableSize = 0;
        handle->nameTableOffset_40 = 0;
        handle->fatOffset_3C = 0;

        handle->flags &= ~((1 << NITROHANDLE_FLAG_7) | (1 << NITROHANDLE_FLAG_5) | (1 << NITROHANDLE_FLAG_IS_POPULATED));
    }
    SetIRQInterruptState(oldState);
    return true;
}

extern "C" unsigned int NitroHandle_LoadFileTables(NitroHandle* handle, void* into, unsigned int capacity)
{
    unsigned int neededSpace = (handle->fatSize + handle->nameTableSize + 0x3f) & ~0x1f;

    if (neededSpace <= capacity)
    {
        void* alignedPtr = (void*)(((unsigned int)into + 0x1f) & ~0x1f);
        NitroVM vm;
        NitroVM_Initialize(&vm);
        if (NitroVM_PrepareRead(&vm, handle, 
            handle->fatOffset_2C,
            handle->fatOffset_2C + handle->fatSize, -1))
        {
            if (NitroVM_MaybeExecuteLoad_v0(&vm, alignedPtr, handle->fatSize) < 0)
                VectorizedMemset(alignedPtr, 0, handle->fatSize);
            NitroVM_MaybeCompleteTasks_020cca80(&vm);
        }
        handle->fatOffset_2C = (unsigned int)alignedPtr;
        alignedPtr = (void*)((unsigned int)alignedPtr + handle->fatSize);

        if (NitroVM_PrepareRead(&vm, handle, 
            handle->nameTableOffset_34,
            handle->nameTableOffset_34 + handle->nameTableSize, -1))
        {
            if (NitroVM_MaybeExecuteLoad_v0(&vm, alignedPtr, handle->nameTableSize) < 0)
                VectorizedMemset(alignedPtr, 0, handle->nameTableSize);
            NitroVM_MaybeCompleteTasks_020cca80(&vm);
        }

        handle->nameTableOffset_34 = (unsigned int)alignedPtr;
        handle->tableRawPointer = into;
        handle->loadFileProc_50 = &OverrideNitroLoadProc;
        handle->flags |= (1 << NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY);
    }

    return neededSpace;
}

extern "C" void* NitroHandle_ReleaseFileTables(NitroHandle* handle)
{
    void* oldPtr = NULL;
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_IS_POPULATED))
    {
        int destructionThingHappened = NitroHandle_UnknownDestructionFunction(handle);
        if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY))
        {
            handle->flags &= ~(1 << NITROHANDLE_FLAG_TABLES_LOADED_IN_MEMORY);
            oldPtr = handle->tableRawPointer;
            handle->tableRawPointer = NULL;
            handle->fatOffset_2C = handle->fatOffset_3C;
            handle->nameTableOffset_34 = handle->nameTableOffset_40;
            handle->loadFileProc_50 = handle->loadFileProc_48;
        }
        if (destructionThingHappened)
            NitroHandle_OtherUnknownDestFunc(handle);
    }
    return oldPtr;
}

extern "C" CBool NitroHandle_UnknownDestructionFunction(NitroHandle* handle)
{
    int oldState = DisableIRQInterrupts();
    CBool act = GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_3) == 0;
    if (act)
    {
        int flagbit4 = GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_4);
        int oldFlags = handle->flags;
        if (flagbit4)
        {
            handle->flags = oldFlags | (1 << NITROHANDLE_FLAG_6);
            do {
                func_020c7898(&handle->unknown_14);
            } while (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_6));
        }
        else
        {
            handle->flags = oldFlags | (1 << NITROHANDLE_FLAG_3);
        }
    }

    SetIRQInterruptState(oldState);
    return act;
}

extern "C" CBool NitroHandle_OtherUnknownDestFunc(NitroHandle* handle)
{
    NitroVM* machine = NULL;
    int oldState = DisableIRQInterrupts();

    CBool skip = GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_3) == 0;
    if (!skip)
    {
        handle->flags &= ~(1 << NITROHANDLE_FLAG_3);
        machine = NitroHandle_020cbc6c(handle);
    }

    SetIRQInterruptState(oldState);
    if (machine != NULL)
        NitroVM_020cbe80(machine);
    return skip;
}

extern "C" void NitroHandle_SetOpcodeOverride(NitroHandle* handle, PFNExecuteCommand fnOverride, unsigned int mask)
{
    if (mask == 0)
        fnOverride = NULL;
    else if (fnOverride == NULL)
        mask = 0;
    
    handle->instructionOverride = fnOverride;
    handle->overrideOpcodeFlags = mask;
}

extern "C" void NitroHandle_020cc6ac(NitroHandle* handle, int result)
{
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_8))
    {
        int previousFlags = handle->flags;
        NitroVM* firstVM = handle->linkToFirstVM.pNext;
        handle->flags = previousFlags & ~(1 << NITROHANDLE_FLAG_8);
        NitroVM_UnlinkAndStoreResult(firstVM, result);
        NitroVM* fs = NitroHandle_020cbc6c(handle);
        if (fs != NULL)
            NitroVM_020cbe80(fs);
    }
    else
    {
        NitroVM* firstVM = handle->linkToFirstVM.pNext;
        int oldState = DisableIRQInterrupts();
        firstVM->storedResult = result;
        handle->flags &= ~(1 << NITROHANDLE_FLAG_9);
        func_020c78e8(&handle->unknown_0C);
        SetIRQInterruptState(oldState);
    }
}

extern "C" void InitializeCartridgeFilesystem(int dmaChannel)
{
    if (data_02111738)
        return;
    data_02111738 = true;
    InitializeROMFilesystem_Internal(dmaChannel);
}

extern "C" void NitroVM_Initialize(NitroVM* vm)
{
    vm->links.pPrev = NULL;
    vm->links.pNext = NULL;
    vm->unknown_sublist_18.pLast = NULL;
    vm->unknown_sublist_18.pFirst = NULL;
    vm->linkedHandle = NULL;
    vm->maybeScheduledCommand = NITROVM_OPCODE_MAYBE_INVALID;
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
            if (!GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_IS_POPULATED))
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

    return NitroVM_020cbf58(vm, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_BY_NAME);
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