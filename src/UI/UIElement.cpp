#include "UI/Text.h"
#include "UI/UIElement.h"
#include "System/Cache.h"
#include "System/BGBases.h"
#include <asmhacks.h>

void LoadToMainBG0CharacterData(const void* data, unsigned int offset, unsigned int length);

typedef void (*LoadToBGCharProc)(const void*, unsigned int, unsigned int);

extern "C"
{
    // probably atoi
    int func_02005a94(const char*);

    // get special character at given size
    const unsigned char* func_02042a50(int size, const char*);

    void func_02046608(void*, unsigned char, const char*, char*, int, int, unsigned char);

    // get function to load to main/SUB BG character data
    LoadToBGCharProc func_0204a5e4(int mainOrSub, int whichBG);

    void func_0204acb0(void*);
    void func_0204acd0(void*);

    void func_0204b878(UIElement::DestinationData*, UIElement::SubstructC8*,
        short, short, short, short, short, short, unsigned short);

    void func_0204df9c(UIElement*, FontIndexFile::Glyph*, short x, short y, int palette, int fontIndex);

    // alternate strlen
    int func_020d2ff0(const char*);
}

// space size?
extern const unsigned char data_020e7bd6[2]; // { 2, 3 }
extern const unsigned char data_020e7bd8[2]; // { 2, 3 }
extern const char data_020e7bda[2]; // "/"
extern const unsigned char data_020e7bdc[2]; // { 2, 3 }

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
extern char data_020f0303[]; // bytes 0x81 0xa9 \0, Shift-JIS for left arrow
extern char data_020f0306[]; // bytes 0x81 0xa8 \0, Shift-JIS for right arrow

void UIElement::GetElementBounds(short elementID, short *outX, short *outY, short *outW, short *outH) const
{
    if (elementID < 0 || elementID >= 18)
        return;
    *outX = elementXValues_[elementID];
    *outY = elementYValues_[elementID];
    *outW = elementWidths_[elementID];
    *outH = elementHeights_[elementID];
}

void UIElement::SetElementBounds(short elementID, short x, short y, short w, short h)
{
    if (elementID < 0 || elementID >= 18)
        return;
    elementXValues_[elementID] = x;
    elementYValues_[elementID] = y;
    elementWidths_[elementID] = w;
    elementHeights_[elementID] = h;
}

void UIElement::Reset()
{
    next_0_ = NULL;
    destinationData_4_ = NULL;
    pixelBuffer_8_ = NULL;
    ClearElementBounds();
    status_9c_ = 0;
    vramOffset_a0_ = 0;
    bufferSize_a4_ = 0;
    leftTile_ac_ = 0;
    topTile_ae_ = 0;
    numTilesX_a8_ = 0;
    numTilesY_aa_ = 0;
    textXOffset_b0_ = 4;
    textYOffset_b2_ = 4;
    unknown_b4_ = 10;
    lineSpacing_b6_ = 11;
    unknown_b8_ = 0;
    overrideLineSpacing_ba_ = 0;
    cursorX_bc_ = 0;
    cursorY_be_ = 0;
    unknown_c0_ = 0;
    borderPalette_c2_ = 0;
    flags_c5_ = 0;
    unknown_c4_ = 0xff;
    unknown_c6_ = 2;
    fontHeight_c7_ = 10;

    func_0204acb0(&substruct_c8_);

    maybePageNumbersEnabled_d8_0_ = false;
    unk_d8_bits = 0;
    pageNumber_d9_ = 0;
    numPages_da_ = 0;
    pageNumberDirty_d8_1_ = true;
    useBG_d8_2_ = false;

    memset(backgroundMargins_db_, 0, sizeof(backgroundMargins_db_));
}

void UIElement::FullReset()
{
    func_0204acd0(&substruct_c8_);
    Reset();
}

void UIElement::SetMargins(const char* margins)
{
    memset(backgroundMargins_db_, 0, sizeof(backgroundMargins_db_));
    if (margins != NULL)
        memcpy(backgroundMargins_db_, margins, sizeof(backgroundMargins_db_));
}

void UIElement::Allocate(SafeAllocator *allocator, void *pixelBuffer, unsigned int indexBufferSize)
{
    indexBuffer_d4_ = (unsigned short*)allocator->Allocate(indexBufferSize);
    pixelBuffer_8_ = (unsigned char*)pixelBuffer;
}

bool UIElement::IsFullyLoaded() const
{
    return status_9c_ == 3;
}

bool UIElement::IsIndexBufferUsable() const
{
    if (indexBuffer_d4_ == NULL)
        return false;
    if (status_9c_ == 0)
        return false;
    return true;
}

void UIElement::UseDefaultSettings()
{
    status_9c_ = 0;
    leftTile_ac_ = 0;
    topTile_ae_ = 0;
    numTilesX_a8_ = 0;
    numTilesY_aa_ = 0;
    textXOffset_b0_ = 4;
    textYOffset_b2_ = 4;
    unknown_b8_ = 0;
    overrideLineSpacing_ba_ = 0;
    unknown_c0_ = 0;
    flags_c5_ &= ~RendererFlags_08;
    maybePageNumbersEnabled_d8_0_ = false;
    pageNumberDirty_d8_1_ = false;
    useBG_d8_2_ = false;
    unknown_c4_ = 0xff;
    flags_c5_ = 0;
    memset(backgroundMargins_db_, 0, sizeof(backgroundMargins_db_));
}

void UIElement::Update(int numTicks)
{
    if (!(flags_c5_ & RendererFlags_01))
        return;
    switch (status_9c_)
    {
    case 0:
    case 3:
        break;
    case 1:
        unknown_c0_ = numTilesY_aa_;
        status_9c_++;
        AssignIndices();
        break;
    case 2:
        AssignIndices();
        break;
    case 4:
        status_9c_ = 3;
        break;
    }
    RefreshPageNumber(true);
}

void UIElement::Display()
{
    if (destinationData_4_ == NULL)
        return;
    if (!(flags_c5_ & RendererFlags_01) || status_9c_ == 0)
        return;
    if (flags_c5_ & RendererFlags_20)
        return;

    func_0204b878(destinationData_4_, &substruct_c8_,
        0, 0, leftTile_ac_, topTile_ae_, substruct_c8_.unknown_0, unknown_c0_, borderPalette_c2_);
}

void UIElement::FullUpdate()
{
    Update(1);
    AssignIndices();
}

