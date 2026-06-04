#include "Filesystem/OverlayFSManagement.h"
#include "Filesystem/FSInnerDefs.h"
#include "System/Cache.h"
#include "System/Memory.h"
#include "System/BiosData.h"

#pragma optimize_for_size off

extern "C"
{
    void func_020c9be0();
    int func_02000970(unsigned int);
}

unsigned int GetOverlaySizeOnCartridge(const OverlayMetadata& overlay)
{
    bool isCompressed = overlay.overlayFlags & 1;
    if (!isCompressed)
        return overlay.uncompressedSize;
    return overlay.compressedSize;
}

void InvalidateCacheAndZeroOverlay(const OverlayMetadata &overlay)
{
    unsigned char* startAddr = (unsigned char*)overlay.loadAddress;
    unsigned int copySize = overlay.uncompressedSize;
    unsigned int totalSize = copySize + overlay.bssSectionSize;

    InvalidateInstructionCacheRange(startAddr, totalSize);
    InvalidateDataCacheRange(startAddr, totalSize);
    VectorizedMemset(startAddr + copySize, 0, totalSize - copySize);
}

void CreateFileAccessorForOverlay(NitroFileAccessor *outAccessor, const OverlayMetadata &overlay)
{
    NitroFileAccessor temp;
    temp.handle = &data_02111754;
    temp.fileID = overlay.nitroFileID;
    
    // this cast forces the temporary struct to be created on the stack
    // (writing *&temp, or *(&temp), isn't enough)
    *outAccessor = *(NitroFileAccessor*)&temp;
}

bool LoadOverlayMetadataFromNitro(OverlayMetadata *into, bool isArm7,
                unsigned int overlayIdx, NitroHandle *romHandle,
                unsigned int arm9OverlayTableStart, unsigned int arm9OverlayTableSize,
                unsigned int arm7OverlayTableStart, unsigned int arm7OverlayTableSize)
{
    unsigned int offset;
    unsigned int start;
    unsigned int size;
    
    if (!isArm7)
    {
        start = arm9OverlayTableStart;
        size = arm9OverlayTableSize;
    }
    else
    {
        start = arm7OverlayTableStart;
        size = arm7OverlayTableSize;
    }

    if (size <= (offset = overlayIdx << 5))
        return false;

    NitroVM machine;
    NitroVM_Initialize(&machine);
    // The compiler loves combining shifts into additions, so elides offset by
    // just using start + (overlayIdx << 5). But the target assembly performs
    // the shift early, so some inline assembly can forcefully cancel this optimization
    {
    NitroVM* arg_machine;
    NitroHandle* arg_handle;
    unsigned int arg_readStart;
    unsigned int arg_readEnd;
    unsigned int arg_capacity;

    __asm("mvn arg_capacity, 0");
    arg_machine = &machine;
    __asm("mov arg_handle, romHandle");
    __asm("add arg_readStart, start, offset");
    arg_readEnd = start + size;
    __asm("b here\nhere:");

    if (!NitroVM_PrepareRead(arg_machine, arg_handle, arg_readStart, arg_readEnd, arg_capacity))
        return false;
    }

    if (NitroVM_MaybeExecuteLoad_v0(&machine, into, 0x20) != 0x20)
    {
        NitroVM_MaybeCompleteTasks_020cca80(&machine);
        return false;
    }

    NitroVM_MaybeCompleteTasks_020cca80(&machine);
    into->isArm7 = isArm7;

    NitroFileAccessor overlayAccessor;
    CreateFileAccessorForOverlay(&overlayAccessor, *into);
    if (!NitroVM_PrepareReadFileByID(&machine, overlayAccessor))
        return false;
    
    into->unknown_24 = machine.regbase_abc.b.u32;
    into->unknown_28 = machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;
    NitroVM_MaybeCompleteTasks_020cca80(&machine);
    return true;
}

bool LoadOverlayMetadata(OverlayMetadata* into, bool isArm7, unsigned int idx)
{
    Struct_0211173c::ArmData* armData;
    armData = (!isArm7) ? &data_0211173c.unknown_8 : &data_0211173c.unknown_10;

    unsigned int offset;
    const char* source = (const char*)armData->unknown[0];
    if (source)
    {
        if (armData->unknown[1] <= (offset = idx * 0x20))
            return false;
        const char* shifted;
        OverlayMetadata* copyDst;
        __asm("mov copyDst, into");
        __asm("add shifted, source, offset");
        __asm("b here\nhere:");
        VectorizedInvertedMemcpy(shifted, copyDst, 0x20);
        into->isArm7 = isArm7;

        NitroVM machine;
        NitroVM_Initialize(&machine);
        NitroFileAccessor accessor;
        CreateFileAccessorForOverlay(&accessor, *into);
        if (!NitroVM_PrepareReadFileByID(&machine, accessor))
            return false;

        into->unknown_24 = machine.regbase_abc.b.u32;
        into->unknown_28 = machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;
        NitroVM_MaybeCompleteTasks_020cca80(&machine);
        return true;
    }
    else
    {
        CartridgeHeader::OverlayTableValues* overlay = 
            (CartridgeHeader::OverlayTableValues*)BIOS_ADDR_OVERLAY_TABLE_VALUES;
        return LoadOverlayMetadataFromNitro(into, isArm7, idx, &data_02111754,
            overlay->arm9TableStart, overlay->arm9TableSize,
            overlay->arm7TableStart, overlay->arm7TableSize);
    }
}

