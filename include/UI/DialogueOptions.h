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
    // No idea what this is/does, it doesn't seem to be read anywhere
    struct Substruct_4
    {
        fix32_t unknown_0;
        fix32_t unknown_4;
        fix32_t unknown_8;
        fix32_t unknown_c;
        unsigned short unknown_10;
        unsigned char unknown_12;
        unsigned char unknown_13;
        unsigned char unknown_14;
        unsigned char unknown_15;
        bool unknown_16;
        char pad_17[1];

        void Reset();
        void SetFields(bool value16);

    } unknown_4_;

    DualInputGridMenu gridMenu_1c_;

    TextBox textBox_b4_;
    int textWidth_d0_;
    int horizontalSpacing_d4_;
    int verticalSpacing_d8_;
    int numTouchRegions_dc_;
    short touchRegionX_e0_[8];
    short touchRegionY_f0_[8];
    short touchRegionWidth_100_[8];
    short touchRegionHeight_110_[8];
#ifdef jpn
    unsigned short textWords_120_[72];
#else
    unsigned short textWords_120_[128];
#endif
    unsigned short optionOffsets_220_[8];
    unsigned short totalWords_230_;
    unsigned char numOptions_232_;
    unsigned char unknown_233_;
    unsigned char unknown_234_;
    char pad_235[3];

    // usa: func_0205c790
    void Reset();
    // usa: func_0205c840
    void Destroy();
    // usa: func_0205c904
    void Update(int numTicks);
    // usa: func_0205c96c
    void Draw(bool use2D);

    void EmptyFunction() const;
    int GetSelection() const;

    // usa: func_0205cb74
    void AddOption(const char* text);
    // usa: func_0205cc50
    void ConfigureTouchRegions(bool alignWidth, int widthAdjust);
    // usa: func_0205cd28
    void Show();
    // usa: func_0205cd94
    void ShowInstant();
    // usa: func_0205cde8
    bool IsActive();

    // fake function that gets optimized out to make inlines get emitted 
    // in the right place
    bool Unused_IsHidden();

    // usa: func_0205ce50
    void Hide();
    // usa: func_0205ce94
    void HideInstant();

    // usa: func_0205cecc
    // returns -1 if no selection made
    int GetTouchInputSelection() const;
};