void UIElement::Create(const char *markup, int vramOffset, int unused,
    bool vCentering, bool expandToFit, unsigned char fontIndex)
{
    if (destinationData_4_ == NULL || pixelBuffer_8_ == NULL || markup == NULL)
        return;

    ClearElementBounds();

    int width = numTilesX_a8_ * 8;
    if (!(flags_c5_ & RendererFlags_10)) width -= 4;
    if (expandToFit) width = 256;
    char processedMarkup[0x800] = {0};
    func_02046608(TextManager::GetInstance(), fontHeight_c7_, markup, processedMarkup, width, textXOffset_b0_, fontIndex);
    // what is going on here?! PrepareText looks like it returns the value of
    // the <ENC=...> tag, why is is being used as border parameter?!
    short prepareTextReturnValue = PrepareText(processedMarkup, unused, vCentering, expandToFit, fontIndex);
    PrepareMenu(processedMarkup, prepareTextReturnValue, fontIndex);
    vramOffset_a0_ = vramOffset;
    LoadToBGCharProc loadProc = func_0204a5e4(destinationData_4_->GetScreen(), destinationData_4_->GetBGIndex());
    CleanInvalidateCacheRange(pixelBuffer_8_, bufferSize_a4_);
    loadProc(pixelBuffer_8_, vramOffset_a0_, bufferSize_a4_);
    if (flags_c5_ & RendererFlags_01)
    {
        unknown_c0_ = 0;
        short tilesY = numTilesY_aa_;
        short tilesX = numTilesX_a8_;
        int tileBufferLength = tilesX * tilesY * 2;
        substruct_c8_.unknown_0 = tilesX;
        substruct_c8_.unknown_2 = tilesY;
        substruct_c8_.unknown_8 = tileBufferLength;
        borderPalette_c2_ = 0;
        status_9c_ = 1;
    }
    else
    {
        flags_c5_ |= RendererFlags_01;
        unknown_c0_ = 0;
        int tileBufferLength = numTilesX_a8_ * numTilesY_aa_ * 2;
        memset(indexBuffer_d4_, 0, tileBufferLength);
        short tilesY = numTilesY_aa_;
        short tilesX = numTilesX_a8_;
        substruct_c8_.unknown_0 = tilesX;
        substruct_c8_.unknown_2 = tilesY;
        substruct_c8_.unknown_8 = tileBufferLength;
        borderPalette_c2_ = 0;
        status_9c_ = 1;
    }
}

void UIElement::LoadMenuPage(const char* markup, bool expandToFit, unsigned char fontIndex)
{
    if (destinationData_4_ == NULL || pixelBuffer_8_ == NULL || markup == NULL)
        return;

    short tilesX = numTilesX_a8_;
    short tilesY = numTilesY_aa_;
    short textX = textXOffset_b0_;
    short textY = textYOffset_b2_;

    int width = numTilesX_a8_ * 8 - 4;
    if (expandToFit)
        width = 256;
    short fieldB4 = unknown_b4_;
    short lineSpacing = lineSpacing_b6_;
    int bufferSize = bufferSize_a4_;
    char processedMarkup[0x800] = {0};
    func_02046608(TextManager::GetInstance(), fontHeight_c7_, markup, processedMarkup, width, textX, fontIndex);
    short prepareTextReturnValue = PrepareText(processedMarkup, 0, false, expandToFit, fontIndex);

    // Why are we rewriting these values?
    numTilesX_a8_ = tilesX;
    numTilesY_aa_ = tilesY;
    textXOffset_b0_ = textX;
    textYOffset_b2_ = textY;
    unknown_b4_ = fieldB4;
    lineSpacing_b6_ = lineSpacing;
    bufferSize_a4_ = bufferSize;

    PrepareMenu(processedMarkup, prepareTextReturnValue, fontIndex);
    RefreshPageNumber(false);
    LoadToBGCharProc loadProc = func_0204a5e4(destinationData_4_->GetScreen(), destinationData_4_->GetBGIndex());
    CleanInvalidateCacheRange(pixelBuffer_8_, bufferSize_a4_);
    loadProc(pixelBuffer_8_, vramOffset_a0_, bufferSize_a4_);
}

void UIElement::FillRectangle(int colorIndex, short left, short top, short right, short bottom)
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
            
            unsigned char* pByte = &pixelBuffer_8_[byteIndex];
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

