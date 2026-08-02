#pragma once

#include "../Vector.h"
#include "NSBXX.h"

struct RenderCommandHandler;

struct BoneMatrixRenderData
{
    unsigned int flags_;
    char unk_4[0x28 - 0x4];
    fix32_t rotationMatrix_[9]; // probably a struct
    fix32_t translateX_;
    fix32_t translateY_;
    fix32_t translateZ_;
};

struct MaterialRenderData
{
    unsigned int flags_;
    unsigned int paramDIF_AMB_;
    unsigned int paramSPE_EMI_;
    unsigned int paramPOLYGON_ATTR_;
    unsigned int paramTEXIMAGE_PARAMS_;
    unsigned int texturePaletteBase_;
    unsigned int unknown_18_;
    unsigned int unknown_1c_;
    unsigned short unknown_20_;
    unsigned short unknown_22_;
    unsigned int unknown_24_;
    unsigned int unknown_28_;
    unsigned short materialWidth_;
    unsigned short materialHeight_;
    fix32_t materialxScale_;
    fix32_t materialyScale_;
};

struct ModelRenderData
{
    struct MaybeAnimationData
    {
        char unk_0[0x10];
        MaybeAnimationData* pNext;
        char unk_14[5];
        unsigned char numEntries_;
        unsigned short entries[1]; // can reference past end
    };

    unsigned int flags_0_;
    NSBXXInternalModel* internalModel_;
    MaybeAnimationData* dataPtr_8_; // some relation to M.AT objects
    int (*functionPtr_c_)(MaterialRenderData*, MaybeAnimationData*, unsigned int);
    MaybeAnimationData* dataPtr_10_; // something to do with J.AC animations
    int (*functionPtr_14_)(BoneMatrixRenderData*, MaybeAnimationData*, unsigned int);
    MaybeAnimationData* dataPtr_18_; // something to do with V.?? objects
    int (*functionPtr_1c_)(void*, MaybeAnimationData*, int);
    void (*renderCommandHook_)(RenderCommandHandler*);
    unsigned char renderCommandHookCommandID_;
    unsigned char renderCommandHookStage_; // 1,2 or 3 depending on when in the function it gets called
    char padding_26[2];
    const void* functionPtr_28_;
    char unk_2c[4];
    uint8_t* renderCommandList_;
    BoneMatrixRenderData* boneMatrixRenderDataArray_;
    MaterialRenderData* materialRenderDataArray_;
    unsigned int bitfield_3c_[2]; // relates to dataPtr_8_
    unsigned int bitfield_44_[2]; // relates to dataPtr_10_
    unsigned int bitfield_4c_[2]; // relates to dataPtr_18_
};

void RenderModelFromRenderData(ModelRenderData* renderData);
// much more barebones implementation of rendering and just does a single
// mesh / material pair. In practice this seems to be used for shadows
// bindMaterial is really a bool
void RenderMeshWithMaterial(NSBXXInternalModel* model, unsigned int materialIdx, unsigned int meshIdx, int bindMaterial);