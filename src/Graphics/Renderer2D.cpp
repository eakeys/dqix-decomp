#include "Graphics/Text.h"
#include "System/Cache.h"
#include "System/BGBases.h"

void LoadToMainBG0CharacterData(const void* data, unsigned int offset, unsigned int length);

typedef void (*LoadToBGCharProc)(const void*, unsigned int, unsigned int);

extern "C"
{
    // probably atoi
    int func_02005a94(const char*);

    // get function to load to main/SUB BG character data
    LoadToBGCharProc func_0204a5e4(int mainOrSub, int whichBG);

    void func_0204df9c(Renderer2D*, FontIndexFile::Glyph*, short x, short y, int palette, int fontIndex);

    // alternate strlen
    int func_020d2ff0(const char*);
}

// space size?
extern const unsigned char data_020e7bd6[2]; // { 2, 3 }
extern const unsigned char data_020e7bd8[2]; // { 2, 3 }
extern const char data_020e7bda[2]; // "/"

extern char data_020f0248[]; // "SIZE="
extern char data_020f024e[]; // "X="
extern char data_020f0251[]; // "Y="
extern char data_020f0254[]; // "XY="
extern char data_020f0258[]; // "W="
extern char data_020f025b[]; // "H="
extern char data_020f025e[]; // "WH="
extern char data_020f0262[]; // "LINE="
extern char data_020f0268[]; // "LINEX="
extern char data_020f026f[]; // "LINEY="
extern char data_020f0276[]; // "WHITE"
extern char data_020f027c[]; // "YELLOW"
extern char data_020f0283[]; // "PINK"
extern char data_020f0288[]; // "RED"
extern char data_020f028c[]; // "PLTT="
extern char data_020f0292[]; // "SDRC="
extern char data_020f0298[]; // "ENC="
extern char data_020f029d[]; // "N="
extern char data_020f02a0[]; // "/N"
extern char data_020f02a3[]; // "SOLID="
extern char data_020f02aa[]; // "FRAME="
extern char data_020f02b1[]; // "CURSOR="
extern char data_020f02b9[]; // "UA="
extern char data_020f02bd[]; // "UB="
extern char data_020f02c1[]; // "DA="
extern char data_020f02c5[]; // "DB="
extern char data_020f02c9[]; // "WIRE="
extern char data_020f02cf[]; // "TEN="
extern char data_020f02d4[]; // "TITLE="
extern char data_020f02db[]; // "/TITLE"
extern char data_020f02e2[]; // "SLT="
extern char data_020f02e7[]; // "XR="
extern char data_020f02eb[]; // "</XR>"
extern char data_020f02f1[]; // "/XR"
extern char data_020f02f5[]; // "%%0%dd"
extern char data_020f02fc[]; // "+%d"
extern char data_020f0300[]; // "%d"

void Renderer2D::FillRectangle(int colorIndex, short left, short top, short right, short bottom)
{
    if (right < 0 || bottom < 0)
        return;

    short pixelHeight = GetHeight();
    short pixelWidth = GetWidth();
    
    if (pixelWidth < right)
        return;
    if (pixelHeight < bottom)
        return;

    if (left == right || top == bottom)
        return;

    unsigned char highMask, lowMask;
    lowMask = colorIndex & 0xf;
    highMask = lowMask << 4;

    for (short y = top; y < bottom; y++)
    {
        short tileY = y >> 3;
        short rowInTile = y & 7;
        short rowInTileOffset = rowInTile << 2;
        
        int tileYBase = tileY * numTilesX_a8_;
        for (short x = left; x < right; x++)
        {
            short colInTile = x & 7;
            short colInTileOffset = colInTile >> 1;
            short tileX = x >> 3;
            short tileIndex = tileYBase + tileX;
            short byteIndex;
            byteIndex = tileIndex << 5;
            byteIndex += colInTileOffset;
            byteIndex += rowInTileOffset;
            
            unsigned char* pByte = &stagingData_8_[byteIndex];
            if (x & 1)
            {
                *pByte &= 0xf;
                *pByte |= highMask;
            }
            else
            {
                *pByte &= 0xf0;
                *pByte |= lowMask;
            }
        }
    }
}

