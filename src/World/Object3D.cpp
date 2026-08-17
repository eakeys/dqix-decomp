#include "World/Object3D.h"
#include "Graphics/NSBXX/RenderConfig.h"
#include "System/Graphics.h"
#include "Combat/Main/BattleList.h"

extern "C"
{
    void* func_020100bc(BattleStruct*);
    // update world matrix rotation
    void func_02016d8c(const fix32_t* rotation);

    const fix32_t* func_0202ed74(void*);

    fix32_t func_02030c68(fix32_t); // sin(x) from lookup table, 0 <= x <= 2*pi
    fix32_t func_02030c9c(fix32_t); // cos(x) from lookup table, 0 <= x <= 2*pi
    // component-wise multiply vectors and store in 3rd argument
    void func_02030e88(const Vector3fix*, const Vector3fix*, Vector3fix*);

    void func_020311f0(fix32_t); // send x-rotation to fifo
    void func_02031234(fix32_t); // send y-rotation to fifo
    void func_02031278(fix32_t); // send z-rotation to fifo

    void func_02034d04(void*);

    void func_02036380(Object3D_ListEntryC*);

    void func_02037674(Object3D*);

    void func_02037934(Object3D*, Object3D*);
    bool func_02037a64(Object3D*);

    // write 3x3 identity
    void func_020c1180(fix32_t*);
    // write 3x3 x-rotation matrix, y-rotation matrix, z-rotation matrix
    void func_020c1264(fix32_t*, fix32_t sine, fix32_t cosine);
    void func_020c1280(fix32_t*, fix32_t sine, fix32_t cosine);
    void func_020c129c(fix32_t*, fix32_t sine, fix32_t cosine);

    // multiply 3x3 matrices a*b
    void func_020c15a4(const fix32_t* a, const fix32_t* b, fix32_t* out);
    // add vector3 a+b, store in 3rd argument
    void func_020c2d90(const Vector3fix*, const Vector3fix*, Vector3fix* out);

    // copy 3x3 matrix
    void func_020ca528(const fix32_t*, fix32_t*);
}

// if set, drawing doesn't take place. Also does something with
// collision detection against monsters
#define OBJECT3D_FLAG_0 0
#define OBJECT3D_FLAG_1 1
#define OBJECT3D_FLAG_2 2
#define OBJECT3D_FLAG_3 3
#define OBJECT3D_FLAG_4 4
#define OBJECT3D_FLAG_5 5
#define OBJECT3D_FLAG_6 6
#define OBJECT3D_FLAG_7 7
#define OBJECT3D_FLAG_8 8
#define OBJECT3D_FLAG_9 9
#define OBJECT3D_FLAG_10 10
// never clip this object
#define OBJECT3D_FLAG_11 11
#define OBJECT3D_FLAG_12 12
#define OBJECT3D_FLAG_13 13
// if set, then we bypass the RenderConfig (i.e. no view matrix)
#define OBJECT3D_FLAG_14 14
// if set, then the fifo is up to date
#define OBJECT3D_FLAG_15 15
#define OBJECT3D_FLAG_16 16
#define OBJECT3D_FLAG_17 17
#define OBJECT3D_FLAG_18 18
#define OBJECT3D_FLAG_19 19
// if flag 20 is set, flag 21 will be toggled with each draw call
#define OBJECT3D_FLAG_20 20
// if flag 20 is set, this will toggle with each draw call, and when set
// to true nothing will draw
#define OBJECT3D_FLAG_21 21
#define OBJECT3D_FLAG_22 22
#define OBJECT3D_FLAG_23 23
#define OBJECT3D_FLAG_24 24
#define OBJECT3D_FLAG_25 25
#define OBJECT3D_FLAG_26 26
#define OBJECT3D_FLAG_27 27
#define OBJECT3D_FLAG_28 28
// don't use own alpha, just stick with model's intrinsic/pre-set value
#define OBJECT3D_FLAG_29 29
#define OBJECT3D_FLAG_30 30
#define OBJECT3D_FLAG_31 31

struct Struct_020efaa8
{
    float unk_0[4]; // holds 30.0f, 30.0f, 180.0f, 180.0f
    char materialAnimExtension[6];
    char paletteAnimExtension[6];
    char textureAnimExtension[6];
    char jointAnimExtension[6];

    void (*rotationFunctionsRelative[3])(fix32_t*, fix32_t, fix32_t);
    void (*rotationFunctionsAbsolute[3])(fix32_t*, fix32_t, fix32_t);
} extern data_020efaa8;

