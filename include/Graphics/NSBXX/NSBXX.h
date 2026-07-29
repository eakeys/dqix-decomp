#pragma once

#include "std_library_functions.h"
#include <globaldefs.h>
#include "../Vector.h"

// Documentation on the various formats is available here:
// https://github.com/scurest/nsbmd_docs/blob/master/nsbmd_docs.txt
// I'm using NSBXX as a generic name for any of these file types

#define SIGNATURE_NSBMD 0x30444d42 // "BMD0"
#define SIGNATURE_NSBTX 0x30585442 // "BTX0"
#define SIGNATURE_NSBCA 0x30414342 // "BCA0"
#define SIGNATURE_NSBTP 0x30505442 // "BTP0"
#define SIGNATURE_NSBTA 0x30415442 // "BTA0"
#define SIGNATURE_NSBMA 0x30414d42 // "BMA0"
#define SIGNATURE_NSBVA 0x30415642 // "BVA0"

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

    struct SearchTreeEntry // https://github.com/scurest/nsbmd_docs/issues/2
    {
        uint8_t bitIndex_;
        uint8_t children_[2];
        uint8_t resourceIndex_;
    } treeRoot_8_;
    // Supposedly the structure continues as follows:
    // SearchTreeEntry otherEntries_[numEntries_];
    // <-- this + offsetToDataStart_ points to the following:
    // uint16_t elementStride_; // holds sizeof(T)
    // uint16_t dataSectionSize_; // includes elementStride and dataSectionSize, so is equal to numEntries * sizeof(T) + 4
    // T dataElements_[numEntries_];
    // Name names_[numEntries_]; // Name is an array of 16 chars

    template<class T>
    inline T* GetEntryByIndex(unsigned int n) const volatile
    {
        if (this != NULL && n < numEntries_)
        {
            intptr_t dataStart = (intptr_t)this + offsetToDataStart_;
            uint16_t stride = *(uint16_t*)dataStart;
            return (T*)(dataStart + 4 + stride * n);
        }
        return NULL;
    }

    inline const char* GetNameByIndexAndOffset(unsigned int n, unsigned int offsetInBuffer) const
    {
        if (this != NULL && n < numEntries_)
        {
            intptr_t dataStart = (intptr_t)this + offsetToDataStart_;
            intptr_t nameStart = dataStart + *(uint16_t*)(dataStart + 2);
            return (const char*)(nameStart + offsetInBuffer);
        }
        return NULL;
    }
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

struct ModelBoundingBox
{
    // all quantities are 16-bit fixed points.
    // scurest nsbmd docs say last 3 are xMax etc but that's not how
    // they're used in functions here
    int16_t xMin_;
    int16_t yMin_;
    int16_t zMin_;
    int16_t xSize_;
    int16_t ySize_;
    int16_t zSize_;
};

struct NSBXXMaterial
{
    uint16_t unk_0;
    uint16_t size_; // in bytes
    uint32_t paramDIF_AMB_;
    uint32_t paramSPE_EMI_;
    uint32_t paramPOLYGON_ATTR_;
    uint32_t maskPOLYGON_ATTR_;
    uint32_t paramTEXIMAGE_PARAMS_;

    uint32_t unk_18;
    uint16_t texturePaletteVRAMOffset_;
    uint16_t unk_1e;
    uint16_t width_;
    uint16_t height_;
    fix32_t xScale_; // to be filled when linked to actual texture data
    fix32_t yScale_; // to be filled when linked to actual texture data
};

struct NSBXXModelMaterialData
{
    // offset to a NameList of MaterialPairing structs
    uint16_t texturePairingsOffset_;
    // offset to a NameList of MaterialPairing structs
    uint16_t palettePairingsOffset_;
    // entries are uint32_t's giving offsets to NSBXXMaterial structs
    NSBXXNameList materialOffsetList_;

    inline struct NSBXXMaterial* GetMaterialByIndex(unsigned int n) const
    {
        if (this != NULL)
        {
            uint32_t* materialOffset = materialOffsetList_.GetEntryByIndex<uint32_t>(n);
            
            if (materialOffset != NULL)
            {
                return (NSBXXMaterial*)((intptr_t)this + *materialOffset);
            }
        }
        return NULL;
    }
};

// Texture image pairing or texture palette pairing.
// No information about the image/palette is stored in this struct, instead
// you use the name corresponding to this entry in the relevant NameList
// pointed to by the offsets in NSBXXModelMaterialData.
struct NSBXXMaterialPairing
{
    // offset to an array of uint8_t giving indices of Materials that use this
    // image / palette. Offsets to these materials are stored in a NameList,
    // and the uint8_t values are indices in that list.
    uint16_t offsetToIndexArray_;
    uint8_t arraySize_;
    uint8_t flags_; // bit 0 = has TEX0 data bound/linked to it
};

struct NSBXXInternalModel
{
    uint32_t filesize_;
    uint32_t renderCommandsOffset_;
    uint32_t materialsOffset_;
    uint32_t meshesOffset_;
    uint32_t inverseBindsOffset_;
    uint8_t unk_14[3];
    uint8_t numBoneMatrices_;
    uint8_t numMaterials_;
    uint8_t numMeshes_;
    uint8_t unk_1a[2];
    int32_t upScale_; // 32-bit fixed point
    int32_t downScale_; // 32-bit fixed point
    uint16_t numVertices_;
    uint16_t numPolygons_;
    uint16_t numTriangles_;
    uint16_t numQuads_;
    ModelBoundingBox bounds_;
    int32_t maybeScale_;
    char unk_3c[4];
    NSBXXNameList boneList_;

