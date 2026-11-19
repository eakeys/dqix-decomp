#pragma ipa file
#include "Graphics/OBJBlitter.h"

extern char data_020f0518[];

static OBJBlitter::BlitRegion* GetBlitRegion(OBJBlitter::BlitRegion* regions, unsigned int count, int x, int y)
{
    OBJBlitter::BlitRegion* candidate = regions;
    for (unsigned char i = 0; i < count; candidate++, i++)
    {
        if (candidate->originX <= x && x < candidate->originX + candidate->width &&
            candidate->originY <= y && y < candidate->originY + candidate->height)
                return candidate;
    }
    return NULL;
}

void OBJBlitter::Reset()
{
    destination_ = NULL;
    regions_ = NULL;
    numRegions_ = false;
}

void OBJBlitter::SetDestination(unsigned char* to)
{
    destination_ = to;
}

void OBJBlitter::SetRegions(BlitRegion* regions, unsigned char count)
{
    regions_ = regions;
    numRegions_ = count;
}

#if defined(usa)
int OBJBlitter::BlitText(short x, short y,
    const char* text, unsigned char colorIndex, unsigned char fontIndex)
{
    if (destination_ == NULL)
        return 0;
    if (regions_ == NULL)
        return 0;
    if (numRegions_ == 0)
        return 0;
    if (text == NULL)
        return 0;

    unsigned char spacing = GetFontSpaceSize(fontIndex);
    int lastGlyphIndex = 0xff;
    short drawX = x;
    while (*text != 0)
    {
        // horrible but it works
        int thisGlyphIndex;
        char glyphWidth = spacing + 1;
        thisGlyphIndex = GetNextFontGlyphIndex(text, fontIndex) & 0xff;
        drawX += GetFontGlyphKerning(lastGlyphIndex, thisGlyphIndex, fontIndex);
        int tagLength = 1;
        if (thisGlyphIndex != 0xff)
        {
            FontIndexFile::Glyph* glyph = GetFontGlyphByIndex(thisGlyphIndex, fontIndex);
            BlitGlyph(glyph, drawX, y, colorIndex, fontIndex);
            glyphWidth = glyph->width + 1;
            tagLength = glyph->tagLength;
        }
        lastGlyphIndex = thisGlyphIndex;
        text += tagLength;
        drawX += glyphWidth;
    }

    return drawX - x;
}
#endif

void OBJBlitter::BlitGlyph(const FontIndexFile::Glyph *glyph, short x, short y,
    unsigned char colorIndex, unsigned char fontIndex)
{
    FontDataFile* dataFile = GetFontDataFile(fontIndex);
    short maybeOffset = glyph->dataOffset;
    unsigned char colorMaskHigh = colorIndex << 4;
    unsigned char colorMaskLow = colorIndex & 15;
    int srcScanlineStride = dataFile->bitsPerScanline >> 3;
    char initialBitShift = 7 - (glyph->dataOffset & 7);

    unsigned int numRegions = numRegions_;
    BlitRegion* regionList = regions_;

    for (int srcY = 0; srcY < 12; srcY++)
    {
        short targetY = y + srcY;
        char bitShift = initialBitShift;
        unsigned char* sourceBits = dataFile->ptr_c + srcScanlineStride * srcY + (maybeOffset >> 3);
        for (int srcX = 0; srcX < glyph->width; srcX++)
        {
            if (*sourceBits & (1 << bitShift))
            {
                short targetX = x + srcX;
                BlitRegion* region = GetBlitRegion(regionList, numRegions, targetX, targetY);
                if (region != NULL)
                {
                    unsigned char* tilePtr = destination_ + region->offset;
                    short relativeX = targetX - region->originX;
                    short relativeY = targetY - region->originY;
                    short relativeTileX = relativeX >> 3;
                    short relativeTileY = relativeY >> 3;
                    short regionNumTilesX = region->width >> 3;
                    short firstWriteTile = relativeTileY * regionNumTilesX + relativeTileX;
                    tilePtr += firstWriteTile * 32;
                    short xWithinTile = relativeX & 7;
                    short yWithinTile = relativeY & 7;
                    unsigned char* writePtr = tilePtr + ((yWithinTile << 2) + (xWithinTile >> 1));
                    if (targetX & 1)
                    {
                        *writePtr &= 0x0f;
                        *writePtr |= colorMaskHigh;
                    }
                    else
                    {
                        *writePtr &= 0xf0;
                        *writePtr |= colorMaskLow;
                    }
                }
            }
            bitShift--;
            if (bitShift < 0)
            {
                bitShift = 7;
                sourceBits++;
            }
        }
    }
}

