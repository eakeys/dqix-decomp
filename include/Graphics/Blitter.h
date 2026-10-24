#pragma once

#include <globaldefs.h>
#include "../UI/Text.h"

class Blitter4Bit
{
public:
    struct BlitRegion
    {
        int offset;
        short originX;
        short originY;
        short width;
        short height;
    };

    unsigned char* destination_;
    BlitRegion* regions_;
    unsigned char numRegions_;

    void Reset();
    void SetDestination(unsigned char* to);
    void SetRegions(BlitRegion* desc, unsigned char count);

    // returns text width. Doesn't support line breaks
    int BlitText(short x, short y, const char* text,
        unsigned char colorIndex, unsigned char fontIndex);

    void BlitGlyph(const FontIndexFile::Glyph* glyph, short x, short y,
        unsigned char colorIndex, unsigned char fontIndex);

    void BlitTile(const void* pixels, short x, short y);
};