#pragma once

#include <std_library_functions.h>
#include "../Graphics/Vector.h"
#include "TextBox.h"

/*
    Function runtime info for substructs:
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

class BasicGridMenu
{
public:
    enum ElementOrdering
    {
        ElementOrdering_ColumnMajor = 0,
        ElementOrdering_RowMajor = 1,
    };

    int numElements_0_;
    int numPages_4_;
    // width and height i.e. number of cols & rows
    int gridWidth_8_;
    int gridHeight_c_;
    unsigned char elementOrdering_10_;
    char pad_11[3];
    int currentPage_14_;
    int currentX_18_;
    int currentY_1c_;
    int* pOuterArrayLength_20_;
    int* pInnerArrayLength_24_;
    int* pOuterIndex_28_;
    int* pInnerIndex_2c_;

public:
    // usa: func_0205ba2c
    void Reset();
    // usa: func_0205ba68
    void SetSize(int w, int h, unsigned char elementOrdering);

    // usa: func_0205bacc
    void SetElementCount(int to);
    // usa: func_0205bafc
    int GetElementCount() const;

    // usa: func_0205bb04
    void SetCurrentElementIndex(int idx);
    // usa: func_0205bb84
    int GetCurrentElementIndex() const;
};

class InputGridMenu : public BasicGridMenu
{
public:
    int changeAmount_30_;
    unsigned short currentKey_34_;
    bool changingY_36_;
    bool changingX_37_;
    unsigned char inputCountdown_38_;
    unsigned char keyRepeatInitialDelay_39_;
    unsigned char keyRepeatLoopDelay_3a_;
    char unk_3b[1];
    bool wrapX_3c_;
    bool wrapY_3d_;
    char pad_3e[2];

public:
    void Initialize();
    void CancelInput();
    bool Update(int numTicks);
    void AdvancePage(int change);
    // returns a range [start, end)
    void GetCurrentPageElementRange(int* outStart, int* outEnd) const;
};

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
        char unk_0[4];

        struct Subsubstruct_4 // 0205bbcc
        {
            BasicGridMenu base_0_;
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
        InputGridMenu subsubstruct_54_;
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