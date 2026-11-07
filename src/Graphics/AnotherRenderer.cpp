#include "Graphics/AnotherRenderer.h"
#include "NitroSDK/Cache.h"
#include "NitroSDK/LoadToVRAM.h"
#include "Resource/GameResources.h"
#include "GameState/GameState.h"

extern "C"
{
    GameResources* func_0200fb8c(GameState*);

    void func_020e12a8(void*, int,int,int,int);
}

void AnotherRenderer::Initialize(int screen, int windowID)
{
    unknown_0_ = 0;
    unknown_4_ = NULL;
    unknown_8_ = 0;
    for (int i = 0; i < 3; i++)
        func_020e12a8(unknown_array_a_[i], 0, 0, 0, 0);
    colors_22_[0] = 0x7fff;
    colors_22_[1] = 0xf0a; // green
    clipWindow_28_.Initialize(screen, windowID);
}

void AnotherRenderer::LoadColorToPalette(int palettePosition, int index)
{
    if (index < 0 || index >= 2)
        return;
    
    unsigned int loadOffset = 2 * palettePosition;
    const unsigned short* pColor = &colors_22_[index];
    CleanInvalidateCacheRange(pColor, 2);
    if (unknown_4_->unknown_3e == 0)
        LoadToMainObjStandardPalette(pColor, loadOffset + 0x1c0, 2);
    else
        LoadToSubObjStandardPalette(pColor, loadOffset + 0x1c0, 2);
    CleanCacheRange(pColor, 2);
}

void AnotherRenderer::SetColor(int index, unsigned short value)
{
    if (index >= 0 && index < 2)
        colors_22_[index] = value;
}

void AnotherRenderer::ReloadDefaultColors()
{
    if (unknown_4_ == NULL)
        return;
    unsigned short* defaultData = func_0200fb8c(GameState::GetInstance())->paletteData_2c;
    if (defaultData == NULL)
        return;
    unsigned short* loadSource = defaultData + 6;
    CleanInvalidateCacheRange(loadSource, 0x20);
    if (unknown_4_->unknown_3e == 0)
        LoadToMainObjStandardPalette(loadSource, 0x1c0, 0x20);
    else
        LoadToSubObjStandardPalette(loadSource, 0x1c0, 0x20);
    CleanCacheRange(loadSource, 0x20);
    LoadColorToPalette(9, 0);
    LoadColorToPalette(11, 0);
}