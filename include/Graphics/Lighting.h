#pragma once

#include "../Memory/SafeAllocator.h"
#include "Vector.h"
#include "std_library_functions.h"

// I have no idea what any of this is. I thought it would be more fruitful
// but it's pretty much a dead end, I don't even see where this data gets 
// consumed. Disabling it causes weird lighting effects, that's all I know really

struct Vector3float
{
    float x;
    float y;
    float z;
};

struct LightInfo0205111c
{
    unsigned short unk_0;
    float unk_4;
    float unk_8;
    float unk_c;
    bool unk_10;
};

// sizeof == 0x30c
class LightingManager
{
public:
    union
    {
        // sizeof == 0xe8
        struct
        {
            Vector3float unk_0[7]; // is indexed with values going up to 5, but 7 fits perfectly
            float unk_54[7];
            float unk_70[7];
            short unk_8c[7];
            short unk_9a[7];
            short unk_a8[7];
            short unk_b6[7];
            short unk_c4[7];
            short unk_d2[7];
            char unk_e0[7];
            char padding_e7;
        } small_;
        // sizeof == 0x17c
        struct
        {
            LightInfo0205111c unk_0[7];
            LightInfo0205111c unk_8c[7];
            unsigned short unk_118[7];
            unsigned short unk_126[7];
            unsigned short unk_134[7];
            unsigned short unk_142[7];
            unsigned short unk_150[7];
            unsigned short unk_15e[7];
            unsigned short unk_16c[7];
            char padding_17a[2];
        } big_;
    };

    // sizeof == 0x38, populated by opcode 69
    struct Struct_17c
    {
        int unk_0;
        int unk_4;
        int unk_8;
        int unk_c;
        unsigned short unk_10;
        int unk_14;
        char buffer_18[0x20];
    };

    Struct_17c unk_17c[7];
    int maybeMode_; // 1 = use small struct, 2 = use big
    float unknown_308_;

    // Initialize() doesn't return *this, so it isn't a constructor, but
    // it's called by Zone3D constructor, so our constructor must be inline and call it
    LightingManager() { Initialize(); }

    void Initialize();
    void Reset();
    void LoadFromScript(const void* script, unsigned int length, SafeAllocator* alloc);
};