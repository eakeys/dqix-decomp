#include "Graphics/Text.h"
#include "System/Graphics.h"
#include <globaldefs.h>

extern "C"
{
    void func_0207df50(void*);
}

extern const int data_020e7a94[];

int TextManager::GetGlyphIndex(const FontIndexFile::Glyph* glyph)
{
    if (glyph == NULL)
        return -1;
    for (int i = 0; i < 0x80; i++)
    {
        if (glyphRefcounts_17b8_[i] == 0)
            continue;
        RenderGlyph* rg = &glyphs_[i];
        if (glyph != rg->indexGlyph)
            continue;
        if (unknown_195b_ & 0x80)
        {
            if (rg->unk_16_2)
                return i;
        }
        else if (unknown_195b_ & 2)
        {
            if (rg->unk_16_1)
                return i;
        }
        else
            return i;
    }
    return -1;
}

void TextManager::ReleaseAllGlyphs()
{
    unknown_19b0_ = 1;
    unknown_9b4_ = 0;
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset2();
        glyphRefcounts_17b8_[i] = 0;
    }
    if (unknown_1e28_ != NULL)
        func_0207df50(unknown_1e28_);
}

void TextManager::DrawPreprocessedText(const unsigned short* words, int x, int y, unsigned short color,
    int, int, bool use2D, unsigned char fontIndex, int lineSpacing)
{
    if (words == NULL || !unknown_19b0_ || words[0] == 0xffff)
        return;

    bool longRepeats = true;
    unsigned short firstWord = words[0];
    for (int j = 1; j < 10; j++)
    {
        if (firstWord != words[j])
        {
            longRepeats = false;
            break;
        }
    }

    if (longRepeats)
        return;

    GXFIFO_MATRIX_PUSH = 0;
    RenderGlyph* glyph;
    unsigned char lastGlyphIndex = 0xff;
    unsigned char defaultSpacing = data_020e7a94[fontIndex];
    int wordIdx = 0;
    int currentX = x;
    unsigned short thisWord = words[0];
    while (thisWord != ControlWord_End)
    {
        thisWord = words[wordIdx];
        wordIdx++;
        if ((thisWord & 0xff00) == 0xff00)
        {
            if (thisWord == ControlWord_LineBreak)
            {
                currentX = x;
                y += lineSpacing;
                lastGlyphIndex = 0xff;
                continue;
            }
            if (thisWord == ControlWord_Unknown_ff19)
            {
                currentX += defaultSpacing + 1;
                lastGlyphIndex = 0xff;
                continue;
            }
            break;
        }
        if (thisWord >= 0x80)
            break;
        glyph = &glyphs_[thisWord];
        if (glyph->unknown_8 < 0)
            break;
        unsigned char thisGlyphIndex = glyph->unknown_17;
        currentX += GetFontGlyphKerning(lastGlyphIndex, thisGlyphIndex, fontIndex);
        lastGlyphIndex = thisGlyphIndex;
        glyph->drawX = currentX;
        glyph->drawY = y;
        glyph->drawZ = 0;
        if (use2D)
        {
            unsigned char paletteIndex = 15;
            if (color != 0x7fff)
                paletteIndex = 5;
            RenderGlyph2D(renderer2D_34_, glyph, currentX, y, paletteIndex);
        }
        else
            glyph->Draw(color, NULL, 31);

        if (glyph->indexGlyph != NULL)
            currentX += glyph->indexGlyph->width + 1;
    }
    GXFIFO_MATRIX_POP = 1;
}