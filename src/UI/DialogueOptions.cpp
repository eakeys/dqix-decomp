#include "UI/DialogueOptions.h"
#include "UI/Text.h"
#include "Input/Input.h"

extern "C"
{
    void func_0205bcdc(TouchGridMenu*, int);

    void func_0205c4a8(KeyInputGridMenu*, int);

    void func_0205c73c(void*);
    void func_0205c53c(void*);
    void func_0205c564(void*);

}

void DialogueOptionManager::Reset()
{
    func_0205c73c(&substruct_4_);
    func_0205c53c(&gridMenu_1c_);
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
    func_0205c564(&gridMenu_1c_);
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