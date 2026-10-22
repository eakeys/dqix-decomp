#include "UI/Text.h"
#include "NitroSDK/Graphics.h"
#include "GameState/GameState.h"
#include <globaldefs.h>

extern "C"
{
    void func_02042b98(TextManager*, int, int, int, int);
    void func_02042fcc(TextBox*);
    void func_0204394c(void*);
    void func_02044c74(TextManager*, UIElement*);
    void func_02044d0c();
    void func_02044f3c(TextManager*, const char*, int);
    void func_02045824(TextManager*, int, int);
    void func_0204483c(TextManager*, const char*, char*);
    unsigned short func_020459b8(TextManager*, unsigned char, FontIndexFile::Glyph*, unsigned char);

    void func_0205e944(void*, int);

    void func_0207df50(void*);
    void func_0207df90(void*);
    void func_0207dfac(void*);

    // alternative strlen
    int func_020d2ff0(const char*);

    // populate noun from item ID?
    void func_020dcf7c(short, Noun*);

    unsigned char* func_020e1b24(void*);
    void func_020e1b2c(void*, int);
    bool func_020e1f1c(void*);
    void func_020e2110(void*);
    void func_020e23f4(void*);
    void func_020e2478(void*);
}

extern const int data_020e7a94[];

extern char data_02108760[];

