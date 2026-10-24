#pragma once

#include <std_library_functions.h>

// note for posterity: be careful if porting this to 64-bit, the file is loaded
// by memcpy where all pointers hold 32-bit offsets from the start of the file
// and then struct base gets added!
struct FontIndexFile
{
    unsigned int maybeSignature; // seems to hold "1.1\0"
    unsigned int numCharEntries;
    unsigned int numKerningEntries;

    struct KerningEntry
    {
        // low 8 bits: first (left) glyph, high 8 bits: second (right) glyph
        uint16_t glyphPair;
        // negative: move closer together, positive: move further apart
        int8_t kerningAmount;
        char pad_3[1];
    };

    struct Glyph
    {
        // usually a single character, but can be e.g. "<1>". Most likely when
        // this is found in a string, the corresponding glyph is rendered. Points
        // into the tag pool (after setup is done)
        const char* tag;
        int8_t width;
        int8_t tagLength : 6;
        int8_t isVowel : 1;
        int8_t unk_5_bit_7 : 1;
        int16_t unknown_6;
    };

    KerningEntry* kerning;
    Glyph* glyphs;
    const char* tagPool; // pool of all null-terminated tags
};

struct FontDataFile
{
    unsigned int maybeSignature; // seems to hold "1.0\0"
    unsigned short unknown_4;
    char unk_6[6];
    unsigned char* ptr_c;
};