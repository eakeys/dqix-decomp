#include "Filesystem/FSInnerDefs.h"
#include "System/Interrupts.h"
#include "System/BiosData.h"
#include <globaldefs.h>

#pragma optimize_for_size off

#if defined(jpn)
#define func_020d0024 func_020d1af0
#define func_020d0008 func_020d1ad4

#define func_020c7080 func_020c8b4c

#define data_020f228c data_020f23f8
#endif

extern "C"
{
    void func_020d0024(unsigned short);
    void func_020d0008(unsigned short);

    int func_020c7080();
}

extern char data_020f228c[]; // "rom"

extern "C" CBool CreateFileAccessor(NitroFileAccessor* outAccessor, const char* path)
{
    NitroVM machine;
    NitroVM_Initialize(&machine);

    int result = GlobalSearchFileOrDirectory_020cc780(&machine, path, outAccessor, NULL);
    return (result != 0);
}

extern "C" CBool NitroVM_PrepareRead(NitroVM* vm, NitroHandle* handle,
    unsigned int start, unsigned int end, unsigned int capacity)
{
    vm->linkedHandle = handle;
    vm->regext_abc.c.u32 = capacity;
    vm->regext_abc.a.u32 = start;
    vm->regext_abc.b.u32 = end;

    // Operand 7 copies capacity into base_a,
    // start into base_b and base_d, end into base_c.
    if (!NitroVM_020cbf58(vm, NITROVM_OPCODE_COPY_REGISTERS))
        return false;

    // Not sure about bit 4, but bit 5 is cleared because this is a file
    // (so command 5 can run properly)
    vm->flags = (vm->flags | (1 << NITROVM_FLAG_4)) & ~(1 << NITROVM_FLAG_SEARCHING_DIRECTORY);
    return true;
}

// Accessor passed by value (in registers r1, r2)
extern "C" bool NitroVM_PrepareReadFileByID(NitroVM* vm, NitroFileAccessor volatile accessor)
{
    NitroHandle* handle = accessor.handle;
    if (handle == NULL)
        return false;

    vm->linkedHandle = handle;
    vm->regext_abc.a.ptr = handle;
    vm->regext_abc.b.u32 = accessor.fileID;
    // After this call, base_A will also hold the file ID
    if (!NitroVM_020cbf58(vm, NITROVM_OPCODE_GET_FAT_ENTRY))
        return false;

    // Not sure about bit 4, but bit 5 is cleared because this is a file
    // (so command 5 can run properly)
    vm->flags = (vm->flags | (1 << NITROVM_FLAG_4)) & ~(1 << NITROVM_FLAG_SEARCHING_DIRECTORY);
    return true;
}

extern "C" bool NitroVM_PrepareReadFileByPath(NitroVM* vm, const char* path)
{
    NitroFileAccessor accessor;
    if (CreateFileAccessor(&accessor, path))
    {
        if (NitroVM_PrepareReadFileByID(vm, accessor))
            return true;
    }

    return false;
}

extern "C" CBool NitroVM_MaybeCompleteTasks_020cca80(NitroVM* vm)
{
    if (!NitroVM_020cbf58(vm, 8))
        return false;

    vm->linkedHandle = NULL;
    vm->maybeScheduledCommand = NITROVM_OPCODE_MAYBE_INVALID;
    vm->flags &= ~((1 << NITROVM_FLAG_4) | (1 << NITROVM_FLAG_SEARCHING_DIRECTORY));
    return true;
}

extern "C" void NitroVM_WriteOutFilePath(NitroVM* vm, char* buffer, unsigned int bufferLength)
{
    if (vm->maybeScheduledCommand != NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_PATH)
    {
        vm->regext_abc.c.u16.low = 0;
        vm->regext_abc.c.u16.high = 0;
    }

    vm->regext_abc.a.ptr = buffer;
    vm->regext_abc.b.u32 = bufferLength;

    // This might be a bool function, in which case we return the return value
    // of this. (We still get tail call optimisation though)
    NitroVM_020cbf58(vm, NITROVM_OPCODE_GET_FILE_OR_DIRECTORY_PATH);
}