// 0x0204e1c8. Not a match, awful bit stuff
void Renderer2D::BlitBitmap4Bit(const void* argsrc, short left, short top, short width, short height, unsigned char bitmaskHigh, unsigned char bitmaskLow)
{
    if (argsrc == NULL)
        return;
    const unsigned char* src = (const unsigned char*)argsrc;

    short tileCountX = numTilesX_a8_;
    short tileCountY = numTilesY_aa_;

    short loopX, loopY, x, y;
    for (loopY = 0, y = top; loopY < height; y++, loopY++)
    {
        short tileY = y >> 3;
        short rowInTileOffset = (unsigned short)(y & 7) << 2;

        int tileYBase;
        bool visibleY = (tileY >= 0 && tileY < tileCountY);
        
        tileYBase = tileY * numTilesX_a8_;

        loopX = 0;
        x = left;
        for (; loopX < width; x++, loopX++)
        {
            short tileX = x >> 3;

            bool visible = (tileX >= 0 && tileX < tileCountX);

            visible |= visibleY;

            short byteIndex = tileYBase + tileX;
            byteIndex <<= 5;
            short colInTileOffset = (short)(x & 7) >> 1;
            byteIndex += (short)(x & 7) >> 1;
            byteIndex += rowInTileOffset;

            unsigned char* pByte = &stagingData_8_[byteIndex];
            int initial = *pByte;
            if (left & 1)
            {
                if (x & 1)
                {
                    if (visible)
                    {
                        unsigned char mask = (*src & bitmaskLow);
                        if (mask != 0)
                        {
                            unsigned int shiftedMask = mask;
                            shiftedMask >>= 4;
                            initial = (unsigned char)(initial & 0xf0) | shiftedMask;
                        }
                        *pByte = initial;
                    }
                    src++;
                }
                else
                {
                    if (visible)
                    {
                        unsigned char mask = (*src & bitmaskHigh);
                        if (mask != 0)
                        {
                            unsigned int shiftedMask = mask;
                            shiftedMask <<= 4;
                            initial = (unsigned char)(initial & 0x0f) | (unsigned char)shiftedMask;
                        }
                        *pByte = initial;
                    }
                    
                }
            }
            
            else
            {
                if (x & 1)
                {
                    if (visible)
                    {
                        unsigned char mask = (*src & bitmaskHigh);
                        if (mask)
                            initial = (unsigned char)(initial & 0x0f) | mask;
                        *pByte = initial;
                    }
                    src++;
                }
                else
                {
                    if (visible)
                    {
                        unsigned char mask = (*src & bitmaskLow);
                        if (mask != 0)
                            initial = (unsigned char)(initial & 0xf0) | mask;
                        *pByte = initial;
                    }
                }
            }

        }
    }
}