void Object3D::Initialize()
{
    unknown_0_ = 1;
    unknown_2_ = -1;
    unknown_4_ = -1;
    unknown_6_ = -1;
    unknown_18_ = -1;
    unknown_19_ = 0;
    animationTime_ = 0;
    unknown_20_ = 0;
    unknown_40_bit_0_ = 0;
    unknown_40_bit_1_ = 0;
    pModel_ = NULL;
    alpha_40_ = 31;
    alpha_41_ = 31;
    noTextures_ = false;
    flags_6c_ = 0;
    memset(unknown_struct_70_, 0, sizeof(unknown_struct_70_));
    unknown_64_ = 1 << 12;
    unknown_68_ = 1 << 12;
    unknown_10_ = 0;
    unknown_78_ = 0xffff;
    unknown_7a_ = 0xffff;
    unknown_7c_ = 1 << 12;
    unknown_7e_ = 1 << 12;
    unknown_80_ = 0;
    unknown_c_ = NULL;
    position_.x = 0;
    position_.y = 0;
    position_.z = 0;
    rotation_.x = 0;
    rotation_.y = 0;
    rotation_.z = 0;
    scale_[0] = 1 << 12;
    scale_[1] = 1 << 12;
    scale_[2] = 1 << 12;
    vec_84_.x = 0;
    vec_84_.y = 0;
    vec_84_.z = 0;
    unknown_90_ = -1;
    func_02034d04(&unknown_struct_2c_);
    unknown_3c_ = 200;
    isRelative_ = false;
    unknown_98_ = 0;
    unknown_92_ = -1;
    unknown_9c_ = 0;
    unknown_14_ = 0;
    unknown_a4_ = 0;
    alphaScaleFactor_ = 1 << 12;
    unknown_9e_ = 0;
    unknown_a8_ = 0;
    drawSuccessful_ = false;
}

fix32_t GetAnimationFrameCountFix32(AnimationData* data)
{
    // bit ugly but we have to go into the raw data for this
    return *(uint16_t*)((intptr_t)data->pRawData_ + 4) << 12;
}

void Object3D::ApplyAnimations(Object3D* otherObj)
{
    Model3D* model = pModel_;
    if (otherObj != NULL)
        model = otherObj->pModel_;
    if (model == NULL || unknown_10_ == NULL)
        return;

    if (unknown_10_->unknown_0_bit_0 == 0)
    {
        model->RemoveAnimations();
        Object3D_ListEntryC::Substruct* animSet = unknown_10_->pSubstruct;
        model->AddAnimation(&animSet->animations[0]);
        model->AddAnimation(&animSet->animations[1]);
        model->AddAnimation(&animSet->animations[3]);
        model->AddAnimation(&animSet->animations[2]);
        animSet->animations[0].SetAnimationTime(animationTime_);
        animSet->animations[1].SetAnimationTime(animationTime_);
        animSet->animations[3].SetAnimationTime(animationTime_);
        animSet->animations[2].SetAnimationTime(animationTime_);
    }
    else if (unknown_10_->unknown_0_bit_0 == 1)
    {
        model->RemoveAnimations();
        fix32_t weight = 1 << 12;
        if (unknown_struct_2c_.unk_0 != NULL)
        {
            Object3D_ListEntryC::Substruct* animSet = &unknown_struct_2c_.unk_0->pSubstruct[unknown_struct_2c_.unk_8];
            for (unsigned int i = 0; i < animSet->maybeNumAnimations; i++)
            {
                model->AddAnimation(&animSet->animations[i]);
                animSet->animations[i].SetAnimationTime(unknown_struct_2c_.unk_4);
                fix32_t thisWeight = unknown_struct_2c_.unk_a;
                animSet->animations[i].GetBasicAnimationData()->weight_ = thisWeight;
            }
            weight -= unknown_struct_2c_.unk_a;
        }
        Object3D_ListEntryC::Substruct* animSet = &unknown_10_->pSubstruct[unknown_18_];
        for (unsigned int i = 0; i < animSet->maybeNumAnimations; i++)
        {
            model->AddAnimation(&animSet->animations[i]);
            animSet->animations[i].SetAnimationTime(animationTime_);
            animSet->animations[i].GetBasicAnimationData()->weight_ = weight;
        }
    }
}

void Object3D::ApplyTextures()
{
    if ((flags_6c_ & (1 << OBJECT3D_FLAG_3)) && pModel_ != NULL)
    {
        if (!noTextures_)
            pModel_->ApplyTexturesFromModel(pModel_);
        else
            pModel_->RemoveTextures();
    }
}