void UIElement::PrepareMenu(const char* markup, short border, unsigned char font)
{
    if (markup == NULL)
        return;
    const char* readCursor = markup;

    unsigned char lastGlyphIndex;
    unsigned char thisGlyphIndex;
    int sltElement;
    bool titleActive = false;
    lastGlyphIndex = 0xff;
    flags_c5_ &= ~RendererFlags_02;
    sltElement = -1;
    short bytesPerRow = (numTilesX_a8_ * 8) >> 1;
    short bytesPerLine = lineSpacing_b6_ * bytesPerRow;
    int currentElement;
    short currentX;
    short currentY = textYOffset_b2_;
    unsigned char* pixelData = pixelBuffer_8_;
    unsigned char* pixelWriteCursor = pixelData + currentY * bytesPerRow;
    short dotCount = 0;
    short dotSize = 0;
    bool hasWireBorder = false;
    short wiredObjectWidth = 0;
    short wiredPaddingL = 0, wiredPaddingT = 0, wiredPaddingR = 0, wiredPaddingB = 0;
    int cursorIndex;
    int upAIndex = -1;
    int upBIndex = -1;
    int downAIndex = -1;
    cursorIndex = -1;
    int downBIndex = -1;
    int frameElement = -1;
    short frameMarginL = 0, frameMarginT = 0, frameMarginR = 0, frameMarginB = 0;
    currentElement = -1;
    int fontHeight = fontHeight_c7_;
    
    currentX = textXOffset_b0_;
    short previousX;
    
    short lineSpacing = lineSpacing_b6_;
    if (useBG_d8_2_)
        memcpy(pixelBuffer_8_, (char*)GetSubBG2CharacterBase() + vramOffset_a0_, bufferSize_a4_);
    else
        memset(pixelBuffer_8_, 0, bufferSize_a4_);

    int fillColor;
    if (flags_c5_ & RendererFlags_04)
        fillColor = 0;
    else
    {
        if (!useBG_d8_2_)
        {
            short boxX = 0;
            short boxY = 0;
            short boxW = numTilesX_a8_ * 8;
            short boxH = numTilesY_aa_ * 8;
            boxY += backgroundMargins_db_[0];
            boxH -= backgroundMargins_db_[0] + backgroundMargins_db_[1];
            boxX += backgroundMargins_db_[2];
            boxW -= backgroundMargins_db_[2] + backgroundMargins_db_[3];
            // bug? this function should take left,top,right,bottom
            // but we treat the arguments like x,y,w,h
            FillRectangle2(0x10 | 0x01, boxX, boxY, boxW, boxH);
            RoundCorners();
        }
        fillColor = 0x10 | 0x01;
    }

    if (border > 0)
    {
        short left = 0;
        short top = 0;
        short right = numTilesX_a8_ * 8;
        short bottom = numTilesY_aa_ * 8;
        FillRectangle(3, left, top, right, bottom);
        FillRectangle(fillColor, left + border, top + border, right - border, bottom - border);
    }
    else if (!(flags_c5_ & RendererFlags_10))
    {
        DrawRoundedRectangleBorder(0, 0, numTilesX_a8_, numTilesY_aa_, backgroundMargins_db_);
    }

    int colorHigh = 0xf0; // color
    int colorLow = 0x0f; // colorIdx
    int prevColorHigh = 0xf0; // saveColor
    int prevColorLow = 0xf; // saveColorIdx

    while (*readCursor != 0)
    {
        char ch0 = readCursor[0];
        char ch1 = readCursor[1];

        if ((ch0 == '\\' && ch1 == 'n') || (ch0 == '\r' && ch1 == '\n'))
        {
            currentY += lineSpacing;
            currentX = textXOffset_b0_;
            readCursor += 2;
            pixelWriteCursor += bytesPerLine;
            continue;
        }

        if (ch0 == '\n')
        {
            currentY += lineSpacing;
            currentX = textXOffset_b0_;
            readCursor += 1;
            pixelWriteCursor += bytesPerLine;
            continue;
        }

        if (ch0 == '<')
        {
            bool tagHandled = true; // handled, might be int and declared earlier
            const char* tagCursor = readCursor + 1; // p
            short tagValues[8];
            if (CheckValueTag(&tagCursor, data_020f0248, tagValues, 1))
            {
                switch (tagValues[0])
                {
                case 8:
                    fontHeight = 8;
                    lineSpacing = 9;
                    break;
                case 12:
                    fontHeight = 12;
                    lineSpacing = 13;
                    break;
                default:
                    fontHeight = 10;
                    lineSpacing = lineSpacing_b6_;
                    break;
                }
                if (overrideLineSpacing_ba_ != 0)
                    lineSpacing = overrideLineSpacing_ba_;
            }
            else if (CheckValueTag(&tagCursor, data_020f024e, tagValues, 1))
            {
                currentX = tagValues[0];
                lastGlyphIndex = 0xff;
            }
            else if (CheckValueTag(&tagCursor, data_020f0251, tagValues, 1))
            {
                currentY = tagValues[0];
                pixelWriteCursor = pixelData + bytesPerRow * currentY;
                lastGlyphIndex = 0xff;
            }
            else if (CheckValueTag(&tagCursor, data_020f0254, tagValues, 2))
            {
                currentY = tagValues[1];
                currentX = tagValues[0];
                pixelWriteCursor = pixelData + bytesPerRow * currentY;
                lastGlyphIndex = 0xff;
            }
            else if (CheckValueTag(&tagCursor, data_020f0258, tagValues, 1))
            {
                currentX += tagValues[0];
                lastGlyphIndex = 0xff;
            }
            else if (CheckValueTag(&tagCursor, data_020f025b, tagValues, 1))
            {
                pixelWriteCursor += bytesPerRow * tagValues[0];
                currentY += tagValues[0];
                lastGlyphIndex = 0xff;
            }
            else if (CheckValueTag(&tagCursor, data_020f025e, tagValues, 2))
            {
                currentX += tagValues[0];
                currentY += tagValues[1];
                pixelWriteCursor += bytesPerRow * tagValues[1];
                lastGlyphIndex = 0xff;
            }
            else if (CheckValueTag(&tagCursor, data_020f0262, tagValues, 1))
            {
                DrawDividingLine(tagValues[0]);
            }
            else if (CheckValueTag(&tagCursor, data_020f0268, tagValues, 4))
            {
                FillRectangle((unsigned char)tagValues[0], tagValues[1], tagValues[3], tagValues[2], tagValues[3] + 1);
            }
            else if (CheckValueTag(&tagCursor, data_020f026f, tagValues, 4))
            {
                FillRectangle((unsigned char)tagValues[0], tagValues[1], tagValues[2], tagValues[1] + 1, tagValues[3]);
            }
            else if (CheckValueTag(&tagCursor, data_020f0276, tagValues, 0))
            {
                colorHigh = 0xf0;
                colorLow = 0x0f;
            }
            else if (CheckValueTag(&tagCursor, data_020f027c, tagValues, 0))
            {
                colorHigh = 0xd0;
                colorLow = 0xd;
            }
            else if (CheckValueTag(&tagCursor, data_020f0283, tagValues, 0))
            {
                colorHigh = 0xb0;
                colorLow = 0xb;
            }
            else if (CheckValueTag(&tagCursor, data_020f0288, tagValues, 0))
            {
                colorHigh = 0x90;
                colorLow = 0x9;
            }
            else if (CheckValueTag(&tagCursor, data_020f028c, tagValues, 1))
            {
                tagValues[0] &= 0xf;
                colorHigh = (unsigned char)(tagValues[0] << 4);
                colorLow = (unsigned char)tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f0292, tagValues, 5))
            {
                FillRectangle((unsigned char)tagValues[0], tagValues[1], tagValues[2], tagValues[1] + tagValues[3], tagValues[2] + tagValues[4]);
            }
            else if (CheckValueTag(&tagCursor, data_020f0298, tagValues, 1))
            {
            }
            else if (CheckValueTag(&tagCursor, data_020f029d, tagValues, 1))
            {
                currentElement = tagValues[0];
                if (currentElement >= 0 && currentElement < 18)
                {
                    elementXValues_[currentElement] = currentX;
                    elementYValues_[currentElement] = currentY;
                }
                if (currentElement >= 0 && currentElement == sltElement)
                {
                    prevColorHigh = colorHigh;
                    prevColorLow = colorLow;
                    colorHigh = 0xe0; // light gray
                    colorLow = 0xe;
                }
                else if (currentElement >= 0 && currentElement == frameElement)
                {
                    prevColorHigh = colorHigh;
                    prevColorLow = colorLow;
                    colorHigh = 0x50; // green
                    colorLow = 0x5;
                }
            }
            else if (CheckValueTag(&tagCursor, data_020f02a0, tagValues, 0))
            {
                if (currentElement >= 0)
                {
                    if (currentElement == sltElement || currentElement == frameElement)
                    {
                        colorHigh = prevColorHigh;
                        colorLow = prevColorLow;
                    }
                }
                if (currentElement >= 0 && currentElement < 18)
                {
                    elementWidths_[currentElement] = currentX - elementXValues_[currentElement];
                    elementHeights_[currentElement] = fontHeight + (currentY - elementYValues_[currentElement]) + 1;
                    if (elementWidths_[currentElement] < 4)
                        elementWidths_[currentElement] = 4;
                }
                if (frameElement >= 0 && currentElement == frameElement)
                {
                    if (!(flags_c5_ & RendererFlags_10) && (upAIndex >= 0 || upBIndex >= 0 || downAIndex >= 0 || downBIndex >= 0))
                    {
                        elementXValues_[currentElement] -= frameMarginL + unknown_c6_;
                        elementYValues_[currentElement] -= frameMarginT;
                        elementWidths_[currentElement] += frameMarginL + frameMarginR;
                        elementHeights_[currentElement] += frameMarginT + frameMarginB;
                    }
                    flags_c5_ |= RendererFlags_02;
                }
                if (!(flags_c5_ & RendererFlags_10) && hasWireBorder)
                {
                    elementWidths_[currentElement] = wiredObjectWidth;
                    short eltX = elementXValues_[currentElement];
                    short eltW = elementWidths_[currentElement];
                    short eltY = elementYValues_[currentElement];
                    short eltH = elementHeights_[currentElement];
                    short eltLeft = eltX - wiredPaddingL;
                    short eltRight = wiredPaddingR + (eltX + eltW);
                    short eltTop = eltY - wiredPaddingT;
                    short eltBottom = wiredPaddingB + (eltY + eltH);
                    FillRectangle(3, eltLeft + 1, eltTop, eltRight - 1, eltTop + 1);
                    FillRectangle(3, eltLeft + 1, eltBottom - 1, eltRight - 1, eltBottom);
                    FillRectangle(3, eltLeft, eltTop + 1, eltLeft + 1, eltBottom - 1);
                    FillRectangle(3, eltRight - 1, eltTop + 1, eltRight, eltBottom - 1);
                }
                currentElement = -1;
            }
            else if (CheckValueTag(&tagCursor, data_020f02a3, tagValues, 2))
            {
            }
            else if (CheckValueTag(&tagCursor, data_020f02aa, tagValues, 5))
            {
                frameElement = tagValues[0];
                frameMarginL = tagValues[1];
                frameMarginT = tagValues[2];
                frameMarginR = tagValues[3];
                frameMarginB = tagValues[4];
            }
            else if (CheckValueTag(&tagCursor, data_020f02b1, tagValues, 1))
            {
                cursorIndex = tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f02b9, tagValues, 3))
            {
                upAIndex = (char)tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f02bd, tagValues, 3))
            {
                upBIndex = (char)tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f02c1, tagValues, 3))
            {
                downAIndex = (char)tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f02c5, tagValues, 3))
            {
                downBIndex = (char)tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f02c9, tagValues, 5))
            {
                hasWireBorder = true;
                wiredObjectWidth = tagValues[0];
                wiredPaddingL = tagValues[1];
                wiredPaddingT = tagValues[2];
                wiredPaddingR = tagValues[3];
                wiredPaddingB = tagValues[4];
                if (wiredObjectWidth == 0)
                    hasWireBorder = false;
            }
            else if (CheckValueTag(&tagCursor, data_020f02cf, tagValues, 2))
            {
                dotCount = tagValues[0];
                dotSize = tagValues[1];
            }
            else if (CheckValueTag(&tagCursor, data_020f02d4, tagValues, 1))
            {
                titleActive = true;
                DrawDividingLine(tagValues[0]);
                pixelWriteCursor = pixelData + textYOffset_b2_ * bytesPerRow;
                currentY = (tagValues[0] - 10) >> 1;
            }
            else if (CheckValueTag(&tagCursor, data_020f02db, tagValues, 0))
            {
                if (titleActive)
                {
                    currentY = textYOffset_b2_;
                    currentX = textXOffset_b0_;
                    pixelWriteCursor = pixelData + currentY * bytesPerRow;
                    titleActive = false;
                }
            }
            else if (CheckValueTag(&tagCursor, data_020f02e2, tagValues, 1))
            {
                sltElement = tagValues[0];
            }
            else if (CheckValueTag(&tagCursor, data_020f02e7, tagValues, 1))
            {
                currentX = tagValues[0];
                lastGlyphIndex = 0xff;
                previousX = currentX;
                char rightText[256] = {0};
                const char* textStart = tagCursor;
                const char* textEnd = strstr(textStart, data_020f02eb);
                if (textEnd != NULL)
                {
                    int length = textEnd - textStart;
                    if (length < 256)
                    {
                        memcpy(rightText, textStart, length);
                        currentX -= MeasureTextWidth(rightText, font);
                    }
                }
            }
            else if (CheckValueTag(&tagCursor, data_020f02f1, tagValues, 0))
            {
                currentX = previousX;
                lastGlyphIndex = 0xff;
            }
            else
                tagHandled = false;

            if (tagHandled)
            {
                readCursor = tagCursor;
                continue;
            }
        }

        unsigned char xAdvance = data_020e7bdc[font] + 1;
        int nextGlyph = GetNextFontGlyphIndex(readCursor, font);
        if (nextGlyph >= 0)
        {
            thisGlyphIndex = nextGlyph;
            FontIndexFile::Glyph* glyph = GetFontGlyphByIndex(nextGlyph, font);
            currentX += GetFontGlyphKerning(lastGlyphIndex, thisGlyphIndex, font);
            func_0204df9c(this, glyph, currentX, currentY, colorLow, font);
            lastGlyphIndex = thisGlyphIndex;
            xAdvance = glyph->width + 1;
            readCursor += glyph->tagLength;
        }
        else
        {
            if (ch0 != ' ')
                func_0204df9c(this, GetFontGlyphByIndex(0, font), currentX, currentY, colorLow, font);
            readCursor += 1;
        }
        currentX += xAdvance;
    }
    
    if (flags_c5_ & RendererFlags_08)
        DrawPageArrows();

    if (!(flags_c5_ & RendererFlags_10))
    {
        if (frameElement >= 0)
            flags_c5_ |= RendererFlags_02;
        if (cursorIndex >= 0)
        {
            short cursorX = elementXValues_[cursorIndex] - 8;
            short cursorY = elementYValues_[cursorIndex] - 1;
            cursorX_bc_ = cursorX;
            cursorY_be_ = cursorY;
            BlitGrayCursor(cursorX, cursorY);
        }
    }
    if (dotCount > 0 && dotSize > 0)
    {
        short bottom = currentY + fontHeight;
        short drawX = (short)(numTilesX_a8_ * 8) - (dotSize * 2 + 2);
        short drawY = bottom - dotSize;
        for (short i = 0; i < dotCount; i++)
        {
            FillRectangle(15, drawX, drawY, drawX + dotSize, drawY + dotSize);
            drawX -= dotSize * 2;
        }
    }

    // nonsense usage of pixelWriteCursor to prevent it being optimized out
    if (ALWAYS_FALSE())
        *pixelWriteCursor = 0;
}

