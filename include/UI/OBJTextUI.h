#pragma once

#include "../Graphics/ClipWindow.h"
#include "../Graphics/OBJBlitter.h"
#include <globaldefs.h>

// sizeof == 0x4c.
// Used for rendering text/UI elements with the OBJ system, predominantly dialogue
// while in the items/spells & abilities menu and the yes/no option box in shops.
// owns bytes 1c0-1ff for the relevant engine's obj standard palette (i.e. palettes 14, 15)
class OBJTextUI
{
public:
    struct Region
    {
        unsigned int shapeSize;
        // relative to the owning OBJGraphics' vram offset
        unsigned int relVramOffset;
        unsigned char x;
        unsigned char y;
        unsigned char width;
        unsigned char height;

        void Reset();
    };

    struct CharBufferDesc
    {
        unsigned int offset;
        unsigned int length;
        unsigned char layer;

        unsigned char* GetBase() const;
        void Reset();
    };

    struct CharacterBuffers
    {
        CharBufferDesc bufferA;
        CharBufferDesc bufferB;

        void SetBufferA(unsigned int offset, unsigned int length, unsigned char layer);
        void SetBufferB(unsigned int offset, unsigned int length, unsigned char layer);

        void LoadAToB();
        void LoadBToA();
    };

    struct OBJGraphics
    {
        OBJBlitter::BlitRegion* blitRegions;
        OBJBlitter* blitter;
        Region* regions;
        CharacterBuffers charBuffers;
        Region backgroundRegion;
        // pixel data copied from stagingBuffer + vramOffset into (obj vram base) + vramOffset
        unsigned int vramOffset;
        unsigned int charDataSize;
        unsigned char baseX;
        unsigned char baseY;
        unsigned char width;
        unsigned char height;
        unsigned char numRegions;
        unsigned char oamIndexBase;
        unsigned char engine;
        unsigned char readyToSubmit : 1;
        unsigned char pendingClose : 1;

        unsigned int GetPixelDataSize() const;
        void Reset();
    };

    struct BoundingRect
    {
        short left;
        short top;
        short right;
        short bottom;

        void Set(short l, short t, short r, short b);
    };

    unsigned char* stagingBuffer_;
    OBJGraphics* graphics_;
    char numOptions_;
    BoundingRect optionBounds_[3];
    // 0: default (white), 1: highlight (green)
    unsigned short textColors_[2];
    char pad_26[2];
    ClipWindow clipWindow_;

public:
    void Initialize(int screen, int windowID);
    // loads to palette #14
    void LoadColorToPalette(int palettePosition, int index);
    void SetColor(int index, unsigned short value);
    void ReloadDefaultColors();

    void SubmitToOAM();
    void ConfigureClipWindow();
    OBJGraphics* GetGraphics();
    unsigned int CreateInVRAM(unsigned int objVramSize);
    void RequestClose();

    bool CreateYesNoBox(unsigned int objVramSize);
    void GetOptionPosition(short* outX, short* outY, int option);

    int GetTouchSelection();
    int GetPointSelection(short x, short y);

    void* BeginRender();
    void Clear();

    void DrawTextBoxBorder();
#if defined(usa)
    void DrawText(int textID, int x, int y, const char* text,
        unsigned char colorIndex, unsigned char fontIndex);
#elif defined(jpn)
    void DrawText(int textID, int x, int y, const char* text, unsigned char size, unsigned char colorIndex);
#endif
    void FillRectangle(unsigned char colorIndex, short left, short top, short right, short bottom);

    void EndRender();
    void FreeStagingMemory();
};