// 0x0204e3e0. Not a match
int Renderer2D::PrepareText(const char *text, int unused, bool bool4, bool bool5, unsigned char fontIndex)
{
    if (text == NULL)
        return 0;

    int textX = unknown_b0_; // iVar13
    int textY = unknown_b2_; // iVar14

    int maxX = textX; // iVar12
    int maxY = textY; // iVar15

    bool hasTitle = false; // bVar4
    int title_iVar11 = 0;
    
    int lastGlyph = 0xff; // local_148
    short vSpacing = lineSpacing_b6_;
    int local_154 = 0;
    int textRightX; // local_158
    

    while (*text != 0)
    {
        char ch0 = *text;
        char ch1 = text[1];
        if ((ch0 == '\\' && ch1 == 'n') || (ch0 == '\r' && ch1 == '\n'))
        {
            if (maxX < textX)
                maxX = textX;
            
            text += 2;
            textY += lineSpacing_b6_;
            textX = unknown_b0_;
            
            lastGlyph = 0xff;
            continue;
        }

        if (ch0 == '\n')
        {
            if (maxX < textX)
                maxX = textX;
            
            text += 1;
            lastGlyph = 0xff;
            textY += lineSpacing_b6_;
            textX = unknown_b0_;
            
            continue;
        }

        int nextGlyph = GetNextFontGlyphIndex(text, fontIndex);
        if (*text == '<' && nextGlyph < 0) // i.e. a tag but not a glyph tag
        {
            const char* localTextPtr = text + 1;
            const char* tagEnd = GetTextTagEnd(const_cast<char*>(text));
            short tagValues[8];
            if (!CheckValueTag(&localTextPtr, data_020f0248, tagValues, 1))
            {
                if (CheckValueTag(&localTextPtr, data_020f024e, tagValues, 1))
                {
                    if (maxX < textX)
                        maxX = textX;
                    lastGlyph = 0xff;
                    textX = tagValues[0];
                }
                else if (CheckValueTag(&localTextPtr, data_020f0251, tagValues, 1))
                {
                    if (maxY < textY)
                        maxY = textY;
                    lastGlyph = 0xff;
                    textY = tagValues[0];
                }
                else if (CheckValueTag(&localTextPtr, data_020f0254, tagValues, 2))
                {
                    if (maxX < textX)
                        maxX = textX;
                    if (maxY < textY)
                        maxY = textY;
                    lastGlyph = 0xff;
                    textX = tagValues[0];
                    textY = tagValues[1];
                }
                else if (CheckValueTag(&localTextPtr, data_020f0258, tagValues, 1))
                {
                    lastGlyph = 0xff;
                    textX += tagValues[0];
                }
                else if (CheckValueTag(&localTextPtr, data_020f025b, tagValues, 1))
                {
                    lastGlyph = 0xff;
                    textY += tagValues[0];
                }
                else if (CheckValueTag(&localTextPtr, data_020f025e, tagValues, 2))
                {
                    lastGlyph = 0xff;
                    textX += tagValues[0];
                    textY += tagValues[1];
                }
                else if (CheckValueTag(&localTextPtr, data_020f0298, tagValues, 1))
                {
                    local_154 = tagValues[0];
                }
                else if (CheckValueTag(&localTextPtr, data_020f02d4, tagValues, 1))
                {
                    hasTitle = true;
                    title_iVar11 = tagValues[0];
                }
                else if (CheckValueTag(&localTextPtr, data_020f02db, tagValues, 1))
                {
                    if (hasTitle)
                    {
                        if (maxX < textX)
                            maxX = textX;
                        if (maxY < textY)
                            maxY = textY;
                        textY = title_iVar11 + unknown_b2_;
                        textX = unknown_b0_;
                    }
                }
                else if (CheckValueTag(&localTextPtr, data_020f02e7, tagValues, 1))
                {
                    if (maxX < textX)
                        maxX = textX;
                    textRightX = tagValues[0];
                    lastGlyph = 0xff;
                    char rightBuffer[256] = {0};
                    const char* sourceStart = localTextPtr;
                    const char* sourceEnd = strstr(sourceStart, data_020f02eb);
                    textX = textRightX;
                    if (sourceEnd != NULL)
                    {
                        int length = sourceEnd - sourceStart;
                        if (length < 256)
                        {
                            memcpy(rightBuffer, sourceStart, length);
                            textX = textRightX - MeasureTextWidth(rightBuffer, fontIndex);
                        }
                    }
                }
                else if (CheckValueTag(&localTextPtr, data_020f02f1, tagValues, 1))
                {
                    if (maxX < textX)
                        maxX = textX;
                
                    lastGlyph = 0xff;
                    textX = textRightX;
                }
            }
            text = tagEnd + 1;
            continue;
        }

        unsigned char spacing = data_020e7bd6[fontIndex] + 1;
        if (nextGlyph < 0)
        {
            text += 1;
        }
        else
        {
            FontIndexFile::Glyph* glyph = GetFontGlyphByIndex(nextGlyph, fontIndex);
            int kerning = GetFontGlyphKerning(lastGlyph, nextGlyph, fontIndex);
            spacing = glyph->width + 1 + kerning;
            text += glyph->tagLength;
            lastGlyph = nextGlyph;
        }
        textX += spacing;
    }

    if (textX < maxX)
        textX = maxX;
    if (textY < maxY)
        textY = maxY;

    if (((textX + 3) & 7) <= 3)
        textX += 4;

    int finalY = (textY + vSpacing) - (lineSpacing_b6_ - 10);

    int numXBlocks = (textX + 7) >> 3;
    int numYBlocks = (finalY + 7) >> 3;

    if (numXBlocks > 32)
        numXBlocks = 32;
    if (numYBlocks > 32)
        numYBlocks = 32;

    if (bool5)
    {
        if (numTilesX_a8_ < numXBlocks)
            numTilesX_a8_ = numXBlocks;
        if (numTilesY_aa_ < numYBlocks)
            numTilesY_aa_ = numYBlocks;
    }

    if (bool4)
    {
        finalY -= unknown_b2_;
        if (8 * numTilesY_aa_ - finalY < 7)
            numTilesY_aa_++;
        unknown_b2_ = (numTilesY_aa_ * 8 - finalY) >> 1;
    }

    if (hasTitle)
    {
        unknown_b2_ += title_iVar11;
        if (bool4)
            unknown_b2_--;
    }

    bufferSize_a4_ = numTilesX_a8_ * numTilesY_aa_ * 32;
    return local_154;
}

