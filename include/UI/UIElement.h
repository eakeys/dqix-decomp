#pragma once

#include "../Memory/SafeAllocator.h"
#include "std_library_functions.h"
#include "../Graphics/Vector.h"
#include "../Graphics/BGLayer.h"
#include "../Graphics/Font.h"

// sizeof == 0xe0.
// Dynamically allocated by func_020421c4.
class UIElement
{
public:
    enum RendererFlags
    {
        RendererFlags_01 = 1,
        RendererFlags_02 = 2,
        RendererFlags_04 = 4,
        RendererFlags_08 = 8,
        RendererFlags_10 = 0x10,
        RendererFlags_20 = 0x20,
        RendererFlags_40_DrawGrayCursor = 0x40,
        RendererFlags_80 = 0x80
    };

public:
    UIElement* next_0_;
    BGLayer* layer_4_;
    unsigned char* pixelBuffer_8_;
    short elementXValues_[18];
    short elementYValues_[18];
    short elementWidths_[18];
    short elementHeights_[18];
    // values 0-4, 0 = uninitialized, 1 = ready to stage?
    // 3 = fully loaded?
    // 4 = ??? from 4 it moves back to 3
    int status_9c_;
    unsigned int vramOffset_a0_;
    unsigned int bufferSize_a4_;
    short numTilesX_a8_;
    short numTilesY_aa_;
    short leftTile_ac_;
    short topTile_ae_;
    short textXOffset_b0_;
    short textYOffset_b2_;
    short unknown_b4_;
    short lineSpacing_b6_;
    short unknown_b8_;
    short overrideLineSpacing_ba_;
    short cursorX_bc_;
    short cursorY_be_;
    short unknown_c0_;
    unsigned short borderPalette_c2_;
    // I don't see this used anywhere within the class but it's set alongside vram offset
    // so probably related to that in some way
    unsigned char unknown_c4_;
    unsigned char flags_c5_;
    unsigned char unknown_c6_;
    unsigned char fontHeight_c7_;
    BGLayer::Sprite sprite_c8_;
    unsigned char maybePageNumbersEnabled_d8_0_ : 1;
    unsigned char pageNumberDirty_d8_1_ : 1;
    unsigned char useBG_d8_2_ : 1;
    unsigned char unk_d8_bits : 5;
    unsigned char pageNumber_d9_; // zero-indexed
    unsigned char numPages_da_;
    // top, bottom, left, right
    char backgroundMargins_db_[4];
    char pad_df[1];

    void GetElementBounds(short elementID, short* outX, short* outY, short* outW, short* outH) const;
    void SetElementBounds(short elementID, short x, short y, short w, short h);
    void Reset();
    // probably Destroy() (but not a destructor)
    void FullReset();

    void SetMargins(const char* margins);
    // SafeAllocator used to allocate index buffer
    void Allocate(SafeAllocator* allocator, void* pixelBuffer, unsigned int indexBufferSize);

    bool IsFullyLoaded() const;
    // Returns true if the index buffer pointer is not null and status != 0
    // (i.e. the dimensions for the index buffer are configured)
    bool IsIndexBufferUsable() const;

    void UseDefaultSettings();
    void Update(int numTicks);
    void Display(); // technically only displays the foreground. needs to run every frame

    // updates and reassigns indices
    void FullUpdate();

    void Create(const char* markup, int vramOffset, int unused, bool vCentering, bool expandToFit, unsigned char fontIndex);
    void LoadMenuPage(const char* markup, bool expandToFit, unsigned char fontIndex);

    void FillRectangle(int colorIndex, short left, short top, short right, short bottom);
    void PrepareMenu(const char* markup, short border, unsigned char font);

    void BlitGlyph(const FontIndexFile::Glyph* glyph, short x, short y, unsigned char color, unsigned char font);
    void BlitBitmap1Bit(const void* src, short x, short y, short width, short height, unsigned char bitmaskHigh, unsigned char bitmaskLow);
    void BlitBitmap4Bit(const void* src, short x, short y, short width, short height, unsigned char bitmaskHigh, unsigned char bitmaskLow);
    short PrepareText(const char* text, int unused, bool vCentering, bool expandToFit, unsigned char fontIndex);

    void AssignIndices();

    void RoundCorners();
    // First two arguments are probably x, y but they're never used, the control's dimensions
    // themselves are used instead. w, h are only checked to be non-negative
    void DrawRoundedRectangleBorder(short, short, short w, short h, const char* margins);
    void ClearElementBounds();
    void GetElementBoundArrays(short** outXArray, short** outYArray, short** outWArray, short** outHArray);
    void SetVRAMOffset(int offset, unsigned char argc4);

    void DrawRoundedRectangle(short x, short y, short w, short h, unsigned char borderColor, bool noBackground, bool noBorder, const char* margins);
    void DrawDividingLine(short y);
    void RenderText(short x, short y, const char* text, unsigned char size, unsigned char colorIndex, 
        short* outW, short* outH, unsigned char fontIndex);

    void RenderNumber(short x, short y, int number, unsigned char size, unsigned char colorIndex,
        short* outW, short* outH, bool rightAlign, unsigned char numDigits,
        bool alwaysShowSign, bool fixedNumDigits);

    void FillRectangle2(int colorIndex, short left, short top, short right, short bottom);
    void BlitGrayCursor(short x, short y);
    void DrawPageArrows();

    void Unmap();
    bool CheckValueTag(const char** ppText, const char* candidate, short* outValues, int numValues);

    // Test specified corners of this control (1 = top left, 2 = top right, 4 = bottom left,
    // 8 = bottom right) to see if they align with an edge of a control further down the list
    // (i.e. a menu behind it). Returns 0 if no overlap, 1,2,3,4 for alignment to a
    // left/top/right/bottom edge, and 5 for generic overlap
    int TestCornerOverlapAlignment(unsigned int corners);

    // wraps BlitGrayCursor() in calls to map and unmap the memory
    void DrawGrayCursor();

    unsigned char* GetCharacterBase() const;
    void RefreshPageNumber(bool stage);

    void ResetPageNumbers(bool enabled);
    void SetPageNumber(unsigned char to);
    void SetPageCount(unsigned char to);

private:
    inline short GetWidth() const { return numTilesX_a8_ << 3; }
    inline short GetHeight() const { return numTilesY_aa_ << 3; }
};