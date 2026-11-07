#pragma once

#include "ClipWindow.h"
#include <globaldefs.h>

// sizeof == 0x4c, I think.
// There are too many of these to keep track of, this name will have to do for now
class AnotherRenderer
{
public:
    struct Substruct_4
    {
        char unk_0[0x3e];
        unsigned char unknown_3e;
    };

    int unknown_0_;
    Substruct_4* unknown_4_;
    unsigned char unknown_8_;
    unsigned short unknown_array_a_[3][4];
    unsigned short colors_22_[2];
    char pad_26[2];
    ClipWindow clipWindow_28_;

public:
    void Initialize(int screen, int windowID);
    // loads to offset 0x1c0 + 2 * palettePosition
    void LoadColorToPalette(int palettePosition, int index);
    void SetColor(int index, unsigned short value);
    void ReloadDefaultColors();
};