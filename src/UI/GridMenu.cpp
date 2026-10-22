#pragma ipa file
#include "UI/GridMenu.h"

void BasicGridMenu::Reset()
{
    numElements_0_ = 0;
    numPages_4_ = 1;
    gridWidth_8_ = 1;
    gridHeight_c_ = 1;
    elementOrdering_10_ = ElementOrdering_ColumnMajor;
    currentPage_14_ = 0;
    currentX_18_ = 0;
    currentY_1c_ = 0;
    pOuterArrayLength_20_ = 0;
    pInnerArrayLength_24_ = 0;
    pOuterIndex_28_ = 0;
    pInnerIndex_2c_ = 0;
}

void BasicGridMenu::SetSize(int w, int h, unsigned char elementOrdering)
{
    gridWidth_8_ = w;
    gridHeight_c_ = h;
    elementOrdering_10_ = elementOrdering;
    if (elementOrdering == ElementOrdering_ColumnMajor)
    {
        pOuterArrayLength_20_ = &gridWidth_8_;
        pInnerArrayLength_24_ = &gridHeight_c_;
        pOuterIndex_28_ = &currentX_18_;
        pInnerIndex_2c_ = &currentY_1c_;
    }
    else if (elementOrdering == ElementOrdering_RowMajor)
    {
        pOuterArrayLength_20_ = &gridHeight_c_;
        pInnerArrayLength_24_ = &gridWidth_8_;
        pOuterIndex_28_ = &currentY_1c_;
        pInnerIndex_2c_ = &currentX_18_;
    }
}

void BasicGridMenu::SetElementCount(int to)
{
    if (to < 0)
        return;
    numElements_0_ = to;
    // prevent going off the end if we reduced the count
    if (GetCurrentElementIndex() >= to)
        SetCurrentElementIndex(to - 1);
}

int BasicGridMenu::GetElementCount() const
{
    return numElements_0_;
}

void BasicGridMenu::SetCurrentElementIndex(int idx)
{
    if (idx < 0)
        return;
    if (idx >= numElements_0_)
        return;
    int pageStride = gridHeight_c_ * gridWidth_8_;
    if (pageStride == 0)
        return;

    currentPage_14_ = idx / pageStride;
    idx %= pageStride;
    *pInnerIndex_2c_ = idx % *pInnerArrayLength_24_;
    *pOuterIndex_28_ = idx / *pInnerArrayLength_24_;
}

int BasicGridMenu::GetCurrentElementIndex() const
{
    short offsetOnPage = (*pInnerArrayLength_24_) * (*pOuterIndex_28_) + (*pInnerIndex_2c_);
    short pageBase = currentPage_14_ * gridHeight_c_ * gridWidth_8_;
    return pageBase + offsetOnPage;
}

void TouchGridMenu::Reset() 
{
    this->BasicGridMenu::Reset();
    highlightedElement_30_ = -1;
    numClickableElements_34_ = 0;
    originX_38_ = 0;
    originY_3a_ = 0;
    elementXValues_3c_ = NULL;
    elementYValues_40_ = NULL;
    elementWidths_44_ = NULL;
    elementHeights_48_ = NULL;
    choiceConfirmed_4c_ = false;
    unknown_4d_ = false;
    choiceHighlighted_4e_ = false;
}

void TouchGridMenu::ClearFlags()
{
    choiceConfirmed_4c_ = false;
    unknown_4d_ = false;
    choiceHighlighted_4e_ = false;
}

void TouchGridMenu::SetHighlight(int highlight)
{
    highlightedElement_30_ = highlight;
}

bool TouchGridMenu::Update(int)
{
    if (numElements_0_ == 0)
        return false;

    int touchX, touchY;
    g_touchInput.GetTouchPoint(&touchX, &touchY);
    if (g_touchInput.pressed_ && !choiceHighlighted_4e_)
    {
        choiceHighlighted_4e_ = true;
        int priorElement = highlightedElement_30_;
        highlightedElement_30_ = ProcessClick(touchX, touchY);
        if (highlightedElement_30_ < 0)
        {
            choiceHighlighted_4e_ = false;
            return false;
        }
        choiceConfirmed_4c_ = true;
        if (priorElement != highlightedElement_30_)
            choiceConfirmed_4c_ = false;
        return true;
    }
    else
    {
        choiceConfirmed_4c_ = false;
        choiceHighlighted_4e_ = false;
        return false;
    }
}