void UIElement::BlitBitmap1Bit(const void* src, short left, short top,
    short width, short height, unsigned char bitmaskHigh, unsigned char bitmaskLow)
{
    const unsigned char* srcBits = (const unsigned char*)src;
    if (srcBits == NULL)
        return;

    short tileIndexX, tileIndexY;
    unsigned char bitIndex;
    short numTilesX = numTilesX_a8_;
    short numTilesY = numTilesY_aa_;
    bitIndex = 0;
    for (short srcY = 0; srcY < height; srcY++)
    {
        short destY = top + srcY;
        short tileInnerY = (destY & 7) << 2;
        tileIndexY = destY >> 3;
        bool tileVisibleY = tileIndexY >= 0 && tileIndexY < numTilesY;
        tileIndexY *= numTilesX_a8_;
        for (short srcX = 0; srcX < width; srcX++)
        {
            if (tileVisibleY && (*srcBits & (1 << bitIndex)))
            {
                int destX = (short)(left + srcX);
                int dx = destX & 7;
                tileIndexX = destX >> 3;
                bool tileVisibleX = tileIndexX >= 0 && tileIndexX < numTilesX;
                bool tileVisible = tileVisibleX | tileVisibleY;
                if (tileVisible)
                {
                    short writeOffset = tileInnerY + (tileIndexX + tileIndexY) * 32 + (dx >> 1);
                    unsigned char* writeBuffer = pixelBuffer_8_;
                    if (destX & 1)
                    {
                        writeBuffer[writeOffset] &= 0x0f;
                        writeBuffer[writeOffset] |= bitmaskHigh;
                    }
                    else
                    {
                        writeBuffer[writeOffset] &= 0xf0;
                        writeBuffer[writeOffset] |= bitmaskLow;
                    }
                }
            }
            bitIndex++;
            if (bitIndex >= 8)
            {
                bitIndex = 0;
                srcBits++;
            }
        }
    }
}