bool LoadCompressedOverlay(const OverlayMetadata &overlay, NitroVM *machine)
{
    NitroVM_Initialize(machine);
    NitroFileAccessor accessor;
    CreateFileAccessorForOverlay(&accessor, overlay);
    if (!NitroVM_PrepareReadFileByID(machine, accessor))
        return false;
    
    unsigned int cartridgeSize = GetOverlaySizeOnCartridge(overlay);
    InvalidateCacheAndZeroOverlay(overlay);
    if (NitroVM_MaybeExecuteLoad_v1(machine, (void*)overlay.loadAddress, cartridgeSize) != cartridgeSize)
    {
        NitroVM_MaybeCompleteTasks_020cca80(machine);
        return false;
    }

    return true;
}

bool LoadCompressedOverlay(const OverlayMetadata& overlay)
{
    NitroVM machine;
    NitroVM_Initialize(&machine);
    NitroFileAccessor accessor;
    CreateFileAccessorForOverlay(&accessor, overlay);
    if (!NitroVM_PrepareReadFileByID(&machine, accessor))
        return false;
    
    unsigned int cartridgeSize = GetOverlaySizeOnCartridge(overlay);
    InvalidateCacheAndZeroOverlay(overlay);
    if (NitroVM_MaybeExecuteLoad_v0(&machine, (void*)overlay.loadAddress, cartridgeSize) != cartridgeSize)
    {
        NitroVM_MaybeCompleteTasks_020cca80(&machine);
        return false;
    }

    NitroVM_MaybeCompleteTasks_020cca80(&machine);
    return true;
}

extern struct UnknownDownloadPlayStruct
{
    unsigned int unknown[5];
} data_020f2e44[];

extern struct {
    const void* pointer;
    unsigned int length;
} data_020f2290;

extern "C" void func_020c0c3c(UnknownDownloadPlayStruct*, unsigned, unsigned, unsigned char*, unsigned);

bool VerifyDownloadPlayStruct(const UnknownDownloadPlayStruct& data,
    unsigned int loadAddress, unsigned int size)
{
    UnknownDownloadPlayStruct clone;
    VectorizedMemset(&clone, 0, sizeof(UnknownDownloadPlayStruct));

    unsigned char buffer[64];
    VectorizedInvertedMemcpy(data_020f2290.pointer, buffer, data_020f2290.length);
    func_020c0c3c(&clone, loadAddress, size, buffer, data_020f2290.length);

    unsigned int offsetCounter = 0;
    unsigned int* cloneReader = &clone.unknown[0];
    do {
        if (*cloneReader != *(unsigned int*)((int)&data + offsetCounter))
            break;
        offsetCounter += 4;
        cloneReader++;
    } while (offsetCounter < sizeof(UnknownDownloadPlayStruct));

    return offsetCounter == sizeof(UnknownDownloadPlayStruct);
}

void DecompressAndStaticInitializeOverlay(const OverlayMetadata& overlay)
{
    unsigned int initialSize = GetOverlaySizeOnCartridge(overlay);

    BootIndicator* bootData = (BootIndicator*)BIOS_ADDR_BOOT_INDICATOR;
    // extremely weird code that never runs (only for DS Download Play?)
    if (bootData->bootMode == 2)
    {
        bool result = false;
        if (overlay.overlayFlags & 2)
        {
            // Using two different formats for the same address (number and label)
            // forces the word to appear twice
            UnknownDownloadPlayStruct* start = (UnknownDownloadPlayStruct*)0x020f2e44;
            UnknownDownloadPlayStruct* end = data_020f2e44;
            if (overlay.overlayIndex < end - start)
            {
                result = VerifyDownloadPlayStruct(
                    *(UnknownDownloadPlayStruct*)((int)start + sizeof(UnknownDownloadPlayStruct) * overlay.overlayIndex),
                    overlay.loadAddress, initialSize);
            }
        }
        if (!result)
        {
            VectorizedMemset((void*)overlay.loadAddress, 0, initialSize);
            func_020c9be0();
            return;
        }
    }

    // Decompress the overlay in place
    if (overlay.overlayFlags & 1)
        func_02000970(overlay.loadAddress + initialSize);
    
    CleanInvalidateCacheRange((void*)overlay.loadAddress, overlay.uncompressedSize);
    OverlayMetadata::PFNStaticInitializer* pInitializer = overlay.staticInitStart;
    OverlayMetadata::PFNStaticInitializer* initializerEnd = overlay.staticInitEnd;
    if (pInitializer < initializerEnd)
    {
        do {
            if (*pInitializer)
                (*pInitializer)();
            pInitializer++;
        } while (pInitializer < initializerEnd);
    }
}