#pragma ipa file
#include "UI/DialogueOptions.h"
#include "UI/Text.h"
#include "Input/Input.h"

void DialogueOptionManager::Substruct_4::Reset()
{
    unknown_0 = unknown_4 = 0;
    unknown_8 = unknown_c = 8 << 12;
    unknown_10 = 0x7fff;
    unknown_12 = 30;
    unknown_13 = 0;
    unknown_14 = 30;
    unknown_15 = 0;
    unknown_16 = true;
}

void DialogueOptionManager::Substruct_4::SetFields(bool to)
{
    unknown_13 = 0;
    unknown_15 = 0;
    unknown_16 = to;
}

void DialogueOptionManager::Reset()
{
    unknown_4_.Reset();
    gridMenu_1c_.Reset();
    textBox_b4_.Reset();
    textWidth_d0_ = 0;
    horizontalSpacing_d4_ = 52;
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

void DialogueOptionManager::Destroy()
{
    TextManager::GetInstance()->unknown_19b0_ = 0;
    unknown_4_.Reset();
    gridMenu_1c_.Destroy();
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

void DialogueOptionManager::Update(int numTicks)
{
    if (unknown_233_ == 0 || unknown_234_ == 0)
        return;
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    mgr->substruct_19e0_.Update(numTicks);
    mgr->SetMainTextBoxActive();
    if (IsActive())
        gridMenu_1c_.Update(numTicks);
}

void DialogueOptionManager::Draw(bool use2D)
{
    if (unknown_233_ == 0 || numOptions_232_ == 0)
        return;
    TextManager* mgr = TextManager::GetInstance();
    int highlight = gridMenu_1c_.touchInput_4_.highlightedElement_30_;
    int pageOptionStart, pageOptionEnd;
    int selected = gridMenu_1c_.GetSelectedElementIndex();
    unsigned short defaultColor = 0x7fff;
    if (unknown_234_ == 0)
        defaultColor = 0x3def; // (15,15,15)
    
    if (!use2D)
    {
        textBox_b4_.color = defaultColor;
        mgr->SetActiveTextBox(&textBox_b4_);
        mgr->substruct_19e0_.Draw(true, 1);
        mgr->SetMainTextBoxActive();
    }

    if (!IsActive())
        return;

    mgr->unknown_19b0_ = true;
    int gridHeight = gridMenu_1c_.keyInput_54_.gridHeight_c_;
    int drawX, drawY, baseY;
    pageOptionStart = gridHeight * gridMenu_1c_.keyInput_54_.currentPage_14_;
    pageOptionEnd = pageOptionStart + gridHeight;
    drawX = textBox_b4_.x + 12;
    drawY = baseY = textBox_b4_.y + 8;

    for (int i = pageOptionStart; i < pageOptionEnd; i++)
    {
        unsigned short textColor = defaultColor;
        if (i == highlight && highlight == selected)
            textColor = 0xf0a; // green
        
        mgr->DrawPreprocessedText(textWords_120_ + optionOffsets_220_[i],
            drawX, drawY, textColor, 12, 20, use2D, 1, 17);
        drawY += verticalSpacing_d8_;
    }


    if (gridMenu_1c_.touchInputEnabled_94_ | gridMenu_1c_.keyInputEnabled_95_)
    {
        int currentX = gridMenu_1c_.GetCurrentX();
        int currentY = gridMenu_1c_.GetCurrentY();
        drawX = currentX * horizontalSpacing_d4_ + (drawX - 16);
        drawY = currentY * verticalSpacing_d8_ + (baseY - 2);
        OBJSpriteSet* cursor = mgr->cursorObjects_2d8_;
        OBJSpriteSet::List* spriteList = mgr->cursorSpriteList_2e0_;
        unsigned short var14 = mgr->unknown_2e4_;
        if (cursor == NULL || spriteList == NULL)
            return;

        spriteList->PlaySpriteAnimation(var14);
        OBJSpriteSet::Sprite* sprite = spriteList->GetSprite(var14);
        if (sprite != NULL)
            sprite->flags |= 8;

        sprite = spriteList->GetSprite(var14);
        if (sprite != NULL)
        {
            sprite->drawX = drawX;
            sprite->drawY = drawY;
        }
        cursor->DrawAll();
    }
}

void DialogueOptionManager::EmptyFunction() const {}

int DialogueOptionManager::GetSelection() const
{
    return gridMenu_1c_.GetSelectedElementIndex();
}

void DialogueOptionManager::AddOption(const char* text)
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

#ifndef jpn
    if (alignWidth)
    {
        short roundedUp = (textBox_b4_.width + 7) & ~7;
        textBox_b4_.width = roundedUp + widthAdjust;
    }
#endif

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

void DialogueOptionManager::Show()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    if (!mgr->substruct_19e0_.IsBoxActive())
        mgr->substruct_19e0_.BeginDisplay();
    mgr->SetMainTextBoxActive();
}

void DialogueOptionManager::ShowInstant()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    if (!mgr->substruct_19e0_.IsBoxActive())
    {
        mgr->substruct_19e0_.activeBox_->status = 2;
        mgr->substruct_19e0_.activeBox_->scaleY = 1 << 12;
    }
    mgr->SetMainTextBoxActive();
}

bool DialogueOptionManager::IsActive()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    bool isActive = mgr->substruct_19e0_.IsBoxActive();
    mgr->SetMainTextBoxActive();
    return isActive;
}

bool DialogueOptionManager::Unused_IsHidden()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    bool hidden = mgr->substruct_19e0_.IsBoxHidden();
    mgr->SetMainTextBoxActive();
    return hidden;
}

void DialogueOptionManager::Hide()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    if (!mgr->substruct_19e0_.IsBoxHidden())
        mgr->substruct_19e0_.EndDisplay();
    mgr->SetMainTextBoxActive();
}

void DialogueOptionManager::HideInstant()
{
    TextManager* mgr = TextManager::GetInstance();
    mgr->SetActiveTextBox(&textBox_b4_);
    mgr->substruct_19e0_.ResetBoxStatus();
    mgr->SetMainTextBoxActive();
}

int DialogueOptionManager::GetTouchInputSelection() const
{
    if (!gridMenu_1c_.touchInput_4_.IsSelectionConfirmed(1))
        return -1;
    return gridMenu_1c_.GetSelectedElementIndex();
}