extern "C" CBool NitroVM_020ccae8(NitroVM* vm)
{
    CBool needToExecute = false;

    int oldState = DisableIRQInterrupts();

    if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_0))
    {
        // Execute iff flag 6 and flag 2 are both clear.
        needToExecute = !(vm->flags & ((1 << NITROVM_FLAG_6) | (1 << NITROVM_FLAG_2)));

        if (needToExecute)
        {
            vm->flags |= (1 << NITROVM_FLAG_2);
            do {
                func_020c7898(&vm->unknown_sublist_18);
            } while (!(vm->flags & (1 << NITROVM_FLAG_6))); // why the different format here?
        }
        else
        {
            do {
                func_020c7898(&vm->unknown_sublist_18);
            } while (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_0));
        }
    }

    SetIRQInterruptState(oldState);

    if (needToExecute)
        return NitroVM_ExecuteAndUnlink_020cbf14(vm);
    else
        return vm->storedResult == NITRO_RESULT_SUCCESS;
}

extern "C" void NitroVM_FlagStuff_020ccba8(NitroVM* vm)
{
    int oldState = DisableIRQInterrupts();

    if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_0))
    {
        vm->flags |= (1 << NITROVM_FLAG_1);
        vm->linkedHandle->flags |= (1 << NITROHANDLE_FLAG_5);
    }
    SetIRQInterruptState(oldState);
}

extern "C" int NitroVM_MaybeExecuteLoad_v1(NitroVM* vm, void* dst, int capacity)
{
    return NitroVM_ExecuteLoad_020cc8c4(vm, dst, capacity, true);
}

extern "C" int NitroVM_MaybeExecuteLoad_v0(NitroVM* vm, void* dst, int capacity)
{
    return NitroVM_ExecuteLoad_020cc8c4(vm, dst, capacity, false);
}

extern "C" CBool NitroVM_Seek(NitroVM* vm, int offset, int whence)
{
    switch (whence)
    {
    case 0: // SEEK_SET
        offset += vm->regbase_abc.b.s32;
        break;
    case 1: // SEEK_CUR
        offset += vm->regbase_d.s32;
        break;
    case 2: // SEEK_END
        offset += vm->regbase_abc.c.s32;
        break;
    default:
        return false;
    }
        
    if (offset < vm->regbase_abc.b.s32)
        offset = vm->regbase_abc.b.s32;
    if (offset > vm->regbase_abc.c.s32)
        offset = vm->regbase_abc.c.s32;

    vm->regbase_d.s32 = offset;
    return true;
}

extern "C" CBool SetPrimaryFilesystemRoot(const char* path)
{
    NitroVM vm;
    NitroVM_Initialize(&vm);
    NitroDirectoryAccessor accessor;
    if (!GlobalSearchFileOrDirectory_020cc780(&vm, path, NULL, &accessor))
        return false;

    data_02111728.primaryFSRoot = accessor;
    return true;
}

extern "C" void NitroHandle_020cccd4(NitroHandle* vm)
{
    int result = func_020d1198() ? NITRO_RESULT_5 : NITRO_RESULT_SUCCESS;
    NitroHandle_020cc6ac(vm, result);
}

extern "C" int ROMFilesystemLoadProc(NitroHandle* handle, void* dst, unsigned offset, unsigned len)
{
    LoadDataFromCartridgeToMemory(data_0211173c.maybeDMAChannel, offset, dst, len, &NitroHandle_020cccd4, handle, true);
    return NITRO_RESULT_REQUIRE_SYNC_MAYBE;
}

extern "C" int ROMFilesystemSaveProc(NitroHandle*, const void*, unsigned, unsigned)
{
    return NITRO_RESULT_FAILURE;
}