// 0x0204f41c. Close but not a match
void Renderer2D::RenderText(short x, short y, const char *argText, unsigned char _size,
    unsigned char _colorIndex, short *outW, short *outH, int fontIndex)
{
    if (argText == NULL)
        return;

    const char* text = argText;
    unsigned char colorIndex = _colorIndex;
    
    unsigned char defaultSpacing;
    unsigned char size;

    short initialX = x;
    short initialY = y;
    
    *outH = 0;
    *outW = 0;
    
    if (_size == 12)
        fontIndex = 1;
    
    size = _size;
    defaultSpacing = 1 + data_020e7bd8[fontIndex];

    while (*text != 0)
    {
        char ch0 = *text;
        char ch1 = text[1];
        if ((ch0 == '\\' && ch1 == 'n') || (ch0 == '\r' && ch1 == '\n'))
        {
            y += lineSpacing_b6_;
            x = initialX;
            text += 2;
            continue;
        }

        if (ch0 == '\n')
        {
            y += lineSpacing_b6_;
            x = initialX;
            text++;
            continue;
        }

        if (ch0 == '<')
        {
            text += 1;
            short tagValues[8];

            if (CheckValueTag(&text, data_020f0248, tagValues, 1))
                continue;
            if (CheckValueTag(&text, data_020f0258, tagValues, 1))
            {
                x += tagValues[0];
                continue;
            }
            if (CheckValueTag(&text, data_020f025b, tagValues, 1))
            {
                y += tagValues[0];
                continue;
            }
            if (CheckValueTag(&text, data_020f025e, tagValues, 2))
            {
                x += tagValues[0];
                y += tagValues[1];
                continue;
            }
            if (CheckValueTag(&text, data_020f024e, tagValues, 1))
            {
                x = tagValues[0];
                continue;
            }
            if (CheckValueTag(&text, data_020f0251, tagValues, 1))
            {
                y = tagValues[0];
                continue;
            }
            if (CheckValueTag(&text, data_020f0254, tagValues, 2))
            {
                x = tagValues[0];
                y = tagValues[1];
                continue;
            }
            if (CheckValueTag(&text, data_020f028c, tagValues, 1))
            {
                tagValues[0] &= 0xf;
                colorIndex = tagValues[0];
                continue;
            }
            // treat the '<' as a literal?
            text--;
        }
        
        unsigned char thisCharWidth = defaultSpacing;
        FontIndexFile::Glyph* glyph;
        glyph = GetNextFontGlyph(text, fontIndex);
        
        if (glyph != NULL)
        {
            func_0204df9c(this, glyph, x, y, colorIndex, fontIndex);
            thisCharWidth = glyph->width + 1;
            short candidateW = x + thisCharWidth;
            short candidateH = y + size;
            if (*outW < candidateW)
                *outW = candidateW;
            if (*outH < candidateH)
                *outH = candidateH;
            text += glyph->tagLength;
        }
        else
        {
            if (*text != ' ')
            {
                glyph = GetFontGlyphByIndex(0, fontIndex);
                func_0204df9c(this, glyph, x, y, colorIndex, fontIndex);
            }
            text++;
        }
        x += thisCharWidth;
    }
    
    *outW -= initialX;
    *outH -= initialY;
}

