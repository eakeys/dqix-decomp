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
    int unknown_8;
    int unknown_c;
    fix32_t scaleX;
    fix32_t scaleY;
    unsigned short color;
    char pad_1a[2];

    void Reset();
};