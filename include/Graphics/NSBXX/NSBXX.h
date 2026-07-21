#pragma once

#include "std_library_functions.h"

// Documentation on the various formats is available here:
// https://github.com/scurest/nsbmd_docs/blob/master/nsbmd_docs.txt
// I'm using NSBXX as a generic name for any of these file types

#define SIGNATURE_NSBMD 
#define SIGNATURE_NSBTX 0x30585442 // "BTX0"
#define SIGNATURE_NSBCA
#define SIGNATURE_NSBTP
#define SIGNATURE_NSBTA
#define SIGNATURE_NSBMA
#define SIGNATURE_NSBVA

struct NSBXXContainer
{
    uint32_t signature_;
    uint16_t byteOrderMark_;
    uint16_t version_;
    uint32_t fileSize_;
    uint16_t headerSize_;
    uint16_t numSubfiles_;
};

struct NSBXXNameList
{
    uint8_t unknown_0_;
    uint8_t numEntries_;
    uint16_t totalSize_; // size in bytes of the entire list

    // subheader?
    uint16_t subheaderSize_;
    uint16_t offsetToDataStart_;
    uint32_t treeRoot_8_; // https://github.com/scurest/nsbmd_docs/issues/2
    // Supposedly the structure continues as follows:
    // uint32_t unknown_c[numEntries_];
    // <-- this + offsetToDataStart_ points to the following:
    // uint16_t elementStride_; // sizeof(T)
    // uint16_t dataSectionSize_; // includes elementStride and dataSectionSize, so is equal to numEntries * sizeof(T) + 4
    // T dataElements_[numEntries_];
    // Name names_[numEntries_]; // Name is an array of 16 chars
};

struct NSBXXInnerFileCommon {
    uint32_t signature;
    uint32_t fileSize;
    NSBXXNameList nameList;
};

struct NSBXXPatternAnimation
{
    uint8_t unk_0[4];
    uint16_t numFrames_;
    uint8_t numTextureNames_;
    uint8_t numPaletteNames_;
    uint16_t textureNamesOffset_;
    uint16_t paletteNamesOffset_;

    NSBXXNameList tracks_;

    struct Track
    {
        uint16_t numKeyframes_;
        uint16_t unk_2; // I thought keyframes was u32 but it's loaded as u16
        int16_t maybeSpeed_4_; // I imagine this is a 1.3.12 fixed point, it took values like 0x0555 (1/3)
        uint16_t keyframeArrayOffset_;

        struct Keyframe
        {
            uint16_t frameTime_;
            uint8_t textureIdx_;
            uint8_t paletteIdx_;
        };
    };
};

struct NSBXXTex
{
    uint32_t signature;
    uint32_t maybeTotalSize_4_;
    uint32_t unk_8;
    uint16_t block1LengthShr3_;
    uint16_t textureListOffset_;
    uint32_t unk_10;
    uint32_t block1Offset_;
    uint32_t unk_18_not_in_documentation; // apicula docs have everything after this 4 bytes earlier
    uint16_t block2LengthShr3_;
    uint16_t unk_1e;
    uint32_t unk_20;
    uint32_t block2Offset_;
    uint32_t block3Offset_;
    uint32_t unk_2c;
    uint16_t block4LengthShr3_;
    uint16_t unk_32;
    uint32_t paletteListOffset_; 
    uint32_t block4Offset_;
};

extern "C"
{
// usa: func_020b2e3c
int NSBXX_Tex_GetBlock1Length(NSBXXTex* tex);
// usa: func_020b2e50
int NSBXX_Tex_GetBlock2Length(NSBXXTex* tex);

// usa: func_020b2f30
int NSBXX_Tex_GetBlock4Length(NSBXXTex* tex);

// usa: func_020b7694
// Returns a pointer to the first file within the file provided.
// In practice, this mainly seems to be used for getting the MDL
// out of an NSBMD, though I've also seen a use for getting a TEX
// out of an NSBTX, so I'm giving it the more generic name for now.
void* NSBXX_GetFirstSubfile(NSBXXContainer* nsbxx);

// usa: func_020b76a4
// Returns a pointer to a .TEX file within the file provided.
// Assumes that the provided file either
//  - contains multiple files, of which the second is a TEX, or
//  - is an NSBTX containing only one file.
// From limited testing the first case is often satisfied
// by NSBMD files that contain one MDL and one TEX.
void* NSBXX_GetTEXFile(NSBXXContainer* nsbxx);

// Various subfiles consist of multiple objects (e.g. MDL files
// comprise multiple models, JNT files contain multiple animations)
// and the subfile has the following format:
// uint32_t signature; // e.g. "MDL0"
// uint32_t filesize; 
// NameListU32 offsetsToObjectWithin;
void* NSBXX_GetObjectFromFirstSubfile(NSBXXContainer* nsbxx, unsigned int idx);

// Might have other uses, but for now I've only seen it called on
// pattern animations.
const char* NSBXX_PatternAnimation_GetTextureName(NSBXXPatternAnimation* anim, unsigned int idx);
const char* NSBXX_PatternAnimation_GetPaletteName(NSBXXPatternAnimation* anim, unsigned int idx);
NSBXXPatternAnimation::Track::Keyframe* NSBXX_PatternAnimation_GetKeyframe(NSBXXPatternAnimation*, uint16_t track, uint16_t frameTime);
NSBXXPatternAnimation::Track* NSBXX_PatternAnimation_GetTrack(NSBXXPatternAnimation*, unsigned int track);
}