#include "Filesystem/FSInnerDefs.h"
#include "Filesystem/FileIOPorts.h"
#include "System/BiosData.h"
#include "System/Memory.h"
#include "System/Interrupts.h"
#include "System/Cache.h"
#include <globaldefs.h>

#pragma optimization_level 2
#pragma optimize_for_size off

#define ADDR_ITCM_START 0x01ff8000
#define ADDR_ITCM_END 0x02000000

#define ADDR_GAMECARD_BUS_ROMCTRL 0x040001a4
#define ADDR_GAMECARD_RECEIVED_DATA 0x04100010
#define GAMECARD_BUS_COMMAND_BYTE(n) *(unsigned char*)(0x040001a8 + n)

#define INTERNAL_ASM_NOP(what) __asm("b " what "\n" what ":")
#define INTERNAL_ASM_NOP_2(num) INTERNAL_ASM_NOP("label__" #num)
#define INTERNAL_ASM_NOP_3(num) INTERNAL_ASM_NOP_2(num)
// Puts in some inline assembly of the form
// ```     b right_here          ```
// ```     right_here:           ```
// with a uniquely generated label to prevent clashes.
// Such an instruction does nothing and is completely optimized out, but it
// prevents some quirks of inline assembly spilling over into later code
// (such as constants being loaded from the end of the function, even if they're
// small enough to be hardcoded into a command). It also disables some
// optimizations that move instructions to more efficient (but not matching)
// places - two C++ instructions on opposite sides of this boundary will 
// (usually?) have their order respected.
#define DECLARE_ASM_BOUNDARY() INTERNAL_ASM_NOP_3(__LINE__)


extern "C"
{
    // Seems to set up DMA to repeatedly read from the read parameter into write
    // pointer, incrementing the write pointer but not the read pointer.
    void func_020ca8e8(unsigned int dmaChannel, const void* readFrom, void* writeTo, unsigned int length);

    void func_020c7950(void*);

    // Gets the base of the tightly coupled memory region
    unsigned int func_020c89e4();

    // Reset / initialize DMA channel
    void func_020c9f2c(unsigned int);

    void func_020cfcbc(const void*);

    void func_020cfe08();

    void func_020cff50();

    // Checks some boolean is set, hangs if not set
    void func_020cff28();

    bool TryReadViaDMA(Struct_02111f20*);
    void func_020d1234(unsigned int);

    // Maybe sets interrupt table?
    void func_020c6aec(unsigned int mask, const void* fn);
    // Enables interrupt enable flag and return old value
    unsigned int func_020c6cbc(unsigned int);
    // Disables the specified interrupt enable flags
    void func_020c6cec(unsigned int);
    // Set interrupt enable flag and return old value
    unsigned int func_020c6d1c(unsigned int);
    
    void func_020d1118();
}

Struct_02111f20::PFNRead GetCartridgeReadProc();

extern "C" bool TransferScratchBuffer(Struct_02111f20* input)
{
    Struct_021118e0* ptr = &data_021118e0;
    unsigned int romCurrentBlockStart = ptr->cartridgeReadOffset & 0xfffffe00;
    if (romCurrentBlockStart == (unsigned int)input->alignedWrite)
    {
        unsigned int offsetFromAlignment = ptr->cartridgeReadOffset
            - romCurrentBlockStart;
        unsigned int usefulLength = 0x200 - offsetFromAlignment;
        if (usefulLength > ptr->writeLength)
            usefulLength = ptr->writeLength;
        
        VectorizedInvertedMemcpy(input->scratchBuffer + offsetFromAlignment,
            ptr->writeDst, usefulLength);
        ptr->cartridgeReadOffset += usefulLength;
        ptr->writeDst += usefulLength;
        ptr->writeLength -= usefulLength;
    }
    return ptr->writeLength != 0;
}