void OBJBlitter::BlitTile8x8(const void *pixels, short left, short top, int, int)
{
    if (pixels == NULL)
        return;
    const unsigned char* sourcePixels = (const unsigned char*)pixels;
    unsigned int numRegions = numRegions_;
    unsigned char* dest = destination_;
    BlitRegion* regionList = regions_;
    
    short drawX;
    short drawY = top;
    for (short y = 0; y < 8; drawY++, y++)
    {
        drawX = left;
        for (short x = 0; x < 8; drawX++, x++)
        {
            BlitRegion* region = GetBlitRegion(regionList, numRegions, drawX, drawY);
            if (region == NULL)
                continue;

            short relativeX = drawX - region->originX;
            short relativeY = drawY - region->originY;
            short regionNumTilesX = region->width >> 3;
            unsigned char* destRegionStart = dest + region->offset;
            short xWithinTileByteOffset = (short)(relativeX & 7) >> 1;
            short yWithinTile = (relativeY & 7);
            int offsetInTile;
            unsigned char* tilePtr = destRegionStart + ((short)(relativeY >> 3) * regionNumTilesX + (relativeX >> 3)) * 32;
            offsetInTile = (yWithinTile << 2) + xWithinTileByteOffset;

            int sourceByte = sourcePixels[x >> 1];
            unsigned char writeColorMask;

            if (relativeX & 1)
            {
                if (x & 1)
                    writeColorMask = sourceByte & 0xf0;
                else
                    writeColorMask = (sourceByte << 4) & 0xf0;

                if (writeColorMask != 0)
                {
                    tilePtr[offsetInTile] &= 0x0f;
                    tilePtr[offsetInTile] |= writeColorMask;
                }
            }
            else
            {
                if (x & 1)
                    writeColorMask = (sourceByte >> 4) & 0x0f;
                else
                    writeColorMask = sourceByte & 0x0f;
                
                if (writeColorMask != 0)
                {
                    tilePtr[offsetInTile] &= 0xf0;
                    tilePtr[offsetInTile] |= writeColorMask;
                }
            }
        }

        sourcePixels += 4;
    }
}

void OBJBlitter::BlitNumber(short x, short y, int number, unsigned char colorIndex,
    unsigned char widthTiles, unsigned char alignment, unsigned char fontIndex)
{
    if (destination_ == NULL || regions_ == NULL)
        return;
    
    int useless = number;
    while (useless /= 10) {}

    int textWidth = MeasureNumberTextWidth(fontIndex, number);
    if (alignment == 1)
    {
        short offset = widthTiles * 8 - textWidth;
        x += offset;
    }

    char buffer[10] = {0};
    sprintf(buffer, data_020f0518, number);
    BlitText(x, y, buffer, colorIndex, fontIndex);
}

