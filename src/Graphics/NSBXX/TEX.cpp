#include "Graphics/NSBXX/NSBXX.h"
#include "System/LoadToVRAM.h"
#include <globaldefs.h>

int NSBXX_Tex_GetBlock1Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block1NumEightBytes_ << 3 : 0;
}

int NSBXX_Tex_GetBlock2Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block2NumEightBytes_ << 3 : 0;
}

void NSBXX_Tex_WriteImageVRAMOffsets(NSBXXTex* tex, int block1, int block2_3)
{
    if (block1 != 0)
        tex->block1VRAMLoadOffset_ = block1;
    if (block2_3 != 0)
        tex->block2Or3VRAMLoadOffset_ = block2_3;
}

void NSBXX_Tex_LoadImageToVRAM(NSBXXTex* tex, bool needsMapping)
{
    if (needsMapping)
        MemoryMapTextureImage();
    unsigned int block1Length = tex->block1NumEightBytes_ << 3;
    if (block1Length != 0)
    {
        LoadToTextureImage((const void*)((intptr_t)tex + tex->block1Offset_),
            (uint16_t)tex->block1VRAMLoadOffset_ << 3, block1Length);
        tex->maybeBlock1Flags_10_ |= (1 << 0);
    }
    int block3SourceOffset;
    unsigned int block2Length = tex->block2NumEightBytes_ << 3;
    if (block2Length != 0)
    {
        block3SourceOffset = tex->block3Offset_;
        unsigned int block2WriteOffset = (tex->block2Or3VRAMLoadOffset_ << 16) >> 13;
        int block2SourceOffset = tex->block2Offset_;
        
        LoadToTextureImage((const void*)((intptr_t)tex + block2SourceOffset),
            block2WriteOffset, block2Length);
        // I think the idea here is as follows: texture image data consists
        // of some of VRAM units A,B,C,D, which are each 0x20000 bytes.
        // If offset & 0x40000 == 0 then block 2 is put into the first
        // available unit and block 3 in the second, while if offset & 0x40000 != 0
        // then block 2 is put into the third unit and block 3 in the fourth.
        // I'm assuming the 0x20000 bit won't be set, as then things will be weird
        // (though I guess you could have block 2 in second unit and block 3
        // in latter half of the third).
        LoadToTextureImage((const void*)((intptr_t)tex + block3SourceOffset),
            (block2WriteOffset & 0x1ffff) / 2 + 0x20000 + ((block2WriteOffset & 0x40000) >> 2),
            block2Length >> 1);
        tex->maybeBlock23Flags_20_ |= (1 << 0);
    }
    if (needsMapping)
        MemoryUnmapTextureImage();
}

int NSBXX_Tex_GetBlock4Length(NSBXXTex* tex)
{
    return (tex != NULL) ? tex->block4NumEightBytes_ << 3 : 0;
}

void NSBXX_Tex_WritePaletteVRAMOffset(NSBXXTex* tex, int offset)
{
    tex->block4VRAMLoadOffset_ = offset;
}

void NSBXX_Tex_LoadPaletteToVRAM(NSBXXTex* tex, bool needsMapping)
{
    if (needsMapping)
        MemoryMapTexturePalette();

    LoadToTexturePalette((const void*)((intptr_t)tex + tex->block4Offset_),
        (uint16_t)tex->block4VRAMLoadOffset_ << 3,
        tex->block4NumEightBytes_ << 3);
    tex->maybeBlock4Flags_32_ |= (1 << 0);

    if (needsMapping)
        MemoryUnmapTexturePalette();
}