#pragma once

#include <globaldefs.h>
#include "OBJSpriteSet.h"
#include "Font.h"

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
    void BlitText(short x, short y, const char* text,
        unsigned char fontSize, unsigned char colorIndex);
#endif

#if defined(usa)
    void BlitGlyph(const FontIndexFile::Glyph* glyph, short x, short y,
        unsigned char colorIndex, unsigned char fontIndex);
#elif defined(jpn)
    // in jpn a glyph is just a 1bpp bitmap with arbitrary size
    void BlitGlyph(const void* argGlyphBytes, short x, short y,
        unsigned char fontSize, unsigned char colorIndex);
#endif

    // colorIndex completely unused as pixel data provides colors.
    // fontSize unused in international version, all tiles assumed 8x8
    void BlitTile(const void* pixels, short x, short y, unsigned char fontSize, unsigned char colorIndex);

    // alignment: 0 = left, 1 = right
#if defined(usa)
    void BlitNumber(short x, short y, int number, unsigned char colorIndex,
        unsigned char widthTiles, unsigned char alignment, unsigned char fontIndex);
#elif defined(jpn)
    void BlitNumber(short x, short y, int number, unsigned char fontSize,
        unsigned char colorIndex, unsigned char widthInChars, unsigned char alignment);
#endif

    void FillRectangle(short left, short top, short right, short bottom, unsigned char colorIndex);

    static void PrepareRegionsFromSprite(BlitRegion* regions, int numRegions, OBJSpriteSet::Metatile* sprite);
};