extern "C" int ROMFilesystemOpcodeOverride(NitroVM* vm, int opcode)
{
    if (opcode != NITROVM_OPCODE_SAVE)
    {
        switch (opcode)
        {
        case NITROVM_OPCODE_9:
            func_020d0008(data_0211173c.unknown_0);
            return NITRO_RESULT_SUCCESS;
        case NITROVM_OPCODE_10:
            func_020d0024(data_0211173c.unknown_0);
            return NITRO_RESULT_SUCCESS;
        }
    }
    else // opcode = 1 (save), not possible for ROM data
        return NITRO_RESULT_4;

    // Override does not implement command
    return NITRO_RESULT_OPCODE_NOT_IMPLEMENTED;
}

extern "C" int UnsupportedFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned)
{
    return NITRO_RESULT_FAILURE;
}

extern "C" int UnsupportedFilesystemOpcodeOverride(NitroVM*, int)
{
    return NITRO_RESULT_4;
}

extern "C" void InitializeROMFilesystem_Internal(unsigned int channel)
{
    data_0211173c.maybeDMAChannel = channel;
    data_0211173c.unknown_0 = func_020c7080();
    data_0211173c.arm9Data.start = NULL;
    data_0211173c.arm9Data.size = 0;
    data_0211173c.arm7Data.start = NULL;
    data_0211173c.arm7Data.size = 0;
    InitRawReadStructs_020d0ec4();

    NitroHandle_ZeroInit(&data_02111754);
    NitroHandle_AddToList(&data_02111754, data_020f228c, 3);

    const BootIndicator* biosData = (BootIndicator*)BIOS_ADDR_BOOT_INDICATOR;
    const CartridgeHeader& cartridgeHeader = biosData->cartHeader;
    if (biosData->bootMode == 2) // DS Download Play? Is this even a thing for DQ9?
    {
        data_0211173c.arm9Data.start = (void*)-1;
        data_0211173c.arm9Data.size = 0;
        data_0211173c.arm7Data.start = (void*)-1;
        data_0211173c.arm7Data.size = 0;
        NitroHandle_SetOpcodeOverride(&data_02111754, &UnsupportedFilesystemOpcodeOverride, -1);
        NitroHandle_Populate(&data_02111754, NULL, 0, 0, 0, 0,
            &UnsupportedFilesystemLoadProc, &ROMFilesystemSaveProc);
        return;
    }

    NitroHandle_SetOpcodeOverride(&data_02111754, &ROMFilesystemOpcodeOverride,
        (1 << NITROVM_OPCODE_10) | (1 << NITROVM_OPCODE_9) | (1 << NITROVM_OPCODE_SAVE));

    if (cartridgeHeader.fileNameTableOffset == -1 ||
        cartridgeHeader.fileNameTableOffset == 0 ||
        cartridgeHeader.fileAllocTableOffset == -1 ||
        cartridgeHeader.fileAllocTableOffset == 0)
        return;
    
    NitroHandle_Populate(&data_02111754, NULL, 
        cartridgeHeader.fileAllocTableOffset,
        cartridgeHeader.fileAllocTableSize,
        cartridgeHeader.fileNameTableOffset,
        cartridgeHeader.fileNameTableSize,
        &ROMFilesystemLoadProc,
        &ROMFilesystemSaveProc);
}

extern "C" unsigned int MaybeChangeDMAChannel_020ccf0c(unsigned int newChannel)
{
    int oldState = DisableIRQInterrupts();
    unsigned int oldChannel = data_0211173c.maybeDMAChannel;
    CBool destructionResult = NitroHandle_UnknownDestructionFunction(&data_02111754);
    data_0211173c.maybeDMAChannel = newChannel;
    if (destructionResult)
        NitroHandle_OtherUnknownDestFunc(&data_02111754);
    SetIRQInterruptState(oldState);
    return oldChannel;
}

extern "C" unsigned int LoadPrimaryFileTables(void* where, unsigned int capacity)
{
    return NitroHandle_LoadFileTables(&data_02111754, where, capacity);
}