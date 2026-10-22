#pragma once

#include <std_library_functions.h>
#include "../Graphics/Vector.h"
#include "TextBox.h"
#include "GridMenu.h"

/*
    Function runtime info for substructs:
    0205c73c substruct_4
    0205c77c substruct_4
*/

// sizeof == 0x238, owned by TextManager at offset 0x90.
class DialogueOptionManager
{
public:
    char unk_0[4];
    struct Substruct_4 // 0205c73c
    {
        char unk_0[0x18];
    } substruct_4_;

    DualInputGridMenu gridMenu_1c_;

    TextBox textBox_b4_;
    int textWidth_d0_;
    int unknown_d4_;
    int verticalSpacing_d8_;
    int numTouchRegions_dc_;
    short touchRegionX_e0_[8];
    short touchRegionY_f0_[8];
    short touchRegionWidth_100_[8];
    short touchRegionHeight_110_[8];
    unsigned short textWords_120_[128];
    unsigned short optionOffsets_220_[8];
    unsigned short totalWords_230_;
    unsigned char numOptions_232_;
    unsigned char unknown_233_;
    unsigned char unknown_234_;
    char pad_235[3];

    // usa: func_0205c790
    void Reset();
    // usa: func_0205c840
    void Release();

    // usa: func_0205cb74
    void AddOption(const char* text);
    // usa: func_0205cc50
    void ConfigureTouchRegions(bool alignWidth, int widthAdjust);
    // usa: func_0205cd28
    void ShowBackground3D();

    // usa: func_0205cecc
    // returns -1 if no selection made
    int GetTouchInputSelection() const;
};