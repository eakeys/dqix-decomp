#pragma once

// sizeof == 0x20
// Describes a location in VRAM?
struct VRAMPointer
{
    char unk_0[0x1c];
    unsigned char engine; // 0 = main, 1 = sub
    unsigned char bgIndex; // 0 to 3
    char unk_1d[3];

    void SetPriority(int);
};