void UIElement::BlitBitmap4Bit(const void* argsrc, short left, short top, 
    short width, short height, unsigned char bitmaskHigh, unsigned char bitmaskLow)
{
    const unsigned char* src = (const unsigned char*)argsrc;
    int destXbutInt;
    unsigned char byteValue;
    short destX;
    short tileIndexY;
    if (src == NULL)
        return;
    short numTilesX = numTilesX_a8_, numTilesY = numTilesY_aa_;
    short destY = top;
    for (short srcY = 0; srcY < height; destY++, srcY++)
    {
        short tileInnerY = destY & 7;
        short tileInnerYOffset = tileInnerY << 2;
        tileIndexY = destY >> 3;
        bool tileVisibleY = tileIndexY >= 0 && tileIndexY < numTilesY;
        tileIndexY *= numTilesX_a8_;
        destX = left;
        for (short srcX = 0; srcX < width; destX++, srcX++)
        {
            destXbutInt = destX;
            short tileIndexX = destXbutInt >> 3;
            bool tileVisibleX = tileIndexX >= 0 && tileIndexX < numTilesX;
            short tileIndex = tileIndexY + tileIndexX;
            bool tileVisible = tileVisibleX | tileVisibleY;
            short writeOffset = tileIndex * 32;
            short tileInnerX = destXbutInt & 7;
            writeOffset += tileInnerX >> 1;
            writeOffset += tileInnerYOffset;
            byteValue = pixelBuffer_8_[writeOffset];
            if (left & 1)
            {
                if (srcX & 1)
                {
                    if (tileVisible)
                    {
                        unsigned char srcNibble = *src & bitmaskHigh;
                        if (srcNibble != 0)
                        {
                            srcNibble >>= 4;
                            byteValue &= 0xf0;
                            byteValue |= srcNibble;
                        }
                        pixelBuffer_8_[writeOffset] = byteValue;
                    }
                    src++;
                }
                else
                {
                    if (tileVisible)
                    {
                        unsigned char srcNibble = *src & bitmaskLow;
                        if (srcNibble != 0)
                        {
                            srcNibble <<= 4;
                            byteValue &= 0x0f;
                            byteValue |= srcNibble;
                        }
                        pixelBuffer_8_[writeOffset] = byteValue;
                    }
                }
            } 
            else
            {
                if (srcX & 1)
                {
                    if (tileVisible)
                    {
                        unsigned char srcNibble = *src & bitmaskHigh;
                        if (srcNibble)
                        {
                            byteValue &= 0x0f;
                            byteValue |= srcNibble;
                        }
                        pixelBuffer_8_[writeOffset] = byteValue;
                    }
                    src++;
                }
                else
                {
                    if (tileVisible)
                    {
                        unsigned char srcNibble = *src & bitmaskLow;
                        if (srcNibble)
                        {
                            byteValue &= 0xf0;
                            byteValue |= srcNibble;
                        }
                        pixelBuffer_8_[writeOffset] = byteValue;
                    }
                }
            }
        }
    }
}

short UIElement::PrepareText(const char *argtext, int unused, bool vCentered,
    bool expandToFit, unsigned char fontIndex)
{
    int titleY;
    int textX, textY, maxX, maxY;
    const char* text = argtext;
    if (text == NULL)
        return 0;

    maxX = textXOffset_b0_;
    maxY = textYOffset_b2_;
    unsigned char lastGlyph;
    int nextGlyphU8;
    bool hasTitle = false;
    
    titleY = 0;
    short encValue = 0;
    int savedTextX;
    short vSpacing = lineSpacing_b6_;
    textX = textXOffset_b0_;
    textY = textYOffset_b2_;
    
    lastGlyph = 0xff;

    while (*text != 0)
    {
        char ch0 = *text;
        char ch1 = text[1];
        if ((ch0 == '\\' && ch1 == 'n') || (ch0 == '\r' && ch1 == '\n'))
        {
            if (maxX < textX)
                maxX = textX;
            
            textX = textXOffset_b0_;
            textY += lineSpacing_b6_;
            text += 2;            
            lastGlyph = 0xff;
            continue;
        }

        if (ch0 == '\n')
        {
            if (maxX < textX)
                maxX = textX;
            
            textX = textXOffset_b0_;
            textY += lineSpacing_b6_;
            text += 1;
            lastGlyph = 0xff;
            continue;
        }

        int nextGlyph = GetNextFontGlyphIndex(text, fontIndex);
        if (*text == '<' && nextGlyph < 0) // i.e. a tag but not a glyph tag
        {
            const char* localTextPtr = text + 1;
            const char* tagEnd = GetTextTagEnd(const_cast<char*>(text));
            short tagValues[8];
            if (CheckValueTag(&localTextPtr, data_020f0248, tagValues, 1))
            {
            }
            else if (CheckValueTag(&localTextPtr, data_020f024e, tagValues, 1))
            {
                if (maxX < textX)
                    maxX = textX;
                textX = tagValues[0];
                lastGlyph = 0xff;
            }
            else if (CheckValueTag(&localTextPtr, data_020f0251, tagValues, 1))
            {
                if (maxY < textY)
                    maxY = textY;
                textY = tagValues[0];
                lastGlyph = 0xff;
            }
            else if (CheckValueTag(&localTextPtr, data_020f0254, tagValues, 2))
            {
                if (maxX < textX)
                    maxX = textX;
                if (maxY < textY)
                    maxY = textY;
                textX = tagValues[0];
                textY = tagValues[1];
                lastGlyph = 0xff;
            }
            else if (CheckValueTag(&localTextPtr, data_020f0258, tagValues, 1))
            {
                textX += tagValues[0];
                lastGlyph = 0xff;
            }
            else if (CheckValueTag(&localTextPtr, data_020f025b, tagValues, 1))
            {
                textY += tagValues[0];
                lastGlyph = 0xff;
            }
            else if (CheckValueTag(&localTextPtr, data_020f025e, tagValues, 2))
            {
                textX += tagValues[0];
                textY += tagValues[1];
                lastGlyph = 0xff;
            }
            else if (CheckValueTag(&localTextPtr, data_020f0298, tagValues, 1))
            {
                encValue = tagValues[0];
            }
            else if (CheckValueTag(&localTextPtr, data_020f02d4, tagValues, 1))
            {
                hasTitle = true;
                titleY = tagValues[0];
            }
            else if (CheckValueTag(&localTextPtr, data_020f02db, tagValues, 0))
            {
                if (hasTitle)
                {
                    if (maxX < textX)
                        maxX = textX;
                    if (maxY < textY)
                        maxY = textY;
                    textX = textXOffset_b0_;
                    textY = titleY + textYOffset_b2_;
                }
            }
            else if (CheckValueTag(&localTextPtr, data_020f02e7, tagValues, 1))
            {
                if (maxX < textX)
                    maxX = textX;
                textX = tagValues[0];
                lastGlyph = 0xff;
                savedTextX = textX;
                char rightBuffer[256] = {0};
                const char* sourceStart = localTextPtr;
                const char* sourceEnd = strstr(sourceStart, data_020f02eb);
                if (sourceEnd != NULL)
                {
                    int length = sourceEnd - sourceStart;
                    if (length < 256)
                    {
                        memcpy(rightBuffer, sourceStart, length);
                        textX -= MeasureTextWidth(rightBuffer, fontIndex);
                    }
                }
            }
            else if (CheckValueTag(&localTextPtr, data_020f02f1, tagValues, 0))
            {
                if (maxX < textX)
                    maxX = textX;
            
                textX = savedTextX;
                lastGlyph = 0xff;
            }
            text = tagEnd + 1;
            continue;
        }

        unsigned char spacing = data_020e7bd6[fontIndex] + 1;
        if (nextGlyph >= 0)
        {
            nextGlyphU8 = nextGlyph & 0xff;
            FontIndexFile::Glyph* glyph = GetFontGlyphByIndex(nextGlyph, fontIndex);
            int kerning = GetFontGlyphKerning(lastGlyph, nextGlyphU8, fontIndex);
            lastGlyph = nextGlyphU8;
            spacing = glyph->width + 1 + kerning;
            text += glyph->tagLength;
        }
        else
        {
            text += 1;
        }
        textX += spacing;
    }

    if (textX < maxX)
        textX = maxX;
    if (textY < maxY)
        textY = maxY;
    textY = textY + vSpacing - (lineSpacing_b6_ - 10);
    if (((textX + 3) & 7) <= 3)
        textX += 4;

    int neededTilesX = ((textX + 7) & ~7) >> 3;
    int neededTilesY = ((textY + 7) & ~7) >> 3;

    if (neededTilesX > 32)
        neededTilesX = 32;
    if (neededTilesY > 32)
        neededTilesY = 32;

    if (expandToFit)
    {
        if (numTilesX_a8_ < neededTilesX)
            numTilesX_a8_ = neededTilesX;
        if (numTilesY_aa_ < neededTilesY)
            numTilesY_aa_ = neededTilesY;
    }

    if (vCentered)
    {
        textY -= textYOffset_b2_;
        if (numTilesY_aa_ * 8 - textY < 7)
            numTilesY_aa_ = numTilesY_aa_ + 1; // needed for byte match :skull:
        textYOffset_b2_ = (numTilesY_aa_ * 8 - textY) >> 1;
    }

    if (hasTitle)
    {
        textYOffset_b2_ += titleY;
        if (vCentered)
            textYOffset_b2_--;
    }

    bufferSize_a4_ = numTilesX_a8_ * numTilesY_aa_ * 32;
    return encValue;
}

