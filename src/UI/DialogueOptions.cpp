#include "UI/DialogueOptions.h"
#include "UI/Text.h"
#include "Input/Input.h"

extern "C"
{
    void func_0205bf3c(InputGridMenu*);

    void func_0205c4a8(InputGridMenu*, int);

    void func_0205c73c(void*);
    void func_0205c53c(void*);
    void func_0205c564(void*);

}

extern const bool data_020e7db0[4], data_020e7db4[4];
extern const unsigned short data_020e7db8[5];
extern const int data_020e7dc4[4];

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

bool InputGridMenu::Update(int numTicks)
{
    if (numElements_0_ == 0)
        return false;
    if (gridWidth_8_ <= 0)
        return false;
    if (gridHeight_c_ <= 0)
        return false;

    unsigned char initialDelay = keyRepeatInitialDelay_39_;
    unsigned char loopDelay = keyRepeatLoopDelay_3a_;

    unsigned short keyCandidates[5];
    COPY_ARRAY(keyCandidates, data_020e7db8);
    
    for (int i = 0; keyCandidates[i] != 0xffff && currentKey_34_ == 0; i++)
    {
        unsigned short candidate = keyCandidates[i];
        if (!g_keyManager.TestKeysPressed(candidate))
            continue;

        if (changeY_36_ | changeX_37_)
        {
            func_0205bf3c(this);
            break;
        }

        int changeAmounts[4];
        COPY_ARRAY(changeAmounts, data_020e7dc4);
        bool changeYLookup[4];
        COPY_ARRAY(changeYLookup, data_020e7db0);
        bool changeXLookup[4];
        COPY_ARRAY(changeXLookup, data_020e7db4);

        currentKey_34_ = candidate;
        inputCountdown_38_ = initialDelay;
        changeAmount_30_ = changeAmounts[i];
        changeY_36_ = changeYLookup[i];
        changeX_37_ = changeXLookup[i];
        if (currentKey_34_ == KEY_UP || currentKey_34_ == KEY_DOWN)
        {
            int oldY = currentY_1c_;
            currentY_1c_ += changeAmount_30_;
            if (unknown_3d_)
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
                if (unknown_3c_)
                {
                    currentX_18_ = 0;
                    if (numPages_4_ > 1)
                        func_0205c4a8(this, 1);
                }
                else if (numPages_4_ == 1)
                    currentX_18_ = oldX;
                else
                {
                    currentX_18_ = 0;
                    func_0205c4a8(this, 1);
                }
            }
            else if (currentX_18_ < 0)
            {
                if (unknown_3c_)
                {
                    currentX_18_ = gridWidth_8_ - 1;
                    if (numPages_4_ > 1)
                        func_0205c4a8(this, -1);
                }
                else if (numPages_4_ == 1)
                    currentX_18_ = oldX;
                else
                {
                    currentX_18_ = gridWidth_8_ - 1;
                    func_0205c4a8(this, -1);
                }
            }
            if (GetCurrentElementIndex() >= numElements_0_)
            {
                if (numPages_4_ == 1)
                    currentX_18_ = oldX;
                else
                {
                    currentX_18_ = 0;
                    func_0205c4a8(this, 1);
                }
            }
        }
        return true;
    }

    if (changeY_36_)
    {
        if (g_keyManager.TestKeysHeld(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
        {
            unsigned char dt = numTicks;
            if (dt < inputCountdown_38_)
                inputCountdown_38_ -= dt;
            else
            {
                inputCountdown_38_ = loopDelay;
                changeY_36_ = true;
                changeX_37_ = false;
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
            func_0205bf3c(this);

        return true;
    }
    else if (changeX_37_)
    {
        if (g_keyManager.TestKeysHeld(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
        {
            unsigned char dt = numTicks;
            if (dt < inputCountdown_38_)
                inputCountdown_38_ -= dt;
            else
            {
                inputCountdown_38_ = loopDelay;
                changeY_36_ = false;
                changeX_37_ = true;
                currentX_18_ += changeAmount_30_;
                if (gridWidth_8_ <= currentX_18_)
                {
                    currentX_18_ = gridWidth_8_ - 1;
                    func_0205c4a8(this, 1);
                }
                else if (currentX_18_ < 0)
                {
                    currentX_18_ = 0;
                    func_0205c4a8(this, -1);
                }

                if (GetCurrentElementIndex() >= numElements_0_)
                {
                    if (numPages_4_ == 1)
                        currentX_18_ = 0;
                    else
                    {
                        currentX_18_ = 0;
                        func_0205c4a8(this, 1);
                    }
                }
            }
        }
        if (g_keyManager.TestKeysReleased(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
            func_0205bf3c(this);

        return true;
    }

    return false;
}

void DialogueOptionManager::Reset()
{
    func_0205c73c(&substruct_4_);
    func_0205c53c(&substruct_1c_);
    textBox_b4_.Initialize();
    textWidth_d0_ = 0;
    unknown_d4_ = 52;
    unknown_d8_ = 16;
    unknown_dc_ = 0;
    memset(substructOrArray_e0_, 0, sizeof(substructOrArray_e0_));
    memset(substructOrArray_f0_, 0, sizeof(substructOrArray_f0_));
    memset(widths_100_, 0, sizeof(widths_100_));
    memset(unknownArray_110_, 0, sizeof(unknownArray_110_));
    memset(textWords_120_, 0, sizeof(textWords_120_));
    memset(optionOffsets_220_, 0, sizeof(optionOffsets_220_));
    totalWords_230_ = 0;
    numOptions_232_ = 0;
    unknown_233_ = 0;
    unknown_234_ = 0;
}

void DialogueOptionManager::Release()
{
    TextManager::GetInstance()->unknown_19b0_ = 0;
    func_0205c73c(&substruct_4_);
    func_0205c564(&substruct_1c_);
    textBox_b4_.Initialize();
    textWidth_d0_ = 0;
    if (numOptions_232_ != 0)
    {
        TextManager* mgr = TextManager::GetInstance();
        for (int i = 0; i < numOptions_232_; i++)
            mgr->ReleaseText16Bit(textWords_120_ + optionOffsets_220_[i], 0x100);
    }

    memset(textWords_120_, 0, sizeof(textWords_120_));
    memset(optionOffsets_220_, 0, sizeof(optionOffsets_220_));
    totalWords_230_ = 0;
    numOptions_232_ = 0;
    unknown_233_ = 0;
    unknown_234_ = 0;
}

void DialogueOptionManager::AddOption(const char *text)
{
    if (text == NULL)
        return;

    TextManager* mgr = TextManager::GetInstance();
    optionOffsets_220_[numOptions_232_] = totalWords_230_;
    totalWords_230_ += mgr->AcquireText16Bit(text, &textWords_120_[totalWords_230_], 1);
    int width = mgr->MeasureText16Bit(&textWords_120_[optionOffsets_220_[numOptions_232_]], 256, 1);

    if (textWidth_d0_ < width)
        textWidth_d0_ = width;

    numOptions_232_++;
    if (unknown_dc_ < 8)
    {
        widths_100_[unknown_dc_] = width;
        unknownArray_110_[unknown_dc_] = 12;
    }
    else
        unknown_dc_ = 7;
    unknown_dc_++;
}