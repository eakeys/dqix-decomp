#pragma once

#include <std_library_functions.h>
#include "../Graphics/Vector.h"

// sizeof == 0x1c
struct TextBox
{
    short x;
    short y;
    short width;
    short height;
    // enum: 0 = destroyed, 1 = ready to create, 2 = exists, 3 = fading out?
    int status;
    int unknown_c;
    fix32_t scaleX;
    fix32_t scaleY;
    unsigned short color;
    char pad_1a[2];

    void Reset();
};