void UIElement::AssignIndices()
{
    if (!(flags_c5_ & RendererFlags_01) || indexBuffer_d4_ == NULL)
        return;

    unsigned short index = vramOffset_a0_ / 32;
    unsigned short* buffer = indexBuffer_d4_;
    for (short y = 0; y < numTilesY_aa_; y++)
    {
        for (short x = 0; x < numTilesX_a8_; x++)
        {
            buffer[x] = index;
            index++;
        }
        buffer += numTilesX_a8_;
    }
    status_9c_++;
}

bool TestPointInBox(short x, short y, short left, short top, short right, short bottom)
{
    bool inside = false;
    if (left <= x && x < right && top <= y && y < bottom)
        inside = true;
    return inside;
}

void UIElement::RoundCorners()
{
    if (flags_c5_ & RendererFlags_10)
        return;

    if (TestCornerOverlapAlignment(1) == 0)
    {
        FillRectangle2(0, 0, 0, 8, 8);
        const unsigned char* tile = GetWindowBorderTile(0x15);
        if (tile != NULL)
            BlitBitmap4Bit(tile, 0, 0, 8, 8, 0xf0, 0xf);
    }
    if (TestCornerOverlapAlignment(2) == 0)
    {
        FillRectangle2(0, (numTilesX_a8_ - 1) * 8, 0, numTilesX_a8_ * 8, 8);
        const unsigned char* tile = GetWindowBorderTile(0x16);
        if (tile != NULL)
            BlitBitmap4Bit(tile, (numTilesX_a8_ - 1) * 8, 0, 8, 8, 0xf0, 0xf);
    }
    if (TestCornerOverlapAlignment(4) == 0)
    {
        FillRectangle2(0, 0, (numTilesY_aa_ - 1) * 8, 8, numTilesY_aa_ * 8);
        const unsigned char* tile = GetWindowBorderTile(0x18);
        if (tile != NULL)
            BlitBitmap4Bit(tile, 0, (numTilesY_aa_ - 1) * 8, 8, 8, 0xf0, 0xf);
    }
    if (TestCornerOverlapAlignment(8) == 0)
    {
        FillRectangle2(0, (numTilesX_a8_ - 1) * 8, (numTilesY_aa_ - 1) * 8, numTilesX_a8_ * 8, numTilesY_aa_ * 8);
        const unsigned char* tile = GetWindowBorderTile(0x17);
        if (tile != NULL)
            BlitBitmap4Bit(tile, (numTilesX_a8_ - 1) * 8, (numTilesY_aa_ - 1) * 8, 8, 8, 0xf0, 0xf);
    }
}

static inline short TileToPixel(short tile) { return tile * 8; }