void CreateRotationX(fix32_t* out, fix32_t s, fix32_t c)
{
    func_020c1264(out, s, c);
}

void CreateRotationY(fix32_t* out, fix32_t s, fix32_t c)
{
    func_020c1280(out, s, c);
}

void CreateRotationZ(fix32_t* out, fix32_t s, fix32_t c)
{
    func_020c129c(out, s, c);
}

void Object3D::PopulateRenderConfigWorld()
{
    if (isRelative_ ? 1 : 0)
    {
        fix32_t worldRotation[9];
        func_020ca528(&data_0210a010.objectRotation[0], &worldRotation[0]);
        fix32_t axisRotation[9];
        fix32_t rotationComponents[3];
        rotationComponents[0] = rotation_.z;
        rotationComponents[1] = rotation_.y;
        rotationComponents[2] = rotation_.x;
        if (flags_6c_ & (1 << OBJECT3D_FLAG_25))
        {
            data_020efaa8.rotationFunctionsRelative[0] = &CreateRotationX;
            fix32_t foo = rotationComponents[0];
            rotationComponents[0] = rotationComponents[2];
            rotationComponents[2] = foo;
            data_020efaa8.rotationFunctionsRelative[2] = &CreateRotationZ;
        }
        for (int i = 0; i < 3; i++)
        {
            fix32_t amount = rotationComponents[i];
            if (amount != 0)
            {
                fix32_t sine = func_02030c68(amount);
                fix32_t cosine = func_02030c9c(amount);
                data_020efaa8.rotationFunctionsRelative[i](&axisRotation[0], sine, cosine);
                func_020c15a4(&worldRotation[0], &axisRotation[0], &worldRotation[0]);
            }
        }
        func_02016d8c(&worldRotation[0]);
        Vector3fix multipliedScale;
        Vector3fix ownScale;
        ownScale.x = scale_[0];
        ownScale.y = scale_[1];
        ownScale.z = scale_[2];
        func_02030e88(&data_0210a010.objectScale, &ownScale, &multipliedScale);
        RenderConfig::SetObjectScale(&multipliedScale);
        Vector3fix multipliedPosition;
        func_02030e88(&position_, &multipliedScale, &multipliedPosition);
        func_020c2d90(&data_0210a010.objectPosition, &multipliedPosition, &multipliedPosition);
        RenderConfig::SetObjectPosition(&multipliedPosition);
    }
    else
    {
        RenderConfig::SetObjectPosition(&position_);
        if (flags_6c_ & (1 << OBJECT3D_FLAG_19))
        {
            const fix32_t* rotation = func_0202ed74(func_020100bc(GetBattleStruct()));
            func_020ca528(&rotation[0], &data_0210a010.objectRotation[0]);
            data_0210a010.flags &= ~((1 << RENDER_CONFIG_FLAG_WORLDVIEW_CACHE_VALID) | (1 << RENDER_CONFIG_FLAG_5) | (1 << RENDER_CONFIG_FLAG_2));
        }
        else
        {
            fix32_t axisRotation[9];
            fix32_t totalRotation[9];
            func_020c1180(&totalRotation[0]);
            fix32_t rotationComponents[3];
            rotationComponents[0] = rotation_.z;
            rotationComponents[1] = rotation_.y;
            rotationComponents[2] = rotation_.x;
            if (flags_6c_ & (1 << OBJECT3D_FLAG_25))
            {
                data_020efaa8.rotationFunctionsAbsolute[0] = &CreateRotationX;
                fix32_t foo = rotationComponents[0];
                rotationComponents[0] = rotationComponents[2];
                rotationComponents[2] = foo;
                data_020efaa8.rotationFunctionsAbsolute[2] = &CreateRotationZ;
            }
            for (int i = 0; i < 3; i++)
            {
                fix32_t amount = rotationComponents[i];
                if (amount != 0)
                {
                    fix32_t sine = func_02030c68(amount);
                    fix32_t cosine = func_02030c9c(amount);
                    data_020efaa8.rotationFunctionsAbsolute[i](&axisRotation[0], sine, cosine);
                    func_020c15a4(&totalRotation[0], &axisRotation[0], &totalRotation[0]);
                }
            }
            func_02016d8c(&totalRotation[0]);
        }
        Vector3fix scaling;
        scaling.x = scale_[0];
        scaling.y = scale_[1];
        scaling.z = scale_[2];
        RenderConfig::SetObjectScale(&scaling);
    }
}

