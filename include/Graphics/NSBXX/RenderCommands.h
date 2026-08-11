#pragma once

#include "../Vector.h"
#include "NSBXX.h"

struct RenderCommandHandler;

struct BoneMatrixRenderData
{
    union Scale
    {
        struct {
            fix32_t x; fix32_t y; fix32_t z;
        };
        fix32_t array[3];
    };
    // bit 0: set if no scaling
    // bit 1: set if no rotation
    // bit 2: set if no translation
    // if bit 3 is set, the bone matrix calculation callback treats scale_v1
    // as the identity, and if bit 4 is set, it treats scale_v2 as the identity
    unsigned int flags_;
    Scale scale_v0_;
    Scale scale_v1_;
    Scale scale_v2_;
    fix32_t rotationMatrix_[9]; // probably a struct
    Vector3fix translate_;
};

struct MaterialRenderData
{
    // bit 0: material does not have type 1 (scaling) extension data
    // bit 1: material does not have type 2 (rotation) extension data
    // bit 2: material does not have type 3 (translation) extension data
    // bit 3: material *has* any of type 1-3 extension data
    // bit 4: ???, ORed with bit 3 in some places, notably to call
    //        callback_f0 in the handler
    // bit 5: if set, alpha = 0 when drawing
    unsigned int flags_;
    unsigned int paramDIF_AMB_;
    unsigned int paramSPE_EMI_;
    unsigned int paramPOLYGON_ATTR_;
    unsigned int paramTEXIMAGE_PARAMS_;
    unsigned int texturePaletteBase_;
    fix32_t extensionScaleX_;
    fix32_t extensionScaleY_;
    fix16_t rotationSine_;
    fix16_t rotationCosine_;
    fix32_t translateX_;
    fix32_t translateY_;
    unsigned short materialWidth_;
    unsigned short materialHeight_;
    fix32_t materialxScale_;
    fix32_t materialyScale_;
};

struct AnimationData;

struct ModelRenderData
{
    unsigned int flags_0_;
    NSBXXInternalModel* internalModel_;
    AnimationData* dataPtr_8_; // some relation to M.AT objects
    int (*functionPtr_c_)(MaterialRenderData*, AnimationData*, unsigned int);
    AnimationData* dataPtr_10_; // something to do with J.AC animations
    // points to ProcessJointAnimationsOnBoneMatrix (020b3bac usa)
    int (*functionPtr_14_)(BoneMatrixRenderData*, AnimationData*, unsigned int);
    AnimationData* dataPtr_18_; // something to do with V.?? objects
    int (*functionPtr_1c_)(void*, AnimationData*, int);
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