void UIElement::DrawRoundedRectangleBorder(short, short, short w, short h, const char* margins)
{
    if (w < 0 || h < 0)
        return;
    
    short edgeInner;
    int top = 0, bottom = 0, left = 0, right = 0;
    if (margins != NULL)
    {
        top = margins[0];
        bottom = margins[1];
        left = margins[2];
        right = margins[3];
    }

    // top left corner
    int alignmentType = TestCornerOverlapAlignment(1);
    int cornerTileIndex = 0;
    if (alignmentType == 1) // top-left corner aligned to left edge
        cornerTileIndex = 8;
    else if (alignmentType == 2) // top-left corner aligned to top edge
        cornerTileIndex = 12;
    const unsigned char* cornerTile = GetWindowBorderTile(cornerTileIndex);
    if (cornerTile != NULL)
        BlitBitmap4Bit(cornerTile, left, top, 8, 8, 0xf0, 0x0f);
    
    // top edge
    const unsigned char* topEdgeTile = GetWindowBorderTile(1);
    if (topEdgeTile != NULL)
    {
        for (short i = 1; i < numTilesX_a8_ - 1; i++)
            BlitBitmap4Bit(topEdgeTile, TileToPixel(i), top, 8, 8, 0xf0, 0x0f);
    }

    // top right corner
    short numTilesX = numTilesX_a8_;
    alignmentType = TestCornerOverlapAlignment(2);
    cornerTileIndex = 2;
    if (alignmentType == 3)
        cornerTileIndex = 9;
    else if (alignmentType == 2)
        cornerTileIndex = 13;
    cornerTile = GetWindowBorderTile(cornerTileIndex);
    if (cornerTile != NULL)
        BlitBitmap4Bit(cornerTile, TileToPixel(numTilesX - 1) - right, top, 8, 8, 0xf0, 0x0f);
    
    // left edge
    cornerTile = GetWindowBorderTile(3);
    if (cornerTile != NULL)
    {
        for (short i = 1; i < numTilesY_aa_ - 1; i++)
            BlitBitmap4Bit(cornerTile, left, TileToPixel(i), 8, 8, 0xf0, 0x0f);
    }

    // right edge
    cornerTile = GetWindowBorderTile(4);
    edgeInner = TileToPixel(numTilesX_a8_ - 1);
    if (cornerTile != NULL)
    {
        for (short i = 1; i < numTilesY_aa_ - 1; i++)
            BlitBitmap4Bit(cornerTile, edgeInner - right, TileToPixel(i), 8, 8, 0xf0, 0x0f);
    }

    // bottom left corner
    short numTilesY = numTilesY_aa_;
    alignmentType = TestCornerOverlapAlignment(4);
    cornerTileIndex = 5;
    if (alignmentType == 1) // bottom left corner aligned to left edge
        cornerTileIndex = 10;
    else if (alignmentType == 4) // bottom left corner aligned to bottom edge
        cornerTileIndex = 14;
    cornerTile = GetWindowBorderTile(cornerTileIndex);
    if (cornerTile != NULL)
        BlitBitmap4Bit(cornerTile, left, TileToPixel(numTilesY - 1) - bottom, 8, 8, 0xf0, 0x0f);

    // bottom edge
    const unsigned char* bottomEdgeTile = GetWindowBorderTile(6);
    edgeInner = TileToPixel(numTilesY_aa_ - 1);
    if (bottomEdgeTile != NULL)
    {
        for (short i = 1; i < numTilesX_a8_ - 1; i++)
            BlitBitmap4Bit(bottomEdgeTile, i * 8, edgeInner - bottom, 8, 8, 0xf0, 0x0f);
    }

    // bottom right corner
    numTilesX = numTilesX_a8_;
    numTilesY = numTilesY_aa_;
    alignmentType = TestCornerOverlapAlignment(8);
    cornerTileIndex = 7;
    if (alignmentType == 3) // bottom right corner aligned to right edge
        cornerTileIndex = 11;
    else if (alignmentType == 4) // bottom right corner aligned to bottom edge
        cornerTileIndex = 15;
    cornerTile = GetWindowBorderTile(cornerTileIndex);
    if (cornerTile != NULL)
        BlitBitmap4Bit(cornerTile, TileToPixel(numTilesX - 1) - right, TileToPixel(numTilesY - 1) - bottom, 8, 8, 0xf0, 0x0f);
}

void UIElement::ClearElementBounds()
{
    memset(elementXValues_, 0, sizeof(elementXValues_));
    memset(elementYValues_, 0, sizeof(elementYValues_));
    memset(elementWidths_, 0, sizeof(elementWidths_));
    memset(elementHeights_, 0, sizeof(elementHeights_));
}

void UIElement::GetElementBoundArrays(short** outXArray, short** outYArray, short** outWArray, short** outHArray)
{
    *outXArray = elementXValues_;
    *outYArray = elementYValues_;
    *outWArray = elementWidths_;
    *outHArray = elementHeights_;
}

void UIElement::SetVRAMOffset(int offset, unsigned char argc4)
{
    if (flags_c5_ & RendererFlags_01)
        return;
    vramOffset_a0_ = offset;
    unknown_c4_ = argc4;
}

void UIElement::DrawRoundedRectangle(short x, short y, short w, short h,
    unsigned char borderColor, bool noBackground, bool noBorder, const char *margins)
{
    leftTile_ac_ = x;
    topTile_ae_ = y;
    borderPalette_c2_ = borderColor;

    if (noBackground)
        flags_c5_ |= RendererFlags_04;
    else
        flags_c5_ &= ~RendererFlags_04;

    if (noBorder)
        flags_c5_ |= RendererFlags_10;
    else
        flags_c5_ &= ~RendererFlags_10;

    int indexBufferLength = w * h * 2;
    if (flags_c5_ & RendererFlags_01)
    {
        if (numTilesX_a8_ < w)
            w = numTilesX_a8_;
        if (numTilesY_aa_ < h)
            h = numTilesY_aa_;
        substruct_c8_.unknown_0 = w;
        substruct_c8_.unknown_2 = h;
        substruct_c8_.unknown_8 = indexBufferLength;
        memset(pixelBuffer_8_, 0, bufferSize_a4_);
        if (!noBackground)
        {
            short left = 0;
            short top = 0;
            short right = w * 8;
            short bottom = h * 8;
            if (margins != NULL)
            {
                top += margins[0];
                bottom -= margins[0] + margins[1];
                left += margins[2];
                right -= margins[2] + margins[3];
            }
            FillRectangle2(0x10 | 0x01, left, top, right, bottom);
        }
    }
    else
    {
        flags_c5_ |= RendererFlags_01;
        numTilesX_a8_ = w;
        numTilesY_aa_ = h;
        bufferSize_a4_ = w * h * 32;
        memset(pixelBuffer_8_, 0, bufferSize_a4_);
        if (!noBackground)
        {
            short left = 0;
            short top = 0;
            short right = w * 8;
            short bottom = h * 8;
            if (margins != NULL)
            {
                top += margins[0];
                bottom -= margins[0] + margins[1];
                left += margins[2];
                right -= margins[2] + margins[3];
            }
            FillRectangle2(0x10 | 0x01, left, top, right, bottom);
        }
        memset(indexBuffer_d4_, 0, indexBufferLength);
        short numTilesX = numTilesX_a8_;
        short numTilesY = numTilesY_aa_;
        substruct_c8_.unknown_0 = numTilesX;
        substruct_c8_.unknown_2 = numTilesY;
        substruct_c8_.unknown_8 = indexBufferLength;
        status_9c_ = 1;
    }

    if (!noBorder)
    {
        if (!noBackground)
            RoundCorners();
        DrawRoundedRectangleBorder(0, 0, w, h, margins);
    }
    else
        flags_c5_ |= RendererFlags_80;
}

void UIElement::DrawDividingLine(short y) {
    if (y < 0)
        return;
    short width = numTilesX_a8_ * 8;
    short height = numTilesY_aa_ * 8;
    if (y >= height)
        return;
    FillRectangle(15, 1, y, width - 1, y + 1);
}

