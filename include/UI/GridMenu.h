#pragma once

#include "../Input/Input.h"
#include <globaldefs.h>

// sizeof == 0x30. Not responsible for any rendering
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
    void Reset();
    void SetSize(int w, int h, unsigned char elementOrdering);

    void SetElementCount(int to);
    int GetElementCount() const;

    void SetCurrentElementIndex(int idx);
    int GetCurrentElementIndex() const;
};

// sizeof == 0x50. Not responsible for any rendering
class TouchGridMenu : public BasicGridMenu
{
public:
    // the element that will show up green
    int highlightedElement_30_;
    int numClickableElements_34_;
    short originX_38_;
    short originY_3a_;
    short* elementXValues_3c_;
    short* elementYValues_40_;
    short* elementWidths_44_;
    short* elementHeights_48_;
    bool choiceConfirmed_4c_;
    bool unknown_4d_;
    bool choiceHighlighted_4e_;
    char pad_4f[1];

public:
    void Reset();

    void ClearFlags();
    void SetHighlight(int);

    // Returns true if an element has just been clicked
    bool Update(int numTicks);
    void SetCurrentElementAndRemoveHighlight(int selection);

    // returns -1 if no element is highlighted (green)
    int GetHighlightedElement() const;
    bool IsSelectionConfirmed(int unused) const;

    // pass an array of each element's x-value, an array of each element's y-value, ... 
    void SetElementBounds(int num, short* x, short* y, short* w, short* h);

private:
    // returns the ID of the clicked element (-1 if none)
    int ProcessClick(int touchX, int touchY);

public:
    // returns true if an element has been clicked this frame,
    // doesn't store the selected element
    bool TestClick();
};

// sizeof == 0x40. Not responsible for any rendering
class KeyInputGridMenu : public BasicGridMenu
{
public:
    // how much to change the relevant coordinate by (+1 when pressing/holding
    // right or down, -1 for left or up)
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
    void Reset();
    void CancelInput();
    // returns true if the selection changed
    bool Update(int numTicks);
    void AdvancePage(int change);
    // returns a range [start, end)
    void GetCurrentPageElementRange(int* outStart, int* outEnd) const;
};

// sizeof == 0x98. Not responsible for any rendering
// TouchGridMenu & KeyInputGridMenu both being derived from the same base class
// makes this a bit of a mess, lots of duplicated fields
class DualInputGridMenu
{
public:
    char unk_0[4];
    TouchGridMenu touchInput_4_;
    KeyInputGridMenu keyInput_54_;
    bool touchInputEnabled_94_;
    // generally used to prevent key input, but if you manually switch it
    // off you lose touch input too. probably just a bug (which never
    // comes up naturally bc key input is always enabled)?
    bool keyInputEnabled_95_;
    char pad_96[2];

public:
    void Reset();
    void Destroy();

    int GetSelectedElementIndex() const;

    // returns 1 = received key input, 2 = received touch input to highlight,
    // 0 otherwise. Returns 0 for confirming touch input
    int Update(int numTicks);
    bool TestClick();
    int GetPageCount() const;
    int GetCurrentPage() const;
};