#include "Graphics/Blitter.h"

Blitter4Bit::BlitRegion* GetBlitRegion(Blitter4Bit::BlitRegion* regions, unsigned int count, int x, int y)
{
    Blitter4Bit::BlitRegion* candidate = regions;
    for (unsigned char i = 0; i < count; candidate++, i++)
    {
        if (candidate->originX <= x && x < candidate->originX + candidate->width &&
            candidate->originY <= y && y < candidate->originY + candidate->height)
                return candidate;
    }
    return NULL;
}

void Blitter4Bit::Reset()
{
    destination_ = NULL;
    regions_ = NULL;
    numRegions_ = false;
}

void Blitter4Bit::SetDestination(unsigned char* to)
{
    destination_ = to;
}

void Blitter4Bit::SetRegions(BlitRegion* regions, unsigned char count)
{
    regions_ = regions;
    numRegions_ = count;
}

int Blitter4Bit::BlitText(short x, short y,
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

void Blitter4Bit::BlitGlyph(const FontIndexFile::Glyph *glyph, short x, short y,
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

void Blitter4Bit::BlitTile(const void *pixels, short left, short top)
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