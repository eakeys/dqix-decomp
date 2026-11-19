#pragma once

#include <globaldefs.h>
#include "../UI/Text.h"
#include "OBJSpriteSet.h"

// sizeof == 0xc
// used as a sort of render texture in OBJ memory
class OBJBlitter
{
public:
    struct BlitRegion
    {
        int offset;
        short originX;
        short originY;
        short width;
        short height;

        // func_02020520, almost certainly an inline function emitted there
        // - not going to decompile it yet, it just sets everything to 0
        void Reset();
    };

    unsigned char* destination_;
    BlitRegion* regions_;
    unsigned char numRegions_;

    void Reset();
    void SetDestination(unsigned char* to);
    void SetRegions(BlitRegion* desc, unsigned char count);

#if defined(usa)
    // returns text width. Doesn't support line breaks
    int BlitText(short x, short y, const char* text,
        unsigned char colorIndex, unsigned char fontIndex);
#elif defined(jpn)
    int BlitText(short x, short y, const char* text,
        unsigned char fontSize, unsigned char colorIndex);
#endif

    void BlitGlyph(const FontIndexFile::Glyph* glyph, short x, short y,
        unsigned char colorIndex, unsigned char fontIndex);

    // last two args unused but were probably intended to be tile dimensions
    // (always 8) and color (15)
    void BlitTile8x8(const void* pixels, short x, short y, int, int);

    // alignment: 0 = left, 1 = right
    void BlitNumber(short x, short y, int number, unsigned char colorIndex,
        unsigned char widthTiles, unsigned char alignment, unsigned char fontIndex);

    void FillRectangle(short left, short top, short right, short bottom, unsigned char colorIndex);

    static void PrepareRegionsFromSprite(BlitRegion* regions, int numRegions, OBJSpriteSet::Metatile* sprite);
};