#include "Graphics/Text.h"
#include "System/Graphics.h"
#include <globaldefs.h>

extern "C"
{
    void func_0204483c(TextManager*, const char*, char*);
    unsigned short func_020459b8(TextManager*, unsigned char, FontIndexFile::Glyph*, unsigned char);

    void func_0207df50(void*);
    void func_0207df90(void*);
    void func_0207dfac(void*);
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
    numGlyphsInUse_9b4_ = 0;
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
            RenderGlyph2D(mainUIElement_34_, glyph, currentX, y, paletteIndex);
        }
        else
            glyph->Draw(color, NULL, 31);

        if (glyph->indexGlyph != NULL)
            currentX += glyph->indexGlyph->width + 1;
    }
    GXFIFO_MATRIX_POP = 1;
}

unsigned short TextManager::AcquireText16Bit(const char *text, unsigned short *outWords, unsigned char fontIndex)
{
    if (text == NULL || outWords == NULL)
        return 0;

    char transformedText[512] = {0};
    func_0204483c(this, text, transformedText);
    unsigned short counter = 0;
    int srcIndex = 0;
    int dstIndex = 0;
    if (unknown_1e28_ != NULL)
        func_0207df90(unknown_1e28_);

    while (true)
    {
        if (dstIndex > 0 && outWords[dstIndex - 1] == ControlWord_End)
            break;
        unsigned short controlCopy, control;
        memcpy(&control, transformedText + srcIndex, 2);
        if ((control & 0xff00) == 0xff00)
        {
            memcpy(&controlCopy, transformedText + srcIndex, 2);
            outWords[dstIndex] = controlCopy;
            srcIndex += 2;
            dstIndex++;
            counter++;

            int numExtraWords = GetControlWordNumExtraWords(&controlCopy);
            memcpy(outWords + dstIndex, transformedText + srcIndex, numExtraWords * 2L);
            counter += numExtraWords;
            srcIndex += numExtraWords * 2;
            dstIndex += numExtraWords;
        }
        else
        {
            FontIndexFile::Glyph* glyph = GetNextFontGlyph(transformedText + srcIndex, fontIndex);
            int glyphIndex = GetNextFontGlyphIndex(transformedText + srcIndex, fontIndex);
            if (glyph != NULL && glyphIndex >= 0)
            {
                int resourceIndex = GetGlyphIndex(glyph);
                if (resourceIndex == -1)
                {
                    outWords[dstIndex] = func_020459b8(this, glyphIndex, glyph, fontIndex);
                    numGlyphsInUse_9b4_++;
                    numGlyphsInUse_9b4_ &= 0x7f;
                }
                else
                {
                    unsigned short outputWord = resourceIndex;
                    outWords[dstIndex] = outputWord;
                    if (glyphRefcounts_17b8_[outputWord] < 0xff)
                        glyphRefcounts_17b8_[outputWord]++;
                }
                srcIndex += glyph->tagLength;
            }
            else
            {
                outWords[dstIndex] = ControlWord_Unknown_ff19;
                srcIndex++;
            }
            dstIndex++;
            counter++;
        }
    }
    if (unknown_1e28_ != NULL)
        func_0207dfac(unknown_1e28_);
    return counter;
}

int TextManager::MeasureText16Bit(const unsigned short *wordList, int numWords, unsigned char fontIndex)
{
    if (wordList == NULL)
        return 0;
    if (numWords < 0)
        return 0;

    unsigned char defaultSpacing = data_020e7a94[fontIndex];
    int currentLength = 0;
    int maxLength = 0;
    unsigned char lastGlyphIndex = 0xff;

    for (int i = 0; i < numWords; i++)
    {
        unsigned short word = wordList[i];
        if (word == ControlWord_End)
            break;
        else if (word == ControlWord_LineBreak)
        {
            if (maxLength < currentLength)
                maxLength = currentLength;
            currentLength = 0;
            lastGlyphIndex = 0xff;
        }
        else if (word == ControlWord_Unknown_ff19)
        {
            lastGlyphIndex = 0xff;
            currentLength += defaultSpacing + 1;
        }
        else if (IsNextCharacterControlAlt(&word))
        {
            lastGlyphIndex = 0xff;
        }
        else
        {
            RenderGlyph* rglyph = &glyphs_[word];
            unsigned char thisGlyphIndex = rglyph->unknown_17;
            currentLength += GetFontGlyphKerning(lastGlyphIndex, thisGlyphIndex, fontIndex);
            lastGlyphIndex = thisGlyphIndex;
            int width = defaultSpacing;
            if (rglyph->indexGlyph != NULL)
                width = rglyph->indexGlyph->width;
            currentLength += width + 1;
        }
    }

    if (currentLength < maxLength)
        currentLength = maxLength;
    return currentLength;
}

void TextManager::ReleaseText16Bit(unsigned short *wordList, int len)
{
    if (wordList == NULL || len < 0)
        return;

    for (int i = 0; i < len; i++)
    {
        unsigned short word = wordList[i];
        wordList[i] = ControlWord_End;
        if (word == ControlWord_End)
            return;

        if ((word & 0xff00) == 0xff00 || word >= 0x80)
            continue;

        RenderGlyph* rglyph = &glyphs_[word];
        if (rglyph->unknown_8 < 0)
            continue;
        glyphRefcounts_17b8_[word]--;
        if (glyphRefcounts_17b8_[word] == 0)
        {
            rglyph->Reset2();
            numGlyphsInUse_9b4_--;
            numGlyphsInUse_9b4_ &= 0x7f;
        }
    }
}