#pragma ipa file
#include "Graphics/OBJManager.h"
#include "NitroSDK/LoadToVRAM.h"
#include "NitroSDK/DMA.h"
#include "NitroSDK/Cache.h"

extern "C"
{
    void func_0203c304(void*);

    // TextManager::GetInstance()
    void* func_020421a0();
}

inline bool ALWAYS_FALSE()
{
    int x = 0;
    return x;
}

extern OBJManager::Substruct_10* data_02105244;
extern MapGraphics data_02105254;
extern OAMEntryEXT data_021052b4[128]; // main engine
extern OAMEntryEXT data_021056b4[128]; // sub engine
extern OBJManager data_02105ab4;
extern unsigned short data_02105fd8[0x20 * 0x18]; // tile indices / screen data for BG1
extern unsigned short data_021065d8[0x40 * 0x20]; // tile indices / screen data for BG0

extern const float data_020e78c4[7][3] __attribute__((aligned(4)));
extern const unsigned short data_020e78a0[7];
extern const float data_020e78b0[5];

MapGraphics* MapGraphics::GetInstance()
{
    return &data_02105254;
}

void MapGraphics::Reset()
{
    if (ALWAYS_FALSE())
        return;
    memset(unknown_0_, 0, sizeof(unknown_0_));
    memset(unknown_20_, 0, sizeof(unknown_20_));
    memset(status_40_, 0, sizeof(status_40_));
}

void MapGraphics::Reset2()
{
    Reset();
}

void MapGraphics::Present()
{
    int engine = 0;
    int bg = 0;
    for (engine = 0; engine < 2; engine++)
    {
        for (bg = 0; bg < 4; bg++)
        {
            int idx = engine * 4 + bg;
            if (status_40_[idx] == 0)
                continue;
            
            status_40_[idx] = 0;
            if (engine == 1)
            {
                switch (bg)
                {
                case 0:
                    LoadToSubBG0ScreenData(data_021065d8, 0, 0x1000);
                    break;
                case 1:
                    LoadToSubBG1ScreenData(data_02105fd8, 0, 0x600);
                    break;
                }
            }
        }
    }
}

OBJManager* OBJManager::GetInstance()
{
    return &data_02105ab4;
}

void OBJManager::Reset()
{
    unknown_8_ = 0;
    oamEnabled_c_[0] = true;
    oamEnabled_c_[1] = true;
    unknown_0_ = 1 << 12;
    unknown_4_ = 0xe0;
    func_0203c304(&substruct_10_);
    substruct_10_.oamBuffer_0_ = data_021056b4;
    data_02105244 = &substruct_10_;
}

void OBJManager::FlushCache()
{
    CleanInvalidateCacheRange(&data_021052b4, sizeof(data_021052b4));
    CleanInvalidateCacheRange(&data_021056b4, sizeof(data_021056b4));
}

void OBJManager::Present()
{
    if (oamEnabled_c_[0])
    {
        LoadToMainOAM(data_021052b4, 0, sizeof(data_021052b4));
        DMAMemsetSynchronous(3, (unsigned int)&data_021052b4, 0x000000c0, sizeof(data_021052b4));
    }
    if (oamEnabled_c_[1])
    {
        LoadToSubOAM(data_021056b4, 0, sizeof(data_021056b4));
        DMAMemsetSynchronous(3, (unsigned int)&data_021056b4, 0x000000c0, sizeof(data_021056b4));
    }
    
    if (oamEnabled_c_[0] || oamEnabled_c_[1])
        substruct_10_.oamWriteIndex_510_ = 4;
}

OAMEntryEXT* OBJManager::GetMainOAMEntries() const
{
    return data_021052b4;
}

OAMEntryEXT* OBJManager::GetSubOAMEntries() const
{
    return data_021056b4;
}

void OBJManager::SetOAMEngineEnabled(int engine, bool to)
{
    oamEnabled_c_[engine] = to;
}

void OBJManager::TextEntry::Reset()
{
    memset(unknown_0_, 0, sizeof(unknown_0_));
    unknown_20_ = 0;
    unknown_21_ = 0;
    oamTileNumber_22_ = -1;
    unknown_24_ = 0;
}

void OBJManager::TextEntry::WriteToOAM(OAMEntryEXT *oam, int priority, int x, int y, int palette)
{
    short tileNumber = oamTileNumber_22_;
    if (tileNumber < 0)
        return;

    for (int i = 0; i < 4; i++)
    {
        oam->words01 = 0x80004000 | (y & 0xff) | ((x & 0x1ff) << 16);
        oam->base.word2 = tileNumber | ((priority & 3) << 10) | ((palette & 0xf) << 12);
        oam++;
        x += 32;
        tileNumber += 2;
    }
}

void OBJManager::Substruct_10::Reset()
{
    for (int i = 0; i < 32; i++)
        textEntries_4_[i].Reset();
    oamBuffer_0_ = NULL;
    unknown_512_ = 0;
    unknown_504_ = 1;
    unknown_50c_ = 0;
    unknown_508_ = 0;
    oamWriteIndex_510_ = 4;
}

inline int R8G8B8ToBGR15(int r, int g, int b)
{
    int shiftedR = r >> 3;
    int shiftedG = g >> 3;
    int shiftedB = b >> 3;
    return shiftedR | (shiftedG << 5) | (shiftedB << 10);
}

void OBJManager::Substruct_10::LoadPalettes()
{
    unsigned short palette[16] = {};
    char* scratchSpace = *(char**)((char*)func_020421a0() + 0x60);
    unsigned int scratchSize = 0xe0;
    memset(scratchSpace, 0, scratchSize);
    
    for (unsigned int i = 0; i < 7; i++)
    {
        palette[1] = 0x294a;
        if (i == 6)
            palette[1] = 0x18c6;

        const float* rgbBase = data_020e78c4[i];

        int j = 0;
        const float* pBrightness = data_020e78b0; // cursed that we need this...
        for (; j < 5; j++)
        {
            float factor;
            factor = data_020e78b0[j];
            int r8 = factor * rgbBase[0];
            factor = data_020e78b0[j];
            int g8 = factor * rgbBase[1];
            factor = data_020e78b0[j];
            int b8 = factor * rgbBase[2];
            
            palette[j + 2] = R8G8B8ToBGR15(r8, g8, b8);
        }
        
        palette[15] = data_020e78a0[i];
        memcpy(scratchSpace + i * 32, palette, 0x20);
    }
    CleanInvalidateCacheRange(scratchSpace, scratchSize);
    if (unknown_504_ == 0)
        LoadToMainObjStandardPalette(scratchSpace, unknown_50c_, scratchSize);
    else if (unknown_504_ == 1)
        LoadToSubObjStandardPalette(scratchSpace, unknown_50c_, scratchSize);
}

void OBJManager::Substruct_10::Destroy()
{
    Reset();
}

void OBJManager::Substruct_10::DrawTextEntry(short idx, int priority, int x, int y, int basePalette)
{
    if (idx < 0 || oamBuffer_0_ == NULL)
        return;

    unsigned int palette = (unknown_50c_ >> 5) + basePalette;
    palette &= 0xf;

    textEntries_4_[idx].WriteToOAM(oamBuffer_0_ + oamWriteIndex_510_, priority, x, y, palette);
    oamWriteIndex_510_ += 4;
    oamWriteIndex_510_ &= 0x7f;
}