#pragma once

// sizeof(NarcHandleInitialPart) == 92.
// Used much more broadly but idk where atm
struct NarcHandleInitialPart
{
    // Signature: (this, dst, imageOffset, copyLength)
    typedef int(*PFNLoadFile)(NarcHandleInitialPart*, void*, unsigned int, unsigned int);
    // Signature: (this, src, imageOffset, copyLength)
    typedef int(*PFNSaveFile)(NarcHandleInitialPart*, const void*, unsigned int, unsigned int);

    char unknown_0[12];
    int unknown_0C;
    int unknown_10;
    int unknown_14;
    int unknown_18;
    char unknown_1C[12];
    void* pFileImage;
    int fatOffset_2C;
    unsigned int fatSize;
    int nameTableOffset_34;
    unsigned int nameTableSize;
    int fatOffset_3C;
    int nameTableOffset_40;
    int unknown_44;
    PFNLoadFile loadFileProc_48;
    PFNSaveFile saveFileProc;
    PFNLoadFile loadFileProc_50;
    char unknown_54[8];
};

struct FSStruct72
{
    char unknown_0[8];
    NarcHandleInitialPart* unknown_8;
    unsigned int flags;
    char unknown_10[0x14];
    unsigned int unknown_24;
    char unknown_28[0x20];
};

struct FSStruct76
{
    unsigned short unknown_0;
    unsigned short unknown_2;
    FSStruct72 inner;
};