void TextManager::ShowMessage(const char *msg, int graphicsMode, int width)
{
    GameState* gs = GameState::GetInstance();
    short probablyItemID = gs->unknown_7f58_;
    Noun itemNoun;
    itemNoun.Reset();
    if (probablyItemID > 0)
    {
        char singular[0x80] = {0};
        char plural[0x80] = {0};
        itemNoun.singularName_ = singular;
        itemNoun.pluralName_ = plural;
        func_020dcf7c(probablyItemID, &itemNoun);
        items_18_[0] = &itemNoun;
    }
    fontIndex_19dc_ = 1;
    if (msg == NULL)
        return;

    // what's the point of this?!
    int length = func_020d2ff0(msg);
    for (int i = 0; i < length; )
    {
        int copyLength = 0x40;
        char scratch[67] = {0};
        if (length < i + 0x40)
            copyLength = length - i;
        memcpy(scratch, msg + i, copyLength);
        i += copyLength;
    }

    for (int i = 0; i < 4; i++)
    {
        tagSTArray1_194e_[i] = -1;
        tagSTArray2_1952_[i] = 0;
    }
    tagSTArrayIndex_1956_ = 0;
    unknown_1957_ = 0;
    unknown_19d2_ = 1;
    unknown_19d1_ = 0;
    unknown_1958_ = 0;
    unknown_195b_ = 0;
    unknown_9b0_ = 0;
    unknown_195a_ = 8;

    if (pauseType_9a0_ == 3)
        func_02044f3c(this, msg, graphicsMode);
    else
    {
        unsigned short indexBufferLength = unknown_1942_;
        for (unsigned short i = 0; i < indexBufferLength; i++)
            ReleaseGlyphByIndex(indexBuffer_1882_[i]);

        // bug? this writes 0x01 and not 0xff01
        memset(indexBuffer_1882_, 0xff01, sizeof(indexBuffer_1882_));
        memset(buffer_1902_, 0, sizeof(buffer_1902_));
        unknown_19c4_ = 0;
        unknown_19c3_ = 0;
        unknown_1942_ = 0;
        speakerNameWidth_1944_ = 0;
        unknown_1946_ = 0;
        speakerDifference_19c2_ = true;
        unknown_19d3_ = 0;
        unknown_19b0_ = 0;
        unknown_19bf_ = 1;
        unknown_19cf_ = 1;
        voiceVolumeLookup_194a_[0] = voiceVolumeLookup_194a_[1] =
            voiceVolumeLookup_194a_[2] = voiceVolumeLookup_194a_[3] = 127;
        func_0205e944(data_02108760, 127);
        memset(substruct_2cc_, 0, sizeof(substruct_2cc_));
        memset(fullMessage_48_, 0, 0x960);
        memset(glyphIndexList_4c_, 0, 0x280);
        memset(currentPageMessage_50_, 0, 0x280);
        unknown_19bd_ = 1;
        numGlyphsInUse_9b4_ = 0;
        for (int i = 0; i < 0x80; i++)
        {
            glyphs_[i].Reset2();
            glyphRefcounts_17b8_[i] = 0;
        }
        skipTag_19bb_ = 1;
        mojiArg1_1860_ = 12;
        mojiArg2_1864_ = 16;
        color_187c_ = 0x7fff;
        gyouParameter_9a4_ = 3;
        func_02042b98(this, 2, 128, 252, 62);
        speakerTextBox_930_.Reset();
        cenTag_19b8_ = 0;
        showWindow_19b1_ = 1;
        centerX_990_ = 128;
        unknown_1868_ = 0;
        unknown_912_ = 0;
        unknown_994_ = -1;
        unknown_94c_ = 0;
        unknown_950_ = 0;
        unknown_954_ = 0;
        unknown_958_ = 0;
        unknown_19ac_ = 0;
        unknown_19ad_ = 0;
        pageTValue_1960_ = 0;
        func_02045824(this, 0, -1);
        maybeBoxShaking_19c0_ = false;
        unknown_195d_ = 30;
        unknown_19c1_ = 0;
        recoveryParam_19c6_ = 0;
        recoveryParam_19c7_ = 0;
        unknown_19c8_ = 0;
        unknown_19c9_ = 0;
        recoveryAmount_187e_ = 0;
        unknown_19cc_ = 0;
        unknown_19ce_ = false;
        unknown_19cd_ = 0;
        if (graphicsMode == 1)
            unknown_19cc_ = 1;
        else if (graphicsMode == 1)
            unknown_19ce_ = true;

        if (unknown_19cc_)
        {
            unknown_19cd_ = 1;
            unknown_40_ = 0;
            func_02044d0c();
        }

        if (unknown_19ce_)
        {
            unknown_44_ = 0;
            func_0204394c(unknown_1e28_);
        }

        char* tempBufferA = pageScratchBuffer_60_;
        char* tempBufferB = scratchBuffer_64_;
        SubstituteValueTags(msg, tempBufferA);
        SubstituteUnsupportedCharacters(tempBufferA, tempBufferB, fontIndex_19dc_);
        SubstituteConditionals(tempBufferB, tempBufferA, fontIndex_19dc_);
        SubstituteNamesAndArticles(tempBufferA, tempBufferB, fontIndex_19dc_);
        SubstituteCaps(tempBufferB, tempBufferA, fontIndex_19dc_);
        SubstituteControlTags(tempBufferA, tempBufferB);
        LoadFirstPage(tempBufferB, width);
        func_02042b98(this, 2, 116, 252, 74);
        unknown_1858_ = 2;
        unknown_185c_ = 126;
        mojiArg1_1860_ = 12;
        mojiArg2_1864_ = 20;
        if (unknown_38_ != NULL)
        {
            unsigned char* ptr = func_020e1b24(unknown_38_);
            if (ptr != NULL)
            {
                ptr[0x38] = 2;
                ptr[0x39] = 116;
                ptr[0x3a] = 252;
                ptr[0x3b] = 74;
                func_020e1b2c(unknown_38_, -1);
                if (func_020e1f1c(unknown_38_))
                {
                    func_020e2110(unknown_38_);
                    func_020e23f4(unknown_38_);
                    func_020e2478(unknown_38_);
                }
            }
        }
        memset(substructOrArray_95c_, 0, sizeof(substructOrArray_95c_));
        memset(substructOrArray_97c_, 0, sizeof(substructOrArray_97c_));
        unknown_994_ = -1;
        unknown_19bc_ = 0;
        pauseType_9a0_ = 5;
        unknown_19b2_ = 1;
        unknown_99c_ = 0;
        unknown_19ae_ = 0;
        unknown_19af_ = 0;
        unknown_19b5_ = 0;
        unknown_19c5_ = 0;
        unknown_19ca_ = 1;
        unknown_19b4_ = 0;
        for (int i = 0; i < 2; i++)
            windows_[i].SetEnabled(false);
        if (unknown_1e28_ != NULL)
            func_0207df50(unknown_1e28_); 
    }
    unknown_30_ = -1;
    unknown_31_ = -1;
    if (probablyItemID > 0)
        gs->unknown_7f58_ = 0;
}

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