bool Object3D::IsVisibleAndAdjustFlags()
{
    if (pModel_ == NULL)
        return false;

    unsigned char alpha = GetCombinedAlpha();
    if (alpha == 0)
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_0))
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_20))
    {
        if (flags_6c_ & (1 << OBJECT3D_FLAG_21))
        {
            flags_6c_ &= ~(1 << OBJECT3D_FLAG_21);
            return false;
        }
        else
            flags_6c_ |= (1 << OBJECT3D_FLAG_21);
    }
    return true;
}

bool Object3D::Draw(bool applyClipping)
{
    drawSuccessful_ = false;
    flags_6c_ &= ~(1 << OBJECT3D_FLAG_28);

    if (!IsVisibleAndAdjustFlags())
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_11))
        applyClipping = false;

    if (!(flags_6c_ & (1 << OBJECT3D_FLAG_13)))
        ApplyAnimations(NULL);

    int prior = GetCombinedAlpha();
    int blendedAlpha = (alphaScaleFactor_ / 4096.0f) * prior;
    if (blendedAlpha != pModel_->GetAlpha() && !(flags_6c_ & (1 << OBJECT3D_FLAG_29)))
        pModel_->SetAlpha(blendedAlpha);

    ApplyTextures();
    if (!(flags_6c_ & (1 << OBJECT3D_FLAG_15)))
    {
        if (flags_6c_ & (1 << OBJECT3D_FLAG_14))
        {
            SendTranslationToFifo();
            SendRotationToFifo();
            SendScalingToFifo();
        }
        else
        {
            PopulateRenderConfigWorld();
            RenderConfig::SubmitToFifo();
        }
    }

    if (unknown_a4_ != 0)
    {
        NSBXXTex* tex0 = pModel_->GetTEX0();
        if (tex0 != NULL)
        {
            NSBXX_Tex_WritePaletteVRAMOffset(tex0, unknown_a4_);
            NSBXX_DetachTexturePaletteFromModel(pModel_->rawInternalModel_);
            NSBXX_AttachTexturePaletteToModel(pModel_->rawInternalModel_, tex0);
        }
    }

    bool success;
    if (flags_6c_ & (1 << OBJECT3D_FLAG_13))
    {
        success = pModel_->DrawMeshWithMaterial(applyClipping, 0, 0, true);
    }
    else
    {
        success = pModel_->Draw(applyClipping);
    }

    drawSuccessful_ = (int)success;
    if (success)
    {
        if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
            func_02037674(this);
        if (!(flags_6c_ & (1 << OBJECT3D_FLAG_17)) && func_02037a64(this))
            func_02037934(this, this);
        flags_6c_ |= (1 << OBJECT3D_FLAG_28);
    }
    else if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
    {
        vec_84_ = position_;
        vec_84_.y = position_.y + (unknown_68_ / 2);
    }
    return success;
}

bool Object3D::DrawSimple(bool applyClipping)
{
    drawSuccessful_ = false;

    if (!IsVisibleAndAdjustFlags())
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_11))
        applyClipping = false;

    ApplyAnimations(NULL);
    unsigned char ownAlpha = GetCombinedAlpha();
    if (ownAlpha != pModel_->GetAlpha())
        pModel_->SetAlpha(ownAlpha);

    ApplyTextures();
    RenderConfig::SubmitToFifo();

    bool success = pModel_->Draw(applyClipping);
    drawSuccessful_ = (int)success;

    if (success)
    {
        if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
            func_02037674(this);
        if (func_02037a64(this))
            func_02037934(this, this);
    }
    else if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
    {
        vec_84_ = position_;
        vec_84_.y = position_.y + (unknown_68_ / 2);
    }
    return success;
}

bool Object3D::DrawSimple2(bool applyClipping)
{
    if (!IsVisibleAndAdjustFlags())
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_11))
        applyClipping = false;

    ApplyAnimations(NULL);
    unsigned char ownAlpha = GetCombinedAlpha();
    if (ownAlpha != pModel_->GetAlpha())
        pModel_->SetAlpha(ownAlpha);

    ApplyTextures();
    return pModel_->Draw(applyClipping);
}

bool Object3D::DrawMeshWithMaterial(bool applyClipping, int material, int mesh, int bind)
{
    if (!IsVisibleAndAdjustFlags())
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_11))
        applyClipping = false;

    ApplyTextures();
    PopulateRenderConfigWorld();
    RenderConfig::SubmitToFifo();
    return pModel_->DrawMeshWithMaterial(applyClipping, material, mesh, bind);
}

