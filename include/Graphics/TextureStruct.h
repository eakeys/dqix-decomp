#pragma once

// I don't know if this is unique to textures, but that's the context
// I've found it in so I'm calling it TextureStruct for now.
// sizeof == 0xb4
struct TextureStruct
{
    char unk_0[0x5c];
    void* fileData_;
    unsigned int fileSize_;
    char unk_64[0xb4 - 0x64];
};