void TouchGridMenu::SetCurrentElementAndRemoveHighlight(int selection)
{
    if (selection < 0)
        return;
    if (numElements_0_ <= selection)
        return;

    highlightedElement_30_ = -1;
    SetCurrentElementIndex(selection);
}

int TouchGridMenu::GetHighlightedElement() const
{
    if (!choiceHighlighted_4e_)
        return -1;
    return GetCurrentElementIndex();
}

bool TouchGridMenu::IsSelectionConfirmed(int) const
{
    bool confirmed = false;
    if (g_touchInput.pressed_ && choiceHighlighted_4e_ == 1 && choiceConfirmed_4c_)
        confirmed = true;
    return confirmed;
}

void TouchGridMenu::SetElementBounds(int num, short* x, short* y, short* w, short* h)
{
    numClickableElements_34_ = num;
    elementXValues_3c_ = x;
    elementYValues_40_ = y;
    elementWidths_44_ = w;
    elementHeights_48_ = h;
}

int TouchGridMenu::ProcessClick(int touchX, int touchY)
{
    if (numClickableElements_34_ == 0)
        return -1;
    if (elementXValues_3c_ == NULL)
        return -1;
    if (elementYValues_40_ == NULL)
        return -1;
    if (elementWidths_44_ == NULL)
        return -1;
    if (elementHeights_48_ == NULL)
        return -1;

    touchX -= originX_38_;
    touchY -= originY_3a_;

    for (int innerCoord = 0; innerCoord < *pInnerArrayLength_24_; innerCoord++)
    {
        for (int outerCoord = 0; outerCoord < *pOuterArrayLength_20_; outerCoord++)
        {
            int elementIdx = outerCoord * *pInnerArrayLength_24_ + innerCoord;
            if (numClickableElements_34_ <= elementIdx)
                continue;
            
            int eltLeft = elementXValues_3c_[elementIdx];
            int eltTop = elementYValues_40_[elementIdx];
            int eltRight = eltLeft + elementWidths_44_[elementIdx];
            int eltBottom = eltTop + elementHeights_48_[elementIdx];

            if (touchX < eltLeft || eltRight < touchX || touchY < eltTop || eltBottom < touchY)
                continue;

            *pInnerIndex_2c_ = innerCoord;
            *pOuterIndex_28_ = outerCoord;

            int highlight = GetHighlightedElement();
            if (numElements_0_ <= highlight)
                highlight = -1;
            SetCurrentElementAndRemoveHighlight(highlight);
            return highlight;
        }
    }

    return -1;
}

bool TouchGridMenu::TestClick()
{
    if (g_touchInput.pressed_)
    {
        int touchX, touchY;
        g_touchInput.GetTouchPoint(&touchX, &touchY);
        return ProcessClick(touchX, touchY) >= 0;
    }
    return false;
}

void KeyInputGridMenu::Reset()
{
    this->BasicGridMenu::Reset();
    changeAmount_30_ = 0;
    currentKey_34_ = 0;
    changingY_36_ = false;
    changingX_37_ = false;
    inputCountdown_38_ = 0;
    keyRepeatInitialDelay_39_ = 20;
    keyRepeatLoopDelay_3a_ = 10;
    wrapX_3c_ = true;
    wrapY_3d_ = true;
}

void KeyInputGridMenu::CancelInput()
{
    changeAmount_30_ = 0;
    currentKey_34_ = 0;
    changingY_36_ = false;
    changingX_37_ = false;
    inputCountdown_38_ = 0;
}