void UIElement::RenderText(short x, short y, const char *argText, unsigned char size,
    unsigned char initialColor, short *outW, short *outH, int fontIndex)
{
    if (argText == NULL)
        return;

    const char* text = argText;
    unsigned char colorIndex = initialColor;

    if (size == 12)
        fontIndex = 1;

    *outH = 0;
    *outW = 0;

    int yAdvance = size;

    short initialX = x;
    short initialY = y;

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
        
        unsigned char thisCharWidth = 1 + data_020e7bd8[fontIndex];
        FontIndexFile::Glyph* glyph;
        glyph = GetNextFontGlyph(text, fontIndex);
        
        if (glyph != NULL)
        {
            func_0204df9c(this, glyph, x, y, colorIndex, fontIndex);
            thisCharWidth = glyph->width + 1;
            short candidateH = y + yAdvance;
            short candidateW = x + thisCharWidth;
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

void UIElement::RenderNumber(short x, short y, int number, unsigned char size,
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

void UIElement::FillRectangle2(int colorIndex, short left, short top, short right, short bottom)
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
            
            unsigned char* pByte = &pixelBuffer_8_[byteIndex];
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

void UIElement::BlitGrayCursor(short x, short y)
{
    if (!(flags_c5_ & RendererFlags_40_DrawGrayCursor))
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

void UIElement::DrawPageArrows()
{
    short x = 0;
    short height = numTilesY_aa_ * 8;
    short y = height - 11;
    short offset = 0;
    if (flags_c5_ & RendererFlags_10)
    {
        offset = 3;
        y += offset;
    }
    x += 4;
    x -= offset;
    const unsigned char* graphic = func_02042a50(8, data_020f0303);
    if (graphic != NULL)
        BlitBitmap1Bit(graphic, x, y, 8, 8, 0xf0, 0xf);
    short width = numTilesX_a8_ * 8;
    x = width - 10;
    x += offset;
    graphic = func_02042a50(8, data_020f0306);
    if (graphic != NULL)
        BlitBitmap1Bit(graphic, (short)(x - 1), y, 8, 8, 0xf0, 0xf);
}

void UIElement::Unmap()
{
    DestinationData* data = destinationData_4_;
    int bgIndex = data->GetBGIndex();
    int screen =  data->GetScreen();
    LoadToBGCharProc loadProc = func_0204a5e4(screen, bgIndex);
    CleanInvalidateCacheRange(pixelBuffer_8_, bufferSize_a4_);
    loadProc(pixelBuffer_8_, vramOffset_a0_, bufferSize_a4_);
}

bool UIElement::CheckValueTag(const char **ppText, const char *candidate, short *outValues, int numValues)
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

int UIElement::TestCornerOverlapAlignment(unsigned int mask)
{
    int result;
    short top, left, right, bottom;
    short otherLeft, otherTop, otherRight, otherBottom;

    UIElement* loopControl = next_0_;
    if (loopControl == NULL)
        return 0;
    
    left = leftTile_ac_;
    top = topTile_ae_;
    right = left + numTilesX_a8_;
    bottom = top + numTilesY_aa_;
    
    unsigned char screen = 0;
    result = 0;
    if (destinationData_4_ != NULL)
        screen = destinationData_4_->screen;
    
    while (loopControl != NULL)
    {
        if (loopControl->destinationData_4_ != NULL && destinationData_4_ != NULL &&
            screen != loopControl->destinationData_4_->screen)
        {
            loopControl = loopControl->next_0_;
            continue;
        }
        if (loopControl->flags_c5_ & RendererFlags_20)
        {
            loopControl = loopControl->next_0_;
            continue;
        }
        if ((loopControl->flags_c5_ & RendererFlags_04) && (loopControl->flags_c5_ & RendererFlags_10))
        {
            loopControl = loopControl->next_0_;
            continue;
        }
        otherLeft = loopControl->leftTile_ac_;
        otherTop = loopControl->topTile_ae_;
        otherRight = otherLeft + loopControl->numTilesX_a8_;
        otherBottom = otherTop + loopControl->numTilesY_aa_;
        if ((mask & 1) && TestPointInBox(left, top, otherLeft, otherTop, otherRight, otherBottom)
            && (left != otherLeft || top != otherTop))
        {
            result = 5;
            if (left == otherLeft)
                result = 1;
            else if (top == otherTop)
                result = 2;
            break;
        }
        if ((mask & 2) && TestPointInBox(right - 1, top, otherLeft, otherTop, otherRight, otherBottom)
            && (right != otherRight || top != otherTop))
        {
            result = 5;
            if (right == otherRight)
                result = 3;
            else if (top == otherTop)
                result = 2;
            break;
        }
        if ((mask & 4) && TestPointInBox(left, bottom - 1, otherLeft, otherTop, otherRight, otherBottom)
            && (left != otherLeft || bottom != otherBottom))
        {
            result = 5;
            if (left == otherLeft)
                result = 1;
            else if (bottom == otherBottom)
                result = 4;
            break;
        }
        if ((mask & 8) && TestPointInBox(right - 1, bottom - 1, otherLeft, otherTop, otherRight, otherBottom)
            && (right != otherRight || bottom != otherBottom))
        {
            result = 5;
            if (right == otherRight)
                result = 3;
            else if (bottom == otherBottom)
                result = 4;
            break;
        }
        loopControl = loopControl->next_0_;
    }
    return result;
}

void UIElement::DrawGrayCursor()
{
    if (destinationData_4_ == NULL || pixelBuffer_8_ == NULL)
        return;
    
    int screen = destinationData_4_->GetScreen();
    int bg = destinationData_4_->GetBGIndex();

    unsigned char* charBase = GetCharacterBase();
    if (charBase == NULL)
        return;
    if (charBase != NULL)
        memcpy(pixelBuffer_8_, charBase + vramOffset_a0_, bufferSize_a4_);

    flags_c5_ |= RendererFlags_40_DrawGrayCursor;
    BlitGrayCursor(cursorX_bc_, cursorY_be_);

    LoadToBGCharProc loadProc = func_0204a5e4(screen, bg);
    CleanInvalidateCacheRange(pixelBuffer_8_, bufferSize_a4_);
    loadProc(pixelBuffer_8_, vramOffset_a0_, bufferSize_a4_);
}

unsigned char* UIElement::GetCharacterBase() const
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

void UIElement::RefreshPageNumber(bool stage)
{
    if (!maybePageNumbersEnabled_d8_0_ || (!pageNumberDirty_d8_1_ && stage))
        return;

    if (destinationData_4_ == NULL || pixelBuffer_8_ == NULL)
        return;

    int screen =  destinationData_4_->GetScreen();
    int bgIndex = destinationData_4_->GetBGIndex();

    if (stage)
    {
        unsigned char* characterData = GetCharacterBase();
        if (characterData == NULL)
            return;
        if (characterData != NULL)
            memcpy(pixelBuffer_8_, characterData + vramOffset_a0_, bufferSize_a4_);
    }
    short firstNumberRight;
    short boxHeight = numTilesY_aa_ * 8;
    short renderY = boxHeight - 13;

    short boxWidth = numTilesX_a8_ * 8;
    short centerX = boxWidth / 2;
    
    if (flags_c5_ & RendererFlags_10)
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
    if (flags_c5_ & RendererFlags_04)
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
        CleanInvalidateCacheRange(pixelBuffer_8_, bufferSize_a4_);
        proc(pixelBuffer_8_, vramOffset_a0_, bufferSize_a4_);
    }

    pageNumberDirty_d8_1_ = false;
}

void UIElement::ResetPageNumbers(bool enabled)
{
    maybePageNumbersEnabled_d8_0_ = enabled;
    pageNumberDirty_d8_1_ = true;
    pageNumber_d9_ = 0;
    numPages_da_ = 0;
}

void UIElement::SetPageNumber(unsigned char to)
{
    pageNumber_d9_ = to;
    pageNumberDirty_d8_1_ = true;
}

void UIElement::SetPageCount(unsigned char to)
{
    numPages_da_ = to;
    pageNumberDirty_d8_1_ = true;
}