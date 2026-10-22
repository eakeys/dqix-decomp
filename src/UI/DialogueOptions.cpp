#include "UI/DialogueOptions.h"
#include "UI/Text.h"
#include "Input/Input.h"

extern "C"
{
    void func_0202f6a4(void*); // makes current box visible?

    void func_0205c73c(void*);
    void func_0205c53c(void*);
    void func_0205c564(void*);

    int func_0205cd6c(void*);

}

void DialogueOptionManager::Reset()
{
    func_0205c73c(&substruct_4_);
    func_0205c53c(&gridMenu_1c_);
    textBox_b4_.Reset();
    textWidth_d0_ = 0;
    unknown_d4_ = 52;
    verticalSpacing_d8_ = 16;
    numTouchRegions_dc_ = 0;
    memset(touchRegionX_e0_, 0, sizeof(touchRegionX_e0_));
    memset(touchRegionY_f0_, 0, sizeof(touchRegionY_f0_));
    memset(touchRegionWidth_100_, 0, sizeof(touchRegionWidth_100_));
    memset(touchRegionHeight_110_, 0, sizeof(touchRegionHeight_110_));
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
    func_0205c564(&gridMenu_1c_);
    textBox_b4_.Reset();
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
    if (numTouchRegions_dc_ < 8)
    {
        touchRegionWidth_100_[numTouchRegions_dc_] = width;
        touchRegionHeight_110_[numTouchRegions_dc_] = 12;
    }
    else
        numTouchRegions_dc_ = 7;
    numTouchRegions_dc_++;
}

void DialogueOptionManager::ConfigureTouchRegions(bool alignWidth, int widthAdjust)
{
    (void)TextManager::GetInstance();
    int numOptions = gridMenu_1c_.keyInput_54_.gridHeight_c_;
    int minHeight = numOptions * 12 + 16;
    textBox_b4_.width = textWidth_d0_ + 24;
    textBox_b4_.height = minHeight + (numOptions - 1) * (verticalSpacing_d8_ - 12);

    if (alignWidth)
    {
        short roundedUp = (textBox_b4_.width + 7) & ~7;
        textBox_b4_.width = roundedUp + widthAdjust;
    }

    for (int i = 0; i < numTouchRegions_dc_; i++)
    {
        touchRegionX_e0_[i] = 12;
        touchRegionY_f0_[i] = verticalSpacing_d8_ * i + 8;
    }

    short oX = textBox_b4_.x, oY = textBox_b4_.y;

    gridMenu_1c_.touchInput_4_.originX_38_ = oX;
    gridMenu_1c_.touchInput_4_.originY_3a_ = oY;

    gridMenu_1c_.touchInput_4_.SetElementBounds(numTouchRegions_dc_,
        touchRegionX_e0_, touchRegionY_f0_, touchRegionWidth_100_, touchRegionHeight_110_);
}

void DialogueOptionManager::ShowBackground3D()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    if (func_0205cd6c(&mgr->substruct_19e0_) == 0)
        func_0202f6a4(&mgr->substruct_19e0_);
    mgr->SetMainTextBoxActive();
}

int DialogueOptionManager::GetTouchInputSelection() const
{
    if (!gridMenu_1c_.touchInput_4_.IsSelectionConfirmed(1))
        return -1;
    return gridMenu_1c_.GetSelectedElementIndex();
}