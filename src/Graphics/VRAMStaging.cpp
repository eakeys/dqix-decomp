#include "Graphics/VRAMStaging.h"
#include <globaldefs.h>

#if defined(jpn)
#define data_0214e5e4 data_0214fdac
#endif

extern VRAMStagingManager data_0214e5e4;

void LockStagedTextureVRAMCopying()
{
    data_0214e5e4.textureLockMask_ <<= 1;
    data_0214e5e4.textureLockMask_ |= 1;
}

void UnlockStagedTextureVRAMCopying()
{
    data_0214e5e4.textureLockMask_ >>= 1;
}

void* AllocateVRAMStagingMemory(unsigned int length)
{
    return data_0214e5e4.AllocateInStagingSpace(length);
}

void FreeVRAMStagingMemory(const void* allocation)
{
    data_0214e5e4.FreeStagingSpaceAllocation(allocation);
}

void StageMemoryToVRAM(VRAMSubregion subregion, const void* data, unsigned int offset,
    unsigned int length, bool highPriority, bool allocateStagingSpace)
{
    data_0214e5e4.Stage(subregion, data, offset, length, highPriority, allocateStagingSpace);
}

int StageTexFilePaletteData(volatile NSBXXTex* texture, bool highPriority)
{
    if (texture == NULL)
        return -1;
        
    texture->maybeBlock4Flags_32_ |= (1 << 0);

    int size = texture->block4NumEightBytes_;
    size <<= 3;
    int offset = (unsigned short)texture->block4VRAMLoadOffset_ << 3;
    const void* src = (const void*)((intptr_t)texture + texture->block4Offset_);

    return data_0214e5e4.Stage(VRAMSubregion_TexturePalette,
        src, offset, size, highPriority, false);
}

int StageTexFileImageData(volatile NSBXXTex* texture, bool highPriority)
{
    if (texture == NULL)
        return -1;
        
    texture->maybeBlock1Flags_10_ |= (1 << 0);

    int size = texture->block1NumEightBytes_;
    size <<= 3;
    int offset = (unsigned short)texture->block1VRAMLoadOffset_ << 3;
    const void* src = (const void*)((intptr_t)texture + texture->block1Offset_);

    return data_0214e5e4.Stage(VRAMSubregion_TextureImage,
        src, offset, size, highPriority, false);
}

bool CancelVRAMStagingOperation(int id)
{
    return data_0214e5e4.CancelTaskByID(id);
}

void UpdateVRAMStagingVRAMBanks()
{
    data_0214e5e4.UpdateBanks();
}