void Renderer2D::RenderNumber(short x, short y, int number, unsigned char size,
    unsigned char colorIndex, short *outW, short *outH, bool rightAlign,
    unsigned char numDigits, bool alwaysShowSign, bool fixedNumDigits)
{
    int topDigit = number;
    do
    {
        topDigit /= 10;
    } while (topDigit != 0);

    char textBuffer[64] = {0};
    if (fixedNumDigits)
    {
        char formatBuffer[8] = {0};
        sprintf(formatBuffer, data_020f02f5, numDigits);
        sprintf(textBuffer, formatBuffer, number);
    }
    else if (alwaysShowSign && number > 0)
        sprintf(textBuffer, data_020f02fc, number);
    else
        sprintf(textBuffer, data_020f0300, number);

    int fontIndex = GetFontIDBySize(size);
    if (rightAlign)
        x -= MeasureTextWidth(textBuffer, fontIndex);
    RenderText(x, y, textBuffer, size, colorIndex, outW, outH, fontIndex);
}

void Renderer2D::FillRectangle2(int colorIndex, short left, short top, short right, short bottom)
{
    if (right < 0 || bottom < 0)
        return;

    short pixelHeight = GetHeight();
    short pixelWidth = GetWidth();
    
    if (pixelWidth < right)
        return;
    if (pixelHeight < bottom)
        return;

    if (left == right || top == bottom)
        return;

    unsigned char highMask, lowMask;
    lowMask = colorIndex & 0xf;
    highMask = lowMask << 4;

    for (short y = top; y < bottom; y++)
    {
        short tileY = y >> 3;
        short rowInTile = y & 7;
        short rowInTileOffset = rowInTile << 2;
        
        int tileYBase = tileY * numTilesX_a8_;
        for (short x = left; x < right; x++)
        {
            short colInTile = x & 7;
            short colInTileOffset = colInTile >> 1;
            short tileX = x >> 3;
            short tileIndex = tileYBase + tileX;
            short byteIndex;
            byteIndex = tileIndex << 5;
            byteIndex += colInTileOffset;
            byteIndex += rowInTileOffset;
            
            unsigned char* pByte = &stagingData_8_[byteIndex];
            if (x & 1)
            {
                *pByte &= 0xf;
                *pByte |= highMask;
            }
            else
            {
                *pByte &= 0xf0;
                *pByte |= lowMask;
            }
        }
    }
}

void Renderer2D::DrawGrayCursor(short x, short y)
{
    if (!(flags_c5_ & (1 << 6)))
        return;
    
    short left = x - 6;
    short top = y - 2;

    short xcoords[4] = { left, left + 8, left, left + 8 };
    short ycoords[4] = { top, top, top + 8, top + 8 };

    unsigned char tileIndex = 16;
    for (unsigned char i = 0; i < 4; i++)
    {
        const unsigned char* tile = GetWindowBorderTile(tileIndex);
        if (tile != NULL)
            BlitBitmap4Bit(tile, xcoords[i], ycoords[i], 8, 8, 0xf0, 0x0f);
        tileIndex++;
    }
}

void Renderer2D::Unmap()
{
    DestinationData* data = destinationData_4_;
    int bgIndex = data->GetBGIndex();
    int screen =  data->GetScreen();
    LoadToBGCharProc loadProc = func_0204a5e4(screen, bgIndex);
    CleanInvalidateCacheRange(stagingData_8_, bufferSize_a4_);
    loadProc(stagingData_8_, vramOffset_a0_, bufferSize_a4_);
}

bool Renderer2D::CheckValueTag(const char **ppText, const char *candidate, short *outValues, int numValues)
{
    int candidateLength = func_020d2ff0(candidate);
    const char* searchPtr = *ppText;
    if (memcmp(searchPtr, candidate, candidateLength) == 0)
    {
        searchPtr += candidateLength;
        for (short i = 0; i < numValues; i++)
        {
            outValues[i] = func_02005a94(searchPtr);
            if (i == numValues - 1)
                break;
            while (*searchPtr != ',')
                searchPtr++;
            searchPtr++; // skip over the comma too
        }

        while (*searchPtr != '>' && *searchPtr != 0)
            searchPtr++;
        searchPtr++; // skip over the >
        *ppText = searchPtr;
        return true;
    }
    return false;
}

