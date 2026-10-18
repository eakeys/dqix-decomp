#pragma once

#include <std_library_functions.h>
#include "Vector.h"

// This needs to go in a different directory but it's related
// to text processing so it's here for now

// sizeof == 0x1c
struct TextBox
{
    short x;
    short y;
    short width;
    short height;
    int unknown_8;
    int unknown_c;
    fix32_t scaleX;
    fix32_t scaleY;
    unsigned short color;
    char pad_1a[2];

    void Initialize();
};

/*
    Function runtime info for substructs:
    0205ba2c substruct_1c::subsubstructBase
    0205ba68 substruct_1c::subsubstructBase
    0205bacc substruct_1c::subsubstructBase
    0205bb04 substruct_1c::subsubstructBase
    0205bb84 substruct_1c::subsubstructBase
    0205bbcc substruct_1c::subsubstruct_4
    0205bc10 substruct_1c::subsubstruct_4
    0205bc2c substruct_1c::subsubstruct_4
    0205bcdc substruct_1c::subsubstruct_4
    0205bd20 substruct_1c::subsubstruct_4
    0205bd58 substruct_1c::subsubstruct_4
    0205bd78 substruct_1c::subsubstruct_4
    0205bea0 substruct_1c::subsubstruct_4
    0205bef8 substruct_1c::subsubstruct_54
    0205bf3c substruct_1c::subsubstruct_54
    0205bf58 substruct_1c::subsubstruct_54
    0205c53c substruct_1c
    0205c564 substruct_1c
    0205c570 substruct_1c
    0205c5b0 substruct_1c
    0205c6bc substruct_1c
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

    struct Substruct_1c // 0205c564
    {
        struct SubsubstructBase // 0205ba2c
        {
            int unknown_0_;
            int unknown_4_;
            int unknown_8_;
            int unknown_c_;
            char unknown_10_;
            char pad_11[3];
            int unknown_14_;
            int unknown_18_;
            int unknown_1c_;
            int unknown_20_;
            int unknown_24_;
            int unknown_28_;
            int unknown_2c_;

            // usa: func_0205ba2c
            void Reset();
        };

        char unk_0[4];

        struct Subsubstruct_4 // 0205bbcc
        {
            SubsubstructBase base_0_;
            int unknown_30_;
            int unknown_34_;
            short unknown_38_;
            short unknown_3a_;
            int unknown_3c_;
            int unknown_40_;
            int unknown_44_;
            int unknown_48_;
            char unknown_4c_;
            char unknown_4d_;
            char unknown_4e_;
            char pad_4f[1];
        } subsubstruct_4_;
        struct Subsubstruct_54 // 0205bef8
        {
            SubsubstructBase base_0_;
            int unknown_30_;
            short unknown_34_;
            char unknown_36_;
            char unknown_37_;
            char unknown_38_;
            char unknown_39_;
            char unknown_3a_;
            char unk_3b[1];
            char unknown_3c_;
            char unknown_3d_;
            char pad_3e[2];
        } subsubstruct_54_;
        unsigned char unknown_94_;
        unsigned char unknown_95_;
        char pad_96[2];
    } substruct_1c_;

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