// Notably this is not a uint64 parameter. This way you can write
// (0x11223344, 0x55667788) and it will write the eight bytes
// (11, 22, 33, 44, 55, 66, 77, 88)
void SendGamecardBusCommand(unsigned int firstWord, unsigned int secondWord)
{
    volatile unsigned int* control = (unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL;
    while (*control & (1 << 31)) {}

    // Upper byte of "AUXSPICNT - Gamecard ROM and SPI Control (R/W)"
    // Set byte 15 (enable NDS slot), set byte 14 (enable transfer ready IRQ)
    // and clear byte 13 (0 meaning parallel NDS slot mode)
    // (Note bytes 12-8 are unused)
    *(unsigned char*)0x40001a1 = 0xC0;

    GAMECARD_BUS_COMMAND_BYTE(0) = firstWord >> 24;
    GAMECARD_BUS_COMMAND_BYTE(1) = firstWord >> 16;
    GAMECARD_BUS_COMMAND_BYTE(2) = firstWord >> 8;
    GAMECARD_BUS_COMMAND_BYTE(3) = firstWord >> 0;

    GAMECARD_BUS_COMMAND_BYTE(4) = secondWord >> 24;
    GAMECARD_BUS_COMMAND_BYTE(5) = secondWord >> 16;
    GAMECARD_BUS_COMMAND_BYTE(6) = secondWord >> 8;
    GAMECARD_BUS_COMMAND_BYTE(7) = secondWord >> 0;
}

void ReadSingleSegmentFromCartridge()
{
    Struct_021118e0* s18e0 = &data_021118e0;
    void* readLocation = (void*)ADDR_GAMECARD_RECEIVED_DATA;
    Struct_02111f20* ps1f20 = &data_02111f00.innerStruct;

    DECLARE_ASM_BOUNDARY();

    func_020ca8e8(s18e0->dmaChannel, readLocation, s18e0->writeDst, 0x200);
    unsigned int offset = s18e0->cartridgeReadOffset;
    SendGamecardBusCommand(0xb7000000 | (offset >> 8), offset << 24);
    *(unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL = ps1f20->control_4;
}

void DMAChainSegmentInterruptHandler()
{
    func_020c9f2c(data_021118e0.dmaChannel);

    Struct_021118e0* ptr = &data_021118e0;
    
    ptr->cartridgeReadOffset += 512;
    ptr->writeDst += 512;
    ptr->writeLength -= 512;

    bool moreToWrite = ptr->writeLength != 0;

    if (!moreToWrite)
    {
        unsigned int interruptFlags;
        __asm("mov interruptFlags, #1 << 19");
        func_020c6cec(interruptFlags);
        __asm("mov interruptFlags, #1 << 19");
        func_020c6d1c(interruptFlags);
        Struct_021118e0* readManager = &data_021118e0;
        unsigned int romChipID = SetupNormalGamecardBusCommandMode();
        func_020d1234(romChipID);

        unsigned int* writeLocation = readManager->unknown_0;
        unsigned int zero;
        __asm("mov zero, 0");
        *writeLocation = zero;
        DECLARE_ASM_BOUNDARY();

        PFNNitroCleanup cleanProc = readManager->maybeCleanupProc_38;
        NitroHandle* handle = readManager->handle_3c;

        int oldState = DisableIRQInterrupts();
        readManager->flags_114 &= ~0x4c;
        func_020c78e8(&readManager->list_10C);

        if (readManager->flags_114 & (1 << 4))
            func_020c7950(readManager->bufferOrOtherStruct_44);
        SetIRQInterruptState(oldState);

        if (cleanProc != NULL)
            cleanProc(handle);

        return;
    }
    ReadSingleSegmentFromCartridge();
}

extern "C" bool TryReadViaDMA(Struct_02111f20* handler)
{
    Struct_021118e0* readManager = &data_021118e0;
    unsigned int length = readManager->writeLength;
    
    bool canDoDMARead = false;
    bool readAlignedToCartridgeSegments = false;
    bool destinationIsSuitableMemoryType = false; // i.e. not DTCM or ITCM
    bool destinationIsCacheAligned = false; 
    
    unsigned int destination = (unsigned int)readManager->writeDst;
    unsigned int alignmentMod32 = (unsigned int)destination & 0x1f;
    
    if (alignmentMod32 == 0 && ((readManager->dmaChannel <= 3)))
        destinationIsCacheAligned = true;
    
    if (destinationIsCacheAligned)
    {
        unsigned int dtcmBase = func_020c89e4();
        bool isDTCM = true;
        bool isITCM = false;
        if (destination + length > ADDR_ITCM_START && destination < ADDR_ITCM_END)
            isITCM = true;
        
        // 0x4000 = 16kb is the size of data tightly coupled memory
        if (!isITCM && (dtcmBase >= destination + length || dtcmBase + 0x4000 <= destination))
            isDTCM = false;
        
        if (!isDTCM)
            destinationIsSuitableMemoryType = true;
    }

    if (destinationIsSuitableMemoryType && ((readManager->cartridgeReadOffset | length) & 0x1ff) == 0)
        readAlignedToCartridgeSegments = true;

    if (readAlignedToCartridgeSegments && length != 0)
        canDoDMARead = true;
    
    handler->control_4 = (data_020f22cc->gamecardBusControlNormalSettings & ~0x07000000) | 0xa1000000;
    
    if (canDoDMARead)
    {
        int oldState = DisableIRQInterrupts();
        if (length < readManager->maybeInstructionCacheLimit_118)
            InvalidateInstructionCacheRange((void*)destination, length);
        else
            InvalidateInstructionCache();

        if (length < readManager->maybeDataCacheLimit_11c)
        {
            if (alignmentMod32 != 0)
            {
                destination -= alignmentMod32;
                CleanCacheRange((void*)destination, 0x20);
                CleanCacheRange((void*)(destination + length), 0x20);
                length += 0x20;
            }
            InvalidateDataCacheRange((void*)destination, length);
            DrainWriteBuffer();
        }
        else
        {
            CleanInvalidateDataCache();
        }

        // Set handler for interrupt ID 19 (gamecard data transfer completion)
        func_020c6aec(1 << 19, &DMAChainSegmentInterruptHandler);
        func_020c6d1c(1 << 19);
        func_020c6cbc(1 << 19);
        SetIRQInterruptState(oldState);
        ReadSingleSegmentFromCartridge();
    }

    return canDoDMARead;
}

extern "C" void ReadBlocksFromCartridge(Struct_02111f20* handler)
{
    Struct_021118e0* readManager = &data_021118e0;
    while (true)
    {
        unsigned int alignedOffset = readManager->cartridgeReadOffset & 0xfffffe00;
        unsigned int* practicalWriteLoc;
        // If read offset is not 512-byte aligned, write dst is not 4-byte aligned
        // or we're loading < 512 bytes, use the scratch space in the input struct
        if (alignedOffset != readManager->cartridgeReadOffset ||
            ((unsigned int)readManager->writeDst & 3) ||
            readManager->writeLength < 0x200)
        {
            handler->alignedWrite = (unsigned int*)alignedOffset;
            practicalWriteLoc = (unsigned int*)handler->scratchBuffer;
        }
        else
            practicalWriteLoc = (unsigned int*)readManager->writeDst;
        SendGamecardBusCommand(0xb7000000 | (alignedOffset >> 8), alignedOffset << 24);
        *(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL = handler->control_4;

        unsigned int control;
        int wordIndex = 0;
        unsigned int bytesCopied = 0;
        do {
            control = *(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL;
            if (control & (1 << 23))
            {
                unsigned int data = *(volatile unsigned int*)ADDR_GAMECARD_RECEIVED_DATA;
                // The gamecard bus command only allows for reading 512 bytes,
                // so is this necessary? I guess if you try to read past the end
                // and get garbage data?
                if (bytesCopied < 0x200)
                {
                    practicalWriteLoc[wordIndex] = data;
                    bytesCopied += 4;
                    wordIndex++;
                }
            }
        } while (control & (1 << 31));

        // If we performed an aligned write, can just move on to the next
        // aligned write
        if (practicalWriteLoc == (unsigned int*)readManager->writeDst)
        {
            unsigned int remainingLength;
            volatile Struct_021118e0* ptr = &data_021118e0;
            ptr->cartridgeReadOffset += 0x200;
            ptr->writeDst += 0x200;
            remainingLength = ptr->writeLength - 0x200;
            ptr->writeLength = remainingLength;
            if (remainingLength != 0)
                continue;
            break;
        }
        else if (!TransferScratchBuffer(handler))
            break;
    }
}

unsigned int SetupNormalGamecardBusCommandMode()
{
    // Get ROM chip ID
    SendGamecardBusCommand(0xb8000000, 0x00000000);
    unsigned int newSettings = data_020f22cc->gamecardBusControlNormalSettings;
    newSettings &= ~0x07000000; // clear data block size to 0 = none
    newSettings |=  0xa7000000; // set block size to 7 = 4 bytes, block status = started/busy, release RESB?
    newSettings &=  0xffffe000; // disable bit 14 (unknown?) and 13 (don't KEY2 encrypt)
    *(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL = newSettings;
    do {
    } while ((*(volatile unsigned int*)ADDR_GAMECARD_BUS_ROMCTRL & (1 << 23)) == 0);
    return *(volatile unsigned int*)ADDR_GAMECARD_RECEIVED_DATA;
}

// I don't know that this is safe per se, but in practice it wraps
// ReadBlocksFromCartridge (the readProc) with some auxiliary stuff
extern "C" void SafeReadBlocksFromCartridge(Struct_021118e0*)
{
    Struct_02111f20* rawHandler = &data_02111f00.innerStruct;
    if (TransferScratchBuffer(rawHandler))
    {
        Struct_02111f20::PFNRead readProc = rawHandler->readProc;
        readProc(rawHandler);
    }

    Struct_021118e0* readManager = &data_021118e0;
    unsigned int romChipID = SetupNormalGamecardBusCommandMode();
    // Might be checking if the cart has been ejected or similar
    func_020d1234(romChipID);

    unsigned int* writeLocation = readManager->unknown_0;
    unsigned int zero;
    __asm("mov zero, 0");
    *writeLocation = zero;
    DECLARE_ASM_BOUNDARY();

    PFNNitroCleanup cleanProc = readManager->maybeCleanupProc_38;
    NitroHandle* handle = readManager->handle_3c;

    int oldState = DisableIRQInterrupts();
    readManager->flags_114 &= ~0x4c;
    func_020c78e8(&readManager->list_10C);

    if (readManager->flags_114 & (1 << 4))
        func_020c7950(readManager->bufferOrOtherStruct_44);
    SetIRQInterruptState(oldState);

    if (cleanProc != NULL)
        cleanProc(handle);
}

void LoadDataFromCartridgeToMemory(unsigned int dmaChannel,
    unsigned int cartridgeOffset, void* dest, unsigned int length,
    PFNNitroCleanup cleanupProc, NitroHandle* handle, CBool unknownBool)
{
    Struct_02111f20* rawHandler = &data_02111f00.innerStruct;
    Struct_021118e0* readManager = &data_021118e0;
    // Verify some bool
    func_020cff28();

    int oldState = DisableIRQInterrupts();

    while (readManager->flags_114 & (1 << 2))
    {
        func_020c7898(&readManager->list_10C);
    }
    readManager->flags_114 |= (1 << 2);
    readManager->maybeCleanupProc_38 = cleanupProc;
    readManager->handle_3c = handle;
    SetIRQInterruptState(oldState);
    readManager->cartridgeReadOffset = cartridgeOffset + data_02111f00.number;
    readManager->dmaChannel = dmaChannel;
    readManager->writeDst = (unsigned char*)dest;
    readManager->writeLength = length;

    if (dmaChannel <= 3)
        func_020c9f2c(dmaChannel);

    if (TryReadViaDMA(rawHandler))
    {
        if (!unknownBool)
            func_020d0f28();
    }
    else if (unknownBool)
    {
        func_020cfcbc(&SafeReadBlocksFromCartridge);
    }
    else
    {
        readManager->pointer_MaybeToPrevStruct_104 = data_02111304.unknown_4;
        SafeReadBlocksFromCartridge(readManager);
    }
}

extern "C" void InitRawReadStructs_020d0ec4()
{
    Struct_021118e0* readManager = &data_021118e0;
    if (readManager->flags_114 != 0)
        return;

    readManager->flags_114 = 1;
    readManager->writeLength = 0;
    readManager->writeDst = NULL;
    readManager->cartridgeReadOffset = 0;
    readManager->dmaChannel = -1;
    readManager->maybeCleanupProc_38 = NULL;
    readManager->handle_3c = NULL;

    data_02111f00.number = 0;
    func_020cfe08();
    data_02111f00.innerStruct.readProc = GetCartridgeReadProc();
    func_020d1118();
}

extern "C" void func_020d0f28()
{
    func_020cff50();
}

Struct_02111f20::PFNRead GetCartridgeReadProc()
{
    return &ReadBlocksFromCartridge;
}