unsigned char* Renderer2D::GetCharacterBase() const
{
    int screen = destinationData_4_->GetScreen();
    int bgIndex = destinationData_4_->GetBGIndex();
    unsigned char* result = NULL;
    if (screen == 0)
    {
        switch (bgIndex)
        {
        case 0: result = (unsigned char*)GetMainBG0CharacterBase(); break;
        case 1: result = (unsigned char*)GetMainBG1CharacterBase(); break;
        case 2: result = (unsigned char*)GetMainBG2CharacterBase(); break;
        case 3: result = (unsigned char*)GetMainBG3CharacterBase(); break;
        }
    }
    else
    {
        switch (bgIndex)
        {
        case 0: result = (unsigned char*)GetSubBG0CharacterBase(); break;
        case 1: result = (unsigned char*)GetSubBG1CharacterBase(); break;
        case 2: result = (unsigned char*)GetSubBG2CharacterBase(); break;
        case 3: result = (unsigned char*)GetSubBG3CharacterBase(); break;
        }
    }
    return result;
}

void Renderer2D::RefreshPageNumber(bool stage)
{
    if (!maybePageNumbersEnabled_d8_0_ || (!pageNumberDirty_d8_1_ && stage))
        return;

    if (destinationData_4_ == NULL || stagingData_8_ == NULL)
        return;

    int screen =  destinationData_4_->GetScreen();
    int bgIndex = destinationData_4_->GetBGIndex();
    

    if (stage)
    {
        unsigned char* characterData = GetCharacterBase();
        if (characterData == NULL)
            return;
        if (characterData != NULL)
            memcpy(stagingData_8_, characterData + vramOffset_a0_, bufferSize_a4_);
    }
    short firstNumberRight;
    short boxHeight = numTilesY_aa_ * 8;
    short renderY = boxHeight - 13;

    short boxWidth = numTilesX_a8_ * 8;
    short centerX = boxWidth / 2;

    
    if (flags_c5_ & (1 << 4))
        renderY -= 3;

    short tempOutWidth = 0;
    short tempOutHeight = 0;

    int numChars = 2;
    // This was probably supposed to count the number of characters but assign at least
    // 2, but it doesnt (in the end, useless == 0 always)
    char useless = numPages_da_;
    while (useless != 0)
    {
        useless++;
        useless /= 10;
    }
    if (useless > 2)
        numChars = useless;

    
    int backgroundColorIndex;
    backgroundColorIndex = 1;
    if (flags_c5_ & (1 << 2))
        backgroundColorIndex = 0;

    int slashWidth = 0;
    FontIndexFile::Glyph* slashGlyph = GetNextFontGlyph(data_020e7bda, 0);
    if (slashGlyph != NULL)
        slashWidth = slashGlyph->width;

    
    
    RenderText(centerX - (slashWidth >> 1), renderY, data_020e7bda, 8, 15, &tempOutWidth, &tempOutHeight, 0);
    firstNumberRight = centerX - slashWidth; 
    FillRectangle2(backgroundColorIndex, firstNumberRight - 14, renderY, firstNumberRight, renderY + 9);
    unsigned char actualNumChars = numChars;
    RenderNumber(firstNumberRight, renderY, pageNumber_d9_ + 1, 8, 15, &tempOutWidth, &tempOutHeight, 
        true, actualNumChars, false, false);
    
    short secondNumberLeft = centerX + slashWidth;
    FillRectangle2(backgroundColorIndex, secondNumberLeft, renderY, secondNumberLeft + 14, renderY + 9);
    RenderNumber(secondNumberLeft + 1, renderY, numPages_da_, 8, 15, &tempOutWidth, &tempOutHeight,
        false, actualNumChars, false, false);

    if (stage)
    {
        LoadToBGCharProc proc = func_0204a5e4(screen, bgIndex);
        CleanInvalidateCacheRange(stagingData_8_, bufferSize_a4_);
        proc(stagingData_8_, vramOffset_a0_, bufferSize_a4_);
    }

    pageNumberDirty_d8_1_ = false;
}

void Renderer2D::ResetPageNumbers(bool enabled)
{
    maybePageNumbersEnabled_d8_0_ = enabled;
    pageNumberDirty_d8_1_ = true;
    pageNumber_d9_ = 0;
    numPages_da_ = 0;
}

void Renderer2D::SetPageNumber(unsigned char to)
{
    pageNumber_d9_ = to;
    pageNumberDirty_d8_1_ = true;
}

void Renderer2D::SetPageCount(unsigned char to)
{
    numPages_da_ = to;
    pageNumberDirty_d8_1_ = true;
}