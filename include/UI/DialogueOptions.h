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
    int unknown_d8_;
    int unknown_dc_;
    char substructOrArray_e0_[16];
    char substructOrArray_f0_[16];
    short widths_100_[8];
    short unknownArray_110_[8];
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
};