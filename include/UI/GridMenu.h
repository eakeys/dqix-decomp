#pragma once

#include "../Input/Input.h"
#include <globaldefs.h>

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

class TouchGridMenu : public BasicGridMenu
{
public:
    int highlightedElement_30_;
    int numClickableElements_34_;
    short originX_38_;
    short originY_3a_;
    short* elementXValues_3c_;
    short* elementYValues_40_;
    short* elementWidths_44_;
    short* elementHeights_48_;
    bool unknown_4c_;
    bool unknown_4d_;
    bool unknown_4e_;
    char pad_4f[1];

public:
    // usa: func_0205bbcc
    void Reset();

    // usa: func_0205bc10
    void ClearFlags();

    // usa: func_0205bc24
    void SetHighlight(int);

    // usa: func_0205bc2c
    // Returns true if an element has just been clicked
    bool Update(int numTicks);
    // usa: func_0205bcdc
    void ChangeSelectionExternal(int selection);

    // usa: func_0205bd04
    int GetSelectedElementIndex() const;
    // usa: func_0205bd20
    bool IsSelectionConfirmed() const;

    // usa: func_0205bd58
    void SetElementBounds(int num, short* x, short* y, short* w, short* h);

    // usa: func_0205bc2c
    // returns the ID of the clicked element (-1 if none)
    int ProcessClick(int touchX, int touchY);

    // usa: func_0205bea0
    // returns true if an element has been clicked this frame,
    // doesn't store the selected element
    bool TestClick();
};

class KeyInputGridMenu : public BasicGridMenu
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
    // returns true if the selection changed
    bool Update(int numTicks);
    void AdvancePage(int change);
    // returns a range [start, end)
    void GetCurrentPageElementRange(int* outStart, int* outEnd) const;
};

// sizeof == 0x98
class DualInputGridMenu
{
public:
    char unk_0[4];
    TouchGridMenu touchInput_4_;
    KeyInputGridMenu keyInput_54_;
    unsigned char unknown_94_;
    unsigned char unknown_95_;
    char pad_96[2];

public:
    void Reset();
    void Destroy();

    int GetSelectedElementIndex() const;

    int Update(int numTicks);
    bool TestClick();
    int GetPageCount() const;
    int GetCurrentPage() const;
};