bool KeyInputGridMenu::Update(int numTicks)
{
    if (numElements_0_ == 0)
        return false;
    if (gridWidth_8_ <= 0)
        return false;
    if (gridHeight_c_ <= 0)
        return false;

    unsigned char initialDelay = keyRepeatInitialDelay_39_;
    unsigned char loopDelay = keyRepeatLoopDelay_3a_;

    unsigned short keyCandidates[] = { KEY_UP, KEY_DOWN, KEY_LEFT, KEY_RIGHT, -1 };
    
    for (int i = 0; keyCandidates[i] != 0xffff && currentKey_34_ == 0; i++)
    {
        unsigned short candidate = keyCandidates[i];
        if (!g_keyManager.TestKeysPressed(candidate))
            continue;

        if (changingY_36_ | changingX_37_)
        {
            CancelInput();
            break;
        }

        int changeAmounts[4] = { -1, 1, -1, 1 };
        bool changeYLookup[4] = { true, true, false, false };
        bool changeXLookup[4] = { false, false, true, true };

        currentKey_34_ = candidate;
        inputCountdown_38_ = initialDelay;
        changeAmount_30_ = changeAmounts[i];
        changingY_36_ = changeYLookup[i];
        changingX_37_ = changeXLookup[i];
        if (currentKey_34_ == KEY_UP || currentKey_34_ == KEY_DOWN)
        {
            int oldY = currentY_1c_;
            currentY_1c_ += changeAmount_30_;
            if (wrapY_3d_)
            {
                if (gridHeight_c_ <= currentY_1c_)
                    currentY_1c_ = 0;
                else if (currentY_1c_ < 0)
                    currentY_1c_ = gridHeight_c_ - 1;
                
                if (currentKey_34_ == KEY_UP)
                {
                    int elementCount = numElements_0_;
                    if (elementCount <= GetCurrentElementIndex())
                        SetCurrentElementIndex(elementCount - 1);
                }
                else if (currentKey_34_ == KEY_DOWN)
                {
                    if (numElements_0_ <= GetCurrentElementIndex())
                        currentY_1c_ = 0;
                }
            }
            else
            {
                if (gridHeight_c_ <= currentY_1c_)
                    currentY_1c_ = oldY;
                else if (currentY_1c_ < 0)
                    currentY_1c_ = oldY;
                if (GetCurrentElementIndex() >= numElements_0_)
                    currentY_1c_ = oldY;
            }
        }
        else if (currentKey_34_ == KEY_LEFT || currentKey_34_ == KEY_RIGHT)
        {
            int oldX = currentX_18_;
            currentX_18_ += changeAmount_30_;
            if (gridWidth_8_ <= currentX_18_)
            {
                if (wrapX_3c_)
                {
                    currentX_18_ = 0;
                    if (numPages_4_ > 1)
                        AdvancePage(1);
                }
                else if (numPages_4_ == 1)
                    currentX_18_ = oldX;
                else
                {
                    currentX_18_ = 0;
                    AdvancePage(1);
                }
            }
            else if (currentX_18_ < 0)
            {
                if (wrapX_3c_)
                {
                    currentX_18_ = gridWidth_8_ - 1;
                    if (numPages_4_ > 1)
                        AdvancePage(-1);
                }
                else if (numPages_4_ == 1)
                    currentX_18_ = oldX;
                else
                {
                    currentX_18_ = gridWidth_8_ - 1;
                    AdvancePage(-1);
                }
            }
            if (GetCurrentElementIndex() >= numElements_0_)
            {
                if (numPages_4_ == 1)
                    currentX_18_ = oldX;
                else
                {
                    currentX_18_ = 0;
                    AdvancePage(1);
                }
            }
        }
        return true;
    }

    if (changingY_36_)
    {
        if (g_keyManager.TestKeysHeld(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
        {
            unsigned char dt = numTicks;
            if (dt < inputCountdown_38_)
                inputCountdown_38_ -= dt;
            else
            {
                inputCountdown_38_ = loopDelay;
                changingY_36_ = true;
                changingX_37_ = false;
                currentY_1c_ += changeAmount_30_;
                if (gridHeight_c_ <= currentY_1c_)
                    currentY_1c_ = gridHeight_c_ - 1;
                else if (currentY_1c_ < 0)
                    currentY_1c_ = 0;
                int elementCount = numElements_0_;
                if (elementCount <= GetCurrentElementIndex())
                    SetCurrentElementIndex(elementCount - 1);
                if (currentKey_34_ == KEY_UP || currentKey_34_ == KEY_LEFT)
                {
                    int elementCount = numElements_0_;
                    if (elementCount <= GetCurrentElementIndex())
                        SetCurrentElementIndex(elementCount - 1);
                }
                else if (currentKey_34_ == KEY_DOWN || currentKey_34_ == KEY_RIGHT)
                {
                    if (numElements_0_ <= GetCurrentElementIndex())
                        currentY_1c_ = 0;
                }
            }
        }
        if (g_keyManager.TestKeysReleased(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
            CancelInput();

        return true;
    }
    else if (changingX_37_)
    {
        if (g_keyManager.TestKeysHeld(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
        {
            unsigned char dt = numTicks;
            if (dt < inputCountdown_38_)
                inputCountdown_38_ -= dt;
            else
            {
                inputCountdown_38_ = loopDelay;
                changingY_36_ = false;
                changingX_37_ = true;
                currentX_18_ += changeAmount_30_;
                if (gridWidth_8_ <= currentX_18_)
                {
                    currentX_18_ = gridWidth_8_ - 1;
                    AdvancePage(1);
                }
                else if (currentX_18_ < 0)
                {
                    currentX_18_ = 0;
                    AdvancePage(-1);
                }

                if (GetCurrentElementIndex() >= numElements_0_)
                {
                    if (numPages_4_ == 1)
                        currentX_18_ = 0;
                    else
                    {
                        currentX_18_ = 0;
                        AdvancePage(1);
                    }
                }
            }
        }
        if (g_keyManager.TestKeysReleased(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
            CancelInput();

        return true;
    }

    return false;
}

void KeyInputGridMenu::AdvancePage(int change)
{
    currentPage_14_ += change;
    if (numPages_4_ <= currentPage_14_)
        currentPage_14_ -= numPages_4_;
    else if (currentPage_14_ < 0)
        currentPage_14_ += numPages_4_;

    int elementCount = GetElementCount();
    if (elementCount <= GetCurrentElementIndex())
        SetCurrentElementIndex(elementCount - 1);
}

void KeyInputGridMenu::GetCurrentPageElementRange(int* outStart, int* outEnd) const
{
    int pageStride = gridWidth_8_ * gridHeight_c_;
    
    *outStart = pageStride * currentPage_14_;
    *outEnd = *outStart + pageStride;

    if (numElements_0_ < *outEnd)
        *outEnd = numElements_0_;
}

void DualInputGridMenu::Reset()
{
    touchInput_4_.Reset();
    keyInput_54_.Reset();
    touchInputEnabled_94_ = false;
    keyInputEnabled_95_ = false;
}

void DualInputGridMenu::Destroy()
{
    Reset();
}

int DualInputGridMenu::GetSelectedElementIndex() const
{
    int idx = -1;
    if (touchInputEnabled_94_ && touchInput_4_.choiceConfirmed_4c_)
        idx = touchInput_4_.GetHighlightedElement();
    if (!keyInputEnabled_95_)
        return idx;

    idx = keyInput_54_.GetCurrentElementIndex();
    return idx;
}

int DualInputGridMenu::Update(int numTicks)
{
    int outcome = 0;
    int highlight = touchInput_4_.highlightedElement_30_;
    if (touchInputEnabled_94_ && TestClick())
    {
        keyInput_54_.SetCurrentElementIndex(touchInput_4_.GetHighlightedElement());
        keyInput_54_.CancelInput();
        touchInput_4_.ClearFlags();
        outcome = 1;
    }
    else if (touchInputEnabled_94_ && touchInput_4_.Update(numTicks))
    {
        int newHighlight = touchInput_4_.GetHighlightedElement();
        if (highlight != newHighlight)
            outcome = 2;
        keyInput_54_.SetCurrentElementIndex(newHighlight);
        keyInput_54_.CancelInput();
    }
    else if (keyInputEnabled_95_)
    {
        int prevKeySelection = keyInput_54_.GetCurrentElementIndex();
        if (keyInput_54_.Update(numTicks) && prevKeySelection != keyInput_54_.GetCurrentElementIndex())
        {
            outcome = 1;
            touchInput_4_.SetCurrentElementAndRemoveHighlight(keyInput_54_.GetCurrentElementIndex());
            touchInput_4_.highlightedElement_30_ = -1;
            touchInput_4_.ClearFlags();
        }
    }

    return outcome;
}

bool DualInputGridMenu::TestClick()
{
    bool clicked = false;
    if (touchInputEnabled_94_)
        clicked = touchInput_4_.TestClick();
    return clicked;
}

int DualInputGridMenu::GetPageCount() const
{
    int count = 0;
    if (touchInputEnabled_94_ && touchInput_4_.choiceConfirmed_4c_)
        count = touchInput_4_.numPages_4_;
    if (keyInputEnabled_95_)
        count = keyInput_54_.numPages_4_;
    return count;
}

int DualInputGridMenu::GetCurrentPage() const
{
    int current = 0;
    if (touchInputEnabled_94_ && touchInput_4_.choiceConfirmed_4c_)
        current = touchInput_4_.currentPage_14_;
    if (keyInputEnabled_95_)
        current = keyInput_54_.currentPage_14_;
    return current;
}