void OBJBlitter::FillRectangle(short left, short top, short right, short bottom, unsigned char colorIndex)
{
    if (destination_ == NULL || regions_ == NULL)
        return;
    
    unsigned char* destination = destination_;
    unsigned char numRegions = numRegions_;
    BlitRegion* regions = regions_;

    unsigned char highMask, lowMask;
    lowMask = colorIndex & 0xf;
    highMask = lowMask << 4;
    short width = right - left;
    short height = bottom - top;

    short dstX, dstY;
    dstY = top;
    for (short srcY = 0; srcY < height; dstY++, srcY++)
    {
        dstX = left;
        for (short srcX = 0; srcX < width; dstX++, srcX++)
        {
            BlitRegion* region = GetBlitRegion(regions, numRegions, dstX, dstY);
            if (region == NULL)
                continue;
            unsigned char* writePtr = destination + region->offset;
            short regionX = dstX - region->originX;
            short regionY = dstY - region->originY;
            int regionTileX = regionX >> 3;
            short regionTileY = regionY >> 3;
            short regionTileStride = region->width >> 3;
            writePtr += (regionTileY * regionTileStride + regionTileX) * 32;
            short tileInnerX = regionX & 7;
            short tileInnerY = regionY & 7;
            int tileWriteOffset = (tileInnerY << 2) + (tileInnerX >> 1);
            if (regionX & 1)
            {
                writePtr[tileWriteOffset] &= 0x0f;
                writePtr[tileWriteOffset] |= highMask;
            }
            else
            {
                writePtr[tileWriteOffset] &= 0xf0;
                writePtr[tileWriteOffset] |= lowMask;
            }
        }
    }
}

void OBJBlitter::PrepareRegionsFromSprite(BlitRegion* regions, int numRegions, OBJSpriteSet::Metatile* sprite)
{
    if (regions == NULL || sprite == NULL)
        return;
    if (numRegions < sprite->nitroData.numObjs)
        return;
    unsigned short numObjs = sprite->nitroData.numObjs;
    
    if (sprite->oamEntries == NULL)
        return;

    OAMEntry* oamEntry = sprite->oamEntries;
    int firstObjIndex;
    int firstObjX, firstObjY;
    for (unsigned short i = 0; i < numObjs; oamEntry++, i++)
    {
        short originX = oamEntry->word1 & 0x1f; // why only go up to 32?
        short originY = oamEntry->word0 & 0xf; // why only go up to 16?

        short width, height;

        unsigned short shape = (oamEntry->word0 & 0xc000) >> 14;
        unsigned short size = (oamEntry->word1 & 0xc000) >> 14;
        switch (shape)
        {
        case 0: // square
            if (size == 0)
            {
                width = 8;
                height = 8;
            }
            else if (size == 1)
            {
                width = 16;
                height = 16;
            }
            else if (size == 2)
            {
                width = 32;
                height = 32;
            }
            else if (size == 3)
            {
                width = 64;
                height = 64;
            }
            break;
        case 1: // horizontal
            if (size == 0)
            {
                width = 16;
                height = 8;
            }
            else if (size == 1)
            {
                width = 32;
                height = 8;
            }
            else if (size == 2)
            {
                width = 32;
                height = 16;
            }
            else if (size == 3)
            {
                width = 64;
                height = 32;
            }
            break;
        case 2: // vertical
            if (size == 0)
            {
                width = 8;
                height = 16;
            }
            else if (size == 1)
            {
                width = 8;
                height = 32;
            }
            else if (size == 2)
            {
                width = 16;
                height = 32;
            }
            else if (size == 3)
            {
                width = 32;
                height = 64;
            }
        }

        int objIndex = oamEntry->word2 & 0x3ff;
        if (i == 0)
        {
            firstObjX = originX;
            firstObjY = originY;
            firstObjIndex = objIndex;
        }
#if defined(jpn)
        regions[i].offset = objIndex - firstObjIndex;
        regions[i].originX = originX - firstObjX;
        regions[i].originY = originY - firstObjY;
#else
        regions[i].offset = (objIndex - firstObjIndex) * 32;
        originX -= firstObjX;
        originY -= firstObjY;
        regions[i].originX = originX;
        regions[i].originY = originY;
#endif
        regions[i].width = width;
        regions[i].height = height;
    }
}