bool Object3D::DrawMeshWithMaterialSimple(bool applyClipping, int material, int mesh, int bind)
{
    if (!IsVisibleAndAdjustFlags())
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_11))
        applyClipping = false;

    ApplyTextures();
    return pModel_->DrawMeshWithMaterial(applyClipping, material, mesh, bind);
}

bool Object3D::DrawSimple3(bool applyClipping)
{
    if (pModel_ == NULL)
        return false;

    unsigned char ownAlpha = GetCombinedAlpha();
    if (ownAlpha == 0)
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_0))
        return false;

    if (flags_6c_ & (1 << OBJECT3D_FLAG_11))
        applyClipping = false;

    ApplyAnimations(NULL);
    if (ownAlpha != pModel_->GetAlpha())
        pModel_->SetAlpha(ownAlpha);
    ApplyTextures();
    SendTranslationToFifo();
    SendRotationToFifo();
    SendScalingToFifo();
    return pModel_->Draw(applyClipping);
}

void Object3D::SendTransformToFifo()
{
    SendTranslationToFifo();
    SendRotationToFifo();
    SendScalingToFifo();
}

void Object3D::SendTranslationToFifo()
{
    fix32_t z = position_.z;
    fix32_t y = position_.y;
    fix32_t x = position_.x;
    GXFIFO_MATRIX_TRANSLATE = x;
    GXFIFO_MATRIX_TRANSLATE = y;
    GXFIFO_MATRIX_TRANSLATE = z;
}

void Object3D::SendRotationToFifo()
{
    if (rotation_.z != 0)
        func_02031278(rotation_.z);
    if (rotation_.y != 0)
        func_02031234(rotation_.y);
    if (rotation_.x != 0)
        func_020311f0(rotation_.x);
}

void Object3D::SendScalingToFifo()
{
    fix32_t z = scale_[2];
    fix32_t y = scale_[1];
    fix32_t x = scale_[0];
    GXFIFO_MATRIX_SCALE = x;
    GXFIFO_MATRIX_SCALE = y;
    GXFIFO_MATRIX_SCALE = z;
}

void Object3D::SetModelFromFileCopy(SafeAllocator* alloc, const void* modelFile,
    unsigned int modelLength, Model3D::TextureStagingMode stagingMode)
{
    if (alloc == NULL || modelFile == NULL)
        return;

    pModel_ = (Model3D*)alloc->Allocate(sizeof(Model3D));
    pModel_->Clear();
    pModel_->CopyAndProcessRawFile(&alloc->allocUnion, modelFile, modelLength, stagingMode);
}

bool Object3D::SetModelFromFile(SafeAllocator* alloc, void* modelFile,
    unsigned int modelLength, Model3D::TextureStagingMode stagingMode)
{
    pModel_ = (Model3D*)alloc->Allocate(sizeof(Model3D));
    if (pModel_ == NULL)
        return false;
    pModel_->Clear();
    pModel_->SetAndProcessRawFile(modelFile, modelLength, stagingMode);
    return true;
}

Object3D_ListEntryC *Object3D::CreateListEntryC(SafeAllocator *alloc)
{
    Object3D_ListEntryC* entry = (Object3D_ListEntryC*)alloc->Allocate(sizeof(Object3D_ListEntryC));
    if (entry == NULL)
        return NULL;
    func_02036380(entry);
    entry->pSubstruct = (Object3D_ListEntryC::Substruct*)alloc->Allocate(sizeof(Object3D_ListEntryC::Substruct));
    if (entry->pSubstruct == NULL)
        return NULL;

    entry->pSubstruct->maybeNumAnimations = 4;
    entry->pSubstruct->animations = (Animation3D*)alloc->Allocate(4 * sizeof(Animation3D));
    if (entry->pSubstruct->animations == NULL)
        return NULL;

    for (int i = 0; i < 4; i++)
        entry->pSubstruct->animations[i].DefaultInitialize();
    return entry;
}

