#include "Filesystem/FSInnerDefs.h"
#include "System/Interrupts.h"
#include "System/BiosData.h"
#include <globaldefs.h>

#pragma optimize_for_size off

extern "C"
{
    void func_020d0dcc(unsigned int, unsigned int offset, void* dst, unsigned int len,
        void (*proc)(NitroHandle*), NitroHandle*, CBool);

    void func_020d0024(unsigned short);
    void func_020d0008(unsigned short);

    void func_020d0ec4();

    int func_020c7080();
}

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
    if (!NitroVM_020cbf58(vm, 7))
        return false;

    // Not sure about bit 4, but bit 5 is cleared because this is a file
    // (so command 5 can run properly)
    vm->flags = (vm->flags | (1 << 4)) & ~(1 << 5);
    return true;
}

// Accessor passed by value (in registers r1, r2)
extern "C" CBool NitroVM_PrepareReadFileByID(NitroVM* vm, NitroFileAccessor volatile accessor)
{
    NitroHandle* handle = accessor.handle;
    if (handle == NULL)
        return false;

    vm->linkedHandle = handle;
    vm->regext_abc.a.ptr = handle;
    vm->regext_abc.b.u32 = accessor.fileID;
    // After this call, base_A will also hold the file ID
    if (!NitroVM_020cbf58(vm, 6))
        return false;

    // Not sure about bit 4, but bit 5 is cleared because this is a file
    // (so command 5 can run properly)
    vm->flags = (vm->flags | (1 << 4)) & ~(1 << 5);
    return true;
}

extern "C" CBool NitroVM_PrepareReadFileByPath(NitroVM* vm, const char* path)
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
    vm->maybeScheduledCommand = 14;
    vm->flags &= ~((1 << 4) | (1 << 5));
    return true;
}

extern "C" void NitroVM_WriteOutFilePath(NitroVM* vm, char* buffer, unsigned int bufferLength)
{
    if (vm->maybeScheduledCommand != 5)
    {
        vm->regext_abc.c.u16.low = 0;
        vm->regext_abc.c.u16.high = 0;
    }

    vm->regext_abc.a.ptr = buffer;
    vm->regext_abc.b.u32 = bufferLength;

    // This might be a bool function, in which case we return the return value
    // of this. (We still get tail call optimisation though)
    NitroVM_020cbf58(vm, 5);
}

extern "C" CBool NitroVM_020ccae8(NitroVM* vm)
{
    CBool needToExecute = false;

    int oldState = DisableIRQInterrupts();

    if (GET_FLAG_BIT(vm->flags, 0))
    {
        needToExecute = !(vm->flags & ((1 << 6) | (1 << 2)));

        if (needToExecute)
        {
            vm->flags |= (1 << 2);
            do {
                func_020c7898(&vm->unknown_sublist_18);
            } while (!(vm->flags & (1 << 6))); // why the different format here?
        }
        else
        {
            do {
                func_020c7898(&vm->unknown_sublist_18);
            } while (GET_FLAG_BIT(vm->flags, 0));
        }
    }

    SetIRQInterruptState(oldState);

    if (needToExecute)
        return NitroVM_ExecuteAndUnlink_020cbf14(vm);
    else
        return vm->storedResult == 0;
}

extern "C" void NitroVM_FlagStuff_020ccba8(NitroVM* vm)
{
    int oldState = DisableIRQInterrupts();

    if (GET_FLAG_BIT(vm->flags, 0))
    {
        vm->flags |= (1 << 1);
        vm->linkedHandle->flags |= (1 << 5);
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
    int opcode = func_020d1198() ? 5 : 0;
    NitroHandle_020cc6ac(vm, opcode);
}

extern "C" int ROMFilesystemLoadProc(NitroHandle* handle, void* dst, unsigned offset, unsigned len)
{
    func_020d0dcc(data_0211173c.maybeDMAChannel, offset, dst, len, &NitroHandle_020cccd4, handle, 1);
    return 6;
}

extern "C" int ROMFilesystemSaveProc(NitroHandle*, const void*, unsigned, unsigned)
{
    return 1;
}

extern "C" int ROMFilesystemOpcodeOverride(NitroVM* vm, int opcode)
{
    if (opcode != 1)
    {
        switch (opcode)
        {
        case 9:
            func_020d0008(data_0211173c.unknown_0);
            return 0;
        case 10:
            func_020d0024(data_0211173c.unknown_0);
            return 0;
        }
    }
    else // opcode = 1 (save), not possible for ROM data
        return 4;

    // Override does not implement command
    return 8;
}

extern "C" int UnsupportedFilesystemLoadProc(NitroHandle*, void*, unsigned, unsigned)
{
    return 1;
}

extern "C" int UnsupportedFilesystemOpcodeOverride(NitroVM*, int)
{
    return 4;
}

extern char data_020f228c[]; // "rom"

//#define biosData ((BootIndicator*)BIOS_ADDR_BOOT_INDICATOR)

extern "C" void InitializeROMFilesystem_Internal(unsigned int channel)
{
    data_0211173c.maybeDMAChannel = channel;
    data_0211173c.unknown_0 = func_020c7080();
    data_0211173c.unknown_8.unknown[0] = 0;
    data_0211173c.unknown_8.unknown[1] = 0;
    data_0211173c.unknown_10.unknown[0] = 0;
    data_0211173c.unknown_10.unknown[1] = 0;
    func_020d0ec4();

    NitroHandle_ZeroInit(&data_02111754);
    NitroHandle_AddToList(&data_02111754, data_020f228c, 3);

    const BootIndicator* biosData = (BootIndicator*)BIOS_ADDR_BOOT_INDICATOR;
    const CartridgeHeader& cartridgeHeader = biosData->cartHeader;
    if (biosData->bootMode == 2) // DS Download Play? Is this even a thing for DQ9?
    {
        data_0211173c.unknown_8.unknown[0] = -1;
        data_0211173c.unknown_8.unknown[1] = 0;
        data_0211173c.unknown_10.unknown[0] = -1;
        data_0211173c.unknown_10.unknown[1] = 0;
        NitroHandle_SetOpcodeOverride(&data_02111754, &UnsupportedFilesystemOpcodeOverride, -1);
        NitroHandle_Populate(&data_02111754, NULL, 0, 0, 0, 0,
            &UnsupportedFilesystemLoadProc, &ROMFilesystemSaveProc);
        return;
    }

    NitroHandle_SetOpcodeOverride(&data_02111754, &ROMFilesystemOpcodeOverride,
        (1 << 10) | (1 << 9) | (1 << 1));

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