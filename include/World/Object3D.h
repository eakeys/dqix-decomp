#pragma once

#include "../Graphics/Model3D.h"
#include "../Graphics/Animation3D.h"
#include "../Resource/BCFG.h"

fix32_t GetAnimationFrameCountFix32(AnimationData*);

struct Object3D_ListEntryC
{
    unsigned int unknown_0_bit_0 : 1;
    unsigned int unknown_0_bits_1_8 : 8;
    BCFG bcfgData;
    struct Substruct
    {
        int maybeNumAnimations;
        Animation3D* animations;
    } *pSubstruct;
    Object3D_ListEntryC* pNext;
};

// Represents an instance of an object in the world that has a 3D model.
class Object3D
{
public:
    short unknown_0_;
    short unknown_2_;
    short unknown_4_;
    unsigned short unknown_6_;
    Model3D* pModel_;
    Object3D_ListEntryC* unknown_c_;
    Object3D_ListEntryC* unknown_10_;
    BCFG::AnimationEntry* unknown_14_; // get this form from func_020370a0
    char unknown_18_;
    unsigned char unknown_19_;
    unsigned char unk_1a[2];

    fix32_t animationTime_;
    fix32_t unknown_20_;
    int unknown_24_;
    int unknown_28_;

    struct Struct2C
    {
        Object3D_ListEntryC* unk_0;
        fix32_t unk_4;
        unsigned short unk_8;
        short unk_a;
        int unk_c;
    } unknown_struct_2c_;
    int unknown_3c_; // might be fix32_t, set to 200 (0.05?)

    unsigned char unknown_40_bit_0_ : 1;
    unsigned char unknown_40_bit_1_ : 1;
    unsigned char unknown_40_bit_2_ : 1;
    unsigned char alpha_40_ : 5;
    unsigned char alpha_41_ : 5;
    unsigned char drawSuccessful_ : 1;
    bool noTextures_;
    char padding_43;
    Vector3fix position_;
    // might not be a vector.
    // As a transformation x applies first, then y then z
    Vector3fix rotation_;
    fix16_t scale_[3];
    char padding_62[2];
    fix32_t unknown_64_; // might be int, but initialized to 0x1000
    fix32_t unknown_68_; // possibly part of a vector
    unsigned int flags_6c_;
    char unknown_struct_70_[8];

    unsigned short unknown_78_;
    unsigned short unknown_7a_;
    unsigned short unknown_7c_;
    unsigned short unknown_7e_;
    short unknown_80_;
    char padding_82[2];
    Vector3fix vec_84_;
    short unknown_90_;
    short unknown_92_;

    // this is a bool: if set, this object's position/rotation/scale will
    // be considered relative to the world matrix in the RenderConfig at the
    // start of drawing this. Possibly used for e.g. player body parts?
    int isRelative_;
    int unknown_98_;
    short unknown_9c_;
    char unknown_9e_;
    char padding_9f;

    fix16_t alphaScaleFactor_;
    char unk_a2[2];
    unsigned int unknown_a4_;
    int unknown_a8_;

public:
    void Initialize();

    // commits the currently active animation(s) at the current time for
    // subsequent draw call. if non-null argument is passed, the animations of
    // this object will be applied there instead of to itself
    void ApplyAnimations(Object3D* otherObj);
    void ApplyTextures();
    void PopulateRenderConfigWorld();

    // I don't know what these flags do, but it is a visibility check among other things
    bool IsVisibleAndAdjustFlags();

    // might need to be int return type, but functionally a bool
    bool Draw(bool applyClipping);

    // I haven't found this used in practice (there are call sites though).
    // Similar to the main Draw function but with much less config
    bool DrawSimple(bool applyClipping);

    // Used to draw the player's head & weapon in the equipment menu,
    // and in a few instances in overlay 23.
    // This version doesn't set the success flag or submit anything
    // to the FIFO
    bool DrawSimple2(bool applyClipping);

    // used in overlay 25
    bool DrawMeshWithMaterial(bool applyClipping, int material, int mesh, int bind);

    // used for drawing shadows under objects
    bool DrawMeshWithMaterialSimple(bool applyClipping, int material, int mesh, int bind);

    // used somewhere in overlay 15, not sure what for
    bool DrawSimple3(bool applyClipping);

    // one last draw function which draws at the origin with identity scaling

    void SendTransformToFifo();
    void SendTranslationToFifo();
    void SendRotationToFifo();
    void SendScalingToFifo();

    // A copy of the raw model file will be created, so you can load
    // it into a scratch buffer or similar just to pass it here
    void SetModelFromFileCopy(SafeAllocator* alloc, const void* modelFile,
        unsigned int modelLength, Model3D::TextureStagingMode stagingMode);

    // The raw model file provided will be used, and must persist until
    // this object is destroyed or has its model changed
    bool SetModelFromFile(SafeAllocator* alloc, void* modelFile,
        unsigned int modelLength, Model3D::TextureStagingMode stagingMode);

    Object3D_ListEntryC* CreateListEntryC(SafeAllocator* alloc);

    bool MaybeSetRegularAnimation(const char* animName, int flags);
    bool MaybeSetBCFGAnimation(int index, int flags);

    bool IsVisible() const;
    void MakeVisible();
    void MakeHidden();

    int GetCombinedAlpha() const;
};