bool Object3D::MaybeSetRegularAnimation(const char *animName, int flags)
{
    if (pModel_ == NULL || animName == NULL || (flags_6c_ & (1 << OBJECT3D_FLAG_22)) || animName[0] == '\0')
        return false;
    
    Object3D_ListEntryC* loopEntry;
    BCFG* bcfg = NULL;
    int index = -1;
    for (loopEntry = unknown_c_; loopEntry != NULL; loopEntry = loopEntry->pNext)
    {
        Object3D_ListEntryC::Substruct* substruct = loopEntry->pSubstruct;
        if (substruct == NULL)
            continue;
        bcfg = &loopEntry->bcfgData;
        if (bcfg == NULL)
            continue;
        index = bcfg->SearchAnimationByName(animName);
        if (index < 0)
            continue;
        if (loopEntry->unknown_0_bit_0 != 1 || loopEntry->pSubstruct[index].animations != NULL)
            break;
        index = -1;
    }

    if (index < 0)
        return false;

    if (!(flags & 8) && loopEntry == unknown_10_ && index == unknown_18_)
    {
        if ((flags & 4) && (unknown_19_ & 4))
            return true;
        if (!(flags & 4) && !(unknown_19_ & 4))
            return true;
    }

    if (loopEntry->unknown_0_bit_0 == 0)
    {
        BCFG::AnimationEntry* bcfgAnim = bcfg->GetAnimationEntry(index);
        fix32_t time;
        if (flags & 4)
            time = bcfgAnim->unknown_vector.y;
        else
            time = bcfgAnim->unknown_vector.x;
        unknown_18_ = index;
        animationTime_ = time;
        unknown_20_ = time;
        unknown_19_ = flags;
        unknown_24_ = 0;
        unknown_28_ = 0;
        unknown_40_bit_1_ = true;
        unknown_40_bit_0_ = false;
        unknown_40_bit_2_ = false;
        unknown_14_ = bcfgAnim;
        unknown_10_ = loopEntry;
    }
    else if (loopEntry->unknown_0_bit_0 == 1)
    {
        BCFG::AnimationEntry* bcfgAnim = bcfg->GetAnimationEntry(index);
        Animation3D* anim3d = loopEntry->pSubstruct[index].animations;
        fix32_t time;
        if (!(flags & 4))
            time = 0;
        else
        {
            time = GetAnimationFrameCountFix32(anim3d->GetBasicAnimationData()) - 0x1000;
        }
        if (flags & 0x10)
        {
            if (unknown_struct_2c_.unk_c != 0)
            {
                func_02034d04(&unknown_struct_2c_);
            }
            else
            {
                func_02034d04(&unknown_struct_2c_);
                unknown_struct_2c_.unk_0 = unknown_10_;
                unknown_struct_2c_.unk_8 = unknown_18_;
                unknown_struct_2c_.unk_4 = animationTime_;
                unknown_struct_2c_.unk_a = 0x1000;
                unknown_struct_2c_.unk_c = unknown_3c_;
            }
        }
        unknown_18_ = index;
        animationTime_ = time;
        unknown_20_ = time;
        unknown_19_ = flags;
        unknown_24_ = 0;
        unknown_28_ = 0;
        unknown_40_bit_1_ = true;
        unknown_40_bit_0_ = false;
        unknown_40_bit_2_ = false;
        unknown_14_ = bcfgAnim;
        unknown_10_ = loopEntry;
        if (unknown_struct_2c_.unk_0 == loopEntry && unknown_struct_2c_.unk_8 == unknown_18_)
            func_02034d04(&unknown_struct_2c_);
    }

    flags_6c_ &= ~(1 << OBJECT3D_FLAG_18);
    return true;
}

bool Object3D::MaybeSetBCFGAnimation(int index, int flags)
{
    if (unknown_c_ == NULL)
        return false;

    if (pModel_ == NULL)
        return false;

    BCFG::AnimationEntry* entry = unknown_c_->bcfgData.GetAnimationEntry(index);
    if (entry == NULL)
        return false;
    
    fix32_t time;
    if (flags & 4)
        time = entry->unknown_vector.y;
    else
        time = entry->unknown_vector.x;
    
    unknown_18_ = index;
    animationTime_ = time;
    unknown_20_ = time;
    unknown_19_ = flags;
    unknown_24_ = unknown_28_ = 0;
    unknown_40_bit_1_ = true;
    unknown_14_ = entry;
    unknown_10_ = unknown_c_;
    flags_6c_ &= ~(1 << OBJECT3D_FLAG_18);

    return true;
}

bool Object3D::IsVisible() const
{
    return !(flags_6c_ & (1 << OBJECT3D_FLAG_0));
}

void Object3D::MakeVisible()
{
    flags_6c_ &= ~(1 << OBJECT3D_FLAG_0);
}

void Object3D::MakeHidden()
{
    flags_6c_ |= (1 << OBJECT3D_FLAG_0);
}

int Object3D::GetCombinedAlpha() const
{
    return (alpha_40_ * alpha_41_) / 31;
}