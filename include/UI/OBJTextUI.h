#pragma once

#include "../Graphics/ClipWindow.h"
#include "../Graphics/OBJBlitter.h"
#include <globaldefs.h>

// sizeof == 0x4c.
// Used for rendering text/UI elements with the OBJ system, predominantly dialogue
// while in the items/spells & abilities menu and the yes/no option box in shops.
// owns bytes 1c0-1ff for the relevant engine's obj standard palette
class OBJTextUI
{
public:
    struct RegionStructC
    {
        unsigned int unknown_0;
        unsigned int offset_4;
        unsigned char originX_8;
        unsigned char originY_9;
        unsigned char width_a;
        unsigned char height_b;

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
        OBJBlitter::BlitRegion* blitRegions_0;
        OBJBlitter* blitter_4;
        RegionStructC* unknown_8;
        CharacterBuffers character_c;
        RegionStructC substruct_24;
        int objVramBase_30;
        int unknown_34;
        unsigned char x_38;
        unsigned char y_39;
        unsigned char width_3a;
        unsigned char height_3b;
        unsigned char numRegions_3c;
        unsigned char firstOAMEntry_3d;
        unsigned char engine_3e;
        unsigned char unknown_3f_0 : 1;
        unsigned char unknown_3f_1 : 1;

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

    unsigned char* buffer_0_;
    OBJGraphics* unknown_4_;
    char unknown_8_;
    BoundingRect textRects_a_[3];
    unsigned short colors_22_[2];
    char pad_26[2];
    ClipWindow clipWindow_28_;

public:
    void Initialize(int screen, int windowID);
    // loads to offset 0x1c0 + 2 * palettePosition
    void LoadColorToPalette(int palettePosition, int index);
    void SetColor(int index, unsigned short value);
    void ReloadDefaultColors();

    void SubmitToOAM();
    void ConfigureClipWindow();
    OBJGraphics* GetGraphics();
    unsigned int CreateInVRAM(unsigned int objVramSize);
    void ResetStateFlags();

    bool CreateYesNoBox(unsigned int objVramSize);
    void GetOptionPosition(short* outX, short* outY, int option);

    int GetTouchSelection();
    int GetPointSelection(short x, short y);

    void* BeginRender();
    void Clear();

    void DrawTextBoxBorder();
    void DrawText(int textID, int x, int y, const char* text,
        unsigned char colorIndex, unsigned char fontIndex);
    void FillRectangle(unsigned char colorIndex, short left, short top, short right, short bottom);

    void EndRender();
    void FreeStagingMemory();
};