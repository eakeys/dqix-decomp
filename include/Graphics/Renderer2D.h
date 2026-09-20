#pragma once

#include "../Memory/SafeAllocator.h"
#include "std_library_functions.h"
#include "Vector.h"



// sizeof == 0xe0.
// Dynamically allocated by func_020421c4.
class Renderer2D
{
public:
    // sizeof == 0x20. This is its own thing but I don't know what yet,
    // see e.g. func_0205d594 that modifies an instance of it.
    struct DestinationData
    {
        char unk_0[0x1c];
        unsigned char screen : 4; // 0 = main, 1 = sub (why 4 bits?)
        unsigned char bgIndex : 4; // BG0 to BG3 (why 4 bits?)

        inline int GetScreen() const { return screen; }
        inline int GetBGIndex() const { return bgIndex; }
    };
public:
char unk_0[4];
    DestinationData* destinationData_4_;
    unsigned char* stagingData_8_;
    short unknown_array_c_[18];
    short unknown_array_30_[18];
    short unknown_array_54_[18];
    short unknown_array_78_[18];
    int unknown_9c_;
    unsigned int vramOffset_a0_;
    unsigned int bufferSize_a4_;
    short numTilesX_a8_;
    short numTilesY_aa_;
char unk_ac[4];
    short unknown_b0_;
    short unknown_b2_;
char unk_b4[2];
    short lineSpacing_b6_;
char unk_b8[4];
    short cursorX_bc_;
    short cursorY_be_;
char unk_c0[5];
    // bit 6: draw the gray cursor
    unsigned char flags_c5_;
char unk_c6[0xd4 - 0xc6];
    unsigned short* unknown_d4_;
    unsigned char maybePageNumbersEnabled_d8_0_ : 1;
    unsigned char pageNumberDirty_d8_1_ : 1;
    unsigned char unk_d8_bits : 6;
    unsigned char pageNumber_d9_; // zero-indexed
    unsigned char numPages_da_;
char unk_db[1];
    char unknown_dc_;
    char unknown_dd_;
    char unknown_de_;
char unk_df[1];

    void FillRectangle(int colorIndex, short left, short top, short right, short bottom);

    void BlitBitmap4Bit(const void* src, short x, short y, short width, short height, unsigned char bitmaskHigh, unsigned char bitmaskLow);
    int PrepareText(const char* text, int unused, bool bool4, bool bool5, unsigned char fontIndex);

    void RenderText(short x, short y, const char* text, unsigned char size, unsigned char colorIndex, 
        short* outW, short* outH, int fontIndex);

    void RenderNumber(short x, short y, int number, unsigned char size, unsigned char colorIndex,
        short* outW, short* outH, bool rightAlign, unsigned char numDigits,
        bool alwaysShowSign, bool fixedNumDigits);

    void FillRectangle2(int colorIndex, short left, short top, short right, short bottom);
    void DrawGrayCursor(short x, short y);

    void Unmap();
    bool CheckValueTag(const char** ppText, const char* candidate, short* outValues, int numValues);

    unsigned char* GetCharacterBase() const;
    void RefreshPageNumber(bool stage);

    void ResetPageNumbers(bool enabled);
    void SetPageNumber(unsigned char to);
    void SetPageCount(unsigned char to);

private:
    inline short GetWidth() const { return numTilesX_a8_ << 3; }
    inline short GetHeight() const { return numTilesY_aa_ << 3; }
};