    // using this seems to screw up register assignment, but it's here in case
    // we can make it work later somehow
    inline NSBXXModelMaterialData* GetMaterialData() const
    {
        if (this != NULL && materialsOffset_ != 0)
            return (NSBXXModelMaterialData*)((intptr_t)this + materialsOffset_);
        else
            return NULL;
    }
};

struct NSBXXMdl
{
    uint32_t signature_;
    uint32_t unknown_4;
    NSBXXNameList nameList_;

    NSBXXInternalModel* GetInternalModelByIndex(unsigned int n) const
    {
        NSBXXInternalModel* internalModel;
        if (this != NULL)
        {
            uint32_t* pModelOffset = nameList_.GetEntryByIndex<uint32_t>(n);

            if (pModelOffset != NULL)
            {
                internalModel = (NSBXXInternalModel*)((intptr_t)this + *pModelOffset);
                return internalModel;
            }
        }
        return NULL;
    }
};

struct NSBXXTex
{
    uint32_t signature;
    uint32_t maybeTotalSize_4_;
    // this is zero in the file but gets overwritten.
    // it actually contains a few things: low 16 bits are the offset
    // divided by eight. Then the next 15 bits encode the size divided by
    // sixteen, and the top bit is some bool.
    uint32_t block1VRAMLoadOffset_;
    uint16_t block1NumEightBytes_; // need to << 3 to get block size
    uint16_t textureListOffset_;
    uint16_t maybeBlock1Flags_10_; // bit 0: loaded to vram
    char padding_12[2];
    uint32_t block1Offset_;
    // see the block 1 analogue for a description
    uint32_t block2Or3VRAMLoadOffset_;
    uint16_t block2NumEightBytes_; // block 3 has as many 4-byte values, so << 3 and << 2 respectively
    uint16_t unk_1e;
    uint16_t maybeBlock23Flags_20_; // bit 0: loaded to vram
    char padding_22[2];
    uint32_t block2Offset_;
    uint32_t block3Offset_;
    // see the block 1 analogue for a description
    uint32_t block4VRAMLoadOffset_;
    uint16_t block4NumEightBytes_;
    uint16_t maybeBlock4Flags_32_; // bit 0: loaded to vram
    uint16_t paletteListOffset_; 
    uint16_t unk_36;
    uint32_t block4Offset_;

    // The documentation would suggest this is at a dynamic offset specified
    // by textureListOffset_, but some functions treat it as being in this fixed
    // location.
    NSBXXNameList textureList_;
};

struct NSBXXTexTexture
{
    uint32_t paramTEXIMAGE_PARAMS_;
    uint32_t unk_4;
};

struct NSBXXTexPalette
{
    uint16_t offsetWithinBlock4_; // need to left-shift by 3
    uint16_t unk_2;
};

extern "C"
{
// usa: func_020b2e3c
int NSBXX_Tex_GetBlock1Length(NSBXXTex* tex);
// usa: func_020b2e50
int NSBXX_Tex_GetBlock2Length(NSBXXTex* tex);
// usa: func_020b2e64
void NSBXX_Tex_WriteImageVRAMOffsets(NSBXXTex* tex, int block1, int block2_3);
// usa: func_020b28e78
void NSBXX_Tex_LoadImageToVRAM(NSBXXTex* tex, bool needsMapping);
// usa: func_020b2f30
int NSBXX_Tex_GetBlock4Length(NSBXXTex* tex);
// usa: func_020b2f44
void NSBXX_Tex_WritePaletteVRAMOffset(NSBXXTex* tex, int offset);
// usa: func_020b2f4c
void NSBXX_Tex_LoadPaletteToVRAM(NSBXXTex* tex, bool needsMapping);

// usa: func_020b3184
bool NSBXX_AttachTextureImageToModel(NSBXXInternalModel* model, NSBXXTex* tex0);
// usa: func_020b3284
void NSBXX_DetachTextureImageFromModel(NSBXXInternalModel* model);
// usa: func_020b33f0
bool NSBXX_AttachTexturePaletteToModel(NSBXXInternalModel* model, NSBXXTex* tex0);
// usa: func_020b34f8
void NSBXX_DetachTexturePaletteFromModel(NSBXXInternalModel* model);
// usa: func_020b357c
int NSBXX_LinkTEX0ToMDL0(NSBXXMdl* mdl0, NSBXXTex* tex0);
// usa: func_020b362c
void NSBXX_UnlinkTEX0FromMDL0(NSBXXMdl* mdl0);

// usa: func_020b7184
// The alpha value can be between 0-31.
// (This function should become static later)
void NSBXX_Model_SetMaterialAlpha(NSBXXInternalModel* model, unsigned int materialIndex, int alpha);

// usa: func_020b732c
// The alpha value can be between 0-31.
void NSBXX_Model_SetAlpha(NSBXXInternalModel* model, int alpha);

// usa: func_020b736c
// Gets the entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
void* NSBXXNameList_Search(NSBXXNameList* nameList, const char* name);

// usa: func_020b752c
// Gets the index of an entry in a NameList given its string.
// The string passed should be a 16-byte buffer padded with zeros, a regular
// null-terminated string is not sufficient.
int NSBXXNameList_SearchIndex(NSBXXNameList* nameList, const char* name);

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