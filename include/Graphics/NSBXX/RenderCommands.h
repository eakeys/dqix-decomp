#pragma once

#include "../Vector.h"
#include "NSBXX.h"

struct RenderCommandHandler;
typedef void (*RenderCommandHook)(RenderCommandHandler*);

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

struct ModelRenderContext
{
    // bits 0, 1, 2, 3 set flags 7, 8, 9, 10 in RenderCommandHandler
    // bit 4: animation bitfields dirty
    unsigned int flags_;
    NSBXXInternalModel* internalModel_;
    AnimationData* materialAnimations_; // M.AM, M.AT and M.PT animations
    // points to ProcessMaterialAnimationsOnBoundMaterial (020b3ad0 usa)
    bool (*pfnProcessMaterialAnimations_)(MaterialRenderData*, AnimationData*, unsigned int);
    AnimationData* jointAnimations_; // J.AC animations
    // points to ProcessJointAnimationsOnBoneMatrix (020b3bac usa)
    bool (*pfnProcessJointAnimations_)(BoneMatrixRenderData*, AnimationData*, unsigned int);
    AnimationData* visibilityAnimations_; // V.AV animations (never used?)
    // points to ProcessVisibilityAnimations (020b3f98 usa)
    bool (*pfnProcessVisibilityAnimations_)(int*, AnimationData*, unsigned int);
    void (*renderCommandHook_)(RenderCommandHandler*);
    unsigned char renderCommandHookCommandID_;
    unsigned char renderCommandHookStage_; // 1,2 or 3 depending on when in the function it gets called
    char padding_26[2];
    void (*preRenderCallback_)(RenderCommandHandler*);
    char unk_2c[4];
    uint8_t* renderCommandList_;
    BoneMatrixRenderData* boneMatrixRenderDataArray_;
    MaterialRenderData* materialRenderDataArray_;
    // packed array of bools, where the k'th entry is given by the
    // (k & 0x1f)'th bit of array[k >> 5]. The index refers to a material
    // (matching the NSBXXInternalModel's MaterialList) or bone matrix (not 
    // sure about visibility animations), and pfnProcess...Animations will
    // only run for a particular object if its bit is set here.
    unsigned int animatedMaterials_[2];
    unsigned int animatedBoneMatrices_[2];
    unsigned int animatedVisibilityConditions_[2];
};

void RenderModelFromRenderData(ModelRenderContext* renderData);
// much more barebones implementation of rendering and just does a single
// mesh / material pair. In practice this seems to be used for shadows
// bindMaterial is really a bool
void RenderMeshWithMaterial(NSBXXInternalModel* model, unsigned int materialIdx, unsigned int meshIdx, int bindMaterial);