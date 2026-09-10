#pragma once

// sizeof == 0x1c.
struct Glyph
{
    char unk_0[4];
    unsigned int textureVRAMOffset;
    int unknown_8;
    int drawX;
    int drawY;
    short drawZ;
    unsigned char unk_16_low : 1;
    unsigned char unk_16_1 : 1;
    unsigned char unk_16_2 : 1;
    char unk_17[1];
    void* unknown_18;

    // Not used in all text rendering, only certain NPCs, combat text
    // and item pickup in overland
    void Draw(int color, void* unknown, int alpha);
};

// sizeof == 0x1e2c == 7724.
// Dynamically allocated by func_020421c4. 
class TextManager
{
public:
    char unk_0[0x48];
    void* ptr_48;
    void* ptr_4c;
    void* ptr_50;
    char unk_54[8];
    void* ptr_5c;
    void* ptr_60;
    void* ptr_64;
    char unk_68[0x9b8 - 0x68];
    Glyph glyphs_[0x80];
    char unk_17b8[0x1e2c - 0x17b8];

    static TextManager* GetInstance();
};
