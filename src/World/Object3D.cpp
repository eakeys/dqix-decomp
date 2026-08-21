#include "World/Object3D.h"
#include "Graphics/NSBXX/RenderConfig.h"
#include "System/Graphics.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include "Graphics/NSBXX/GeometryFifo.h"
#include "Filesystem/NarcHandle.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Resource/ResourceMutex.h"
#include "Graphics/VRAMStaging.h"

ModelRenderContext* GetModel3DContext(Model3D* model);
void AnimationPackageListInsert(AnimationPackage** pListStart, AnimationPackage* entry);

extern "C"
{
    // zero memory
    void func_0200f374(void*, unsigned);

    void* func_020100bc(BattleStruct*);

    // deltaTime for animation blending
    fix32_t func_02010208(BattleStruct*);
    // deltaTime for model animations
    fix32_t func_02010218(BattleStruct*);
    // get some kind of deltaTime
    int func_02010220(BattleStruct*);
    // update world matrix rotation
    void func_02016d8c(const fix32_t* rotation);

    const fix32_t* func_0202ed74(void*);

    fix32_t func_02030c68(fix32_t); // sin(x) from lookup table, 0 <= x <= 2*pi
    fix32_t func_02030c9c(fix32_t); // cos(x) from lookup table, 0 <= x <= 2*pi
    // multiply vector by scalar and store in third argument
    void func_02030e2c(const Vector3fix*, fix32_t, Vector3fix*);
    // component-wise multiply vectors and store in 3rd argument
    void func_02030e88(const Vector3fix*, const Vector3fix*, Vector3fix*);

    void func_020311f0(fix32_t); // send x-rotation to fifo
    void func_02031234(fix32_t); // send y-rotation to fifo
    void func_02031278(fix32_t); // send z-rotation to fifo

    // based on testing, this is responsible for playing sound effects
    void func_0203ac40(void*, int, int, int);

    // write 3x3 identity
    void func_020c1180(fix32_t*);
    // write 3x3 x-rotation matrix, y-rotation matrix, z-rotation matrix
    void func_020c1264(fix32_t*, fix32_t sine, fix32_t cosine);
    void func_020c1280(fix32_t*, fix32_t sine, fix32_t cosine);
    void func_020c129c(fix32_t*, fix32_t sine, fix32_t cosine);

    // multiply 3x3 matrices a*b
    void func_020c15a4(const fix32_t* a, const fix32_t* b, fix32_t* out);
    // set 4x3 matrix to identity
    void func_020c1840(fix32_t* mat);
    // multiply 4x3 matrices a*b
    void func_020c1d60(const fix32_t* a, const fix32_t* b, fix32_t* out);
    // synchronous fix32 division
    fix32_t func_020c2bf4(fix32_t num, fix32_t denom);
    // add vector3 a+b, store in 3rd argument
    void func_020c2d90(const Vector3fix*, const Vector3fix*, Vector3fix* out);

    // copy 3x3 matrix
    void func_020ca528(const fix32_t*, fix32_t*);

    // compute a checksum for a string
    void func_020d1d1c(unsigned short* out, const void* data, unsigned int length);

    // memcpy and flush cache
    void func_020d8524(void*, const void*, unsigned);
}

// if set, drawing doesn't take place. Also does something with
// collision detection against monsters
#define OBJECT3D_FLAG_0 0
#define OBJECT3D_FLAG_1 1
// has a shadow?
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
// if set, drawing is more basic: no animations applied and use DrawMeshWithMaterial
// with material 0 and mesh 0
#define OBJECT3D_FLAG_13 13
// if set, then we bypass the RenderConfig (i.e. no view matrix)
#define OBJECT3D_FLAG_14 14
// if set, then the fifo is up to date
#define OBJECT3D_FLAG_15 15
#define OBJECT3D_FLAG_16 16
#define OBJECT3D_FLAG_17 17
#define OBJECT3D_FLAG_18 18
#define OBJECT3D_FLAG_19 19
// flickering effect
#define OBJECT3D_FLAG_20 20
// if flickering is enabled, this flips every frame
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

extern Vector3fix data_020e7800;

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

// Extremely bizarre setup for dynamic dispatch in animation advancing
extern Object3D::AnimationAdvanceProc data_020efae8[2];

// points to the four extension names in the above struct
extern const char* data_020efaf8[4];

extern char data_020efb08[]; // "damage"
extern char data_020efb0f[]; // "ARC"
extern char data_020efb13[]; // ".nsbmd"
extern char data_020efb1a[]; // ".nsbca"
extern char data_020efb21[]; // ".nsbma"
extern char data_020efb28[]; // ".nsbta"
extern char data_020efb2f[]; // ".nsbtp"
extern char data_020efb36[]; // ".bcfg"
extern char data_020efb3c[]; // "%s.%s"

struct Struct_02104b18
{
    char unk_0[0x14];
    Object3D::TrackedBoneMatrix* boneMatrixList;
    fix32_t boneMatrixWithView[12];
} extern data_02104b18;

extern char data_02108760[];

void Object3D::Initialize()
{
    unknown_0_ = 1;
    unknown_2_ = -1;
    unknown_4_ = -1;
    unknown_6_ = -1;
    activeAnimationIndex_ = -1;
    animationFlags_ = 0;
    animationTime_ = 0;
    prevFrameAnimationTime_ = 0;
    animationEnded_ = 0;
    unknown_40_bit_1_ = 0;
    pModel_ = NULL;
    inheritedAlpha_ = 31;
    ownAlpha_ = 31;
    noTextureTimer_ = 0;
    flags_6c_ = 0;
    memset(&alphaTransition_, 0, sizeof(AlphaTween));
    radius_ = 1 << 12;
    height_ = 1 << 12;
    activeAnimationPackage_ = 0;
    unknown_78_ = 0xffff;
    unknown_7a_ = 0xffff;
    animationPlaybackSpeed_ = 1 << 12;
    targetAnimationPlaybackSpeed_ = 1 << 12;
    animationPlaybackSpeedTransitionRemainingTicks_ = 0;
    loadedAnimationPackageList_ = NULL;
    position_.x = 0;
    position_.y = 0;
    position_.z = 0;
    rotation_.x = 0;
    rotation_.y = 0;
    rotation_.z = 0;
    scale_[0] = 1 << 12;
    scale_[1] = 1 << 12;
    scale_[2] = 1 << 12;
    maybeShadowPosition_.x = 0;
    maybeShadowPosition_.y = 0;
    maybeShadowPosition_.z = 0;
    shadowBoneIndex_ = -1;
    priorAnimationBlend_.Clear();
    defaultAnimBlendDuration_ = 200;
    pPrevChild_ = NULL;
    pNextChild_ = NULL;
    attachmentBoneIndex_ = -1;
    unknown_9c_ = 0;
    activeAnimationRecord_ = 0;
    alternativeTexturePaletteOffset_ = 0;
    alphaScaleFactor_ = 1 << 12;
    unknown_9e_ = 0;
    trackedBoneMatrixList_ = NULL;
    drawSuccessful_ = false;
}

void Object3D::AnimationBlend::Clear()
{
    source = NULL;
    sourceTime = 0;
    unk_8 = 0;
    blendWeight = 0;
    blendTimeRemaining = 0;
}

void Object3D::AdvanceEffects()
{
    if (flags_6c_ & (1 << OBJECT3D_FLAG_5))
        return;

    int deltaTimeTicks = func_02010208(GetBattleStruct());
    if ((0.0f != alphaTransition_.changePerTick) ? 1 : 0)
    {
        unsigned int inheritedAlphau16 = 65535.0f * (inheritedAlpha_ / 31.0f);
        alphaTransition_.current = inheritedAlphau16;
        inheritedAlpha_ = (char)alphaTransition_.Advance(deltaTimeTicks);
    }

    if (flags_6c_ & (1 << OBJECT3D_FLAG_3))
    {
        if (deltaTimeTicks < noTextureTimer_)
            noTextureTimer_ -= deltaTimeTicks;
        else
            noTextureTimer_ = 0;
    }

    if (animationPlaybackSpeedTransitionRemainingTicks_ != 0)
    {
        if (deltaTimeTicks < animationPlaybackSpeedTransitionRemainingTicks_)
        {
            // subtracting deltaTime and then dividing by the new value, isn't
            // this a bug? we divide by 0 in the last frame
            animationPlaybackSpeedTransitionRemainingTicks_ -= deltaTimeTicks;
            fix32_t rateOfChange = func_020c2bf4(
                targetAnimationPlaybackSpeed_ - animationPlaybackSpeed_,
                animationPlaybackSpeedTransitionRemainingTicks_ << 12);
            animationPlaybackSpeed_ += deltaTimeTicks * rateOfChange;
        }
        else
        {
            animationPlaybackSpeed_ = targetAnimationPlaybackSpeed_;
            animationPlaybackSpeedTransitionRemainingTicks_ = 0;
        }
    }

    AdvanceAnimations();
}

void Object3D::AdvanceAnimations()
{
    (void)func_02010220(GetBattleStruct());
    if (activeAnimationPackage_ == NULL)
        return;
    (this->*data_020efae8[activeAnimationPackage_->animationType])();
}

void Object3D::AdvanceAnimations_v0()
{
    fix16_t deltaTime = func_02010218(GetBattleStruct());
    BCFG* activeBCFG = &activeAnimationPackage_->bcfgData;
    if (activeBCFG == NULL || activeAnimationIndex_ < 0 || (flags_6c_ & (1 << OBJECT3D_FLAG_12)))
        return;

    animationEnded_ = false;
    animationReachedEnd_ = false;
    BCFG::AnimationRecord* record = activeBCFG->GetAnimationRecord(activeAnimationIndex_);
    if (record == NULL)
        return;
    bool shouldAdvanceTimer = true;
    if ((flags_6c_ & (1 << OBJECT3D_FLAG_18)) && (activeAnimationRecord_ == NULL ||
        strcmp(activeAnimationRecord_->name, data_020efb08) != 0))
        shouldAdvanceTimer = false;

    fix32_t timer = animationTime_;
    prevFrameAnimationTime_ = timer;
    if (!unknown_40_bit_1_)
    {
        if (!(animationFlags_ & OBJECT_ANIMATION_REVERSE))
        {
            fix32_t product = FIX32_MULTIPLY(record->frameRate, deltaTime);
            product = FIX32_MULTIPLY(product, animationPlaybackSpeed_);
            fix32_t startTime = record->startTime;
            fix32_t endTime = record->endTime;
            if (shouldAdvanceTimer)
                timer += product;
            if (record->endTime <= timer)
            {
                if (animationFlags_ & OBJECT_ANIMATION_DONT_LOOP)
                {
                    animationEnded_ = true;
                    timer = endTime;
                }
                else
                    timer = startTime + (timer - endTime);
                animationReachedEnd_ = true;
            }

        }
        else
        {
            fix32_t product = FIX32_MULTIPLY(record->frameRate, deltaTime);
            product = FIX32_MULTIPLY(product, animationPlaybackSpeed_);
            fix32_t endTime = record->endTime;
            fix32_t startTime = record->startTime;
            if (shouldAdvanceTimer)
                timer -= product;
            if (timer <= startTime)
            {
                fix32_t newTime;
                if (animationFlags_ & OBJECT_ANIMATION_DONT_LOOP)
                {
                    animationEnded_ = true;
                    timer = startTime;
                }
                else
                    timer = endTime - (startTime - timer);
                animationReachedEnd_ = true;
            }
        }
        prevFrameNormalizedAnimationTime_ = normalizedAnimationTime_;
        normalizedAnimationTime_ = func_020c2bf4(
            timer - record->startTime,
            record->endTime - record->startTime
        );
        animationTime_ = timer;
    }
    else
        unknown_40_bit_1_ = false;

    if ((flags_6c_ & (1 << OBJECT3D_FLAG_6)) || unknown_78_ == 0xffff)
        return;
    
    for (BCFG::AltSizeCEntry* loopEntry = record->firstAlt; loopEntry != NULL; loopEntry = loopEntry->pNext)
    {
        if (loopEntry->triggerTime == 0)
        {
            if (unknown_40_bit_1_ || (animationReachedEnd_ && !(animationFlags_ & OBJECT_ANIMATION_DONT_LOOP)))
                func_0203ac40(data_02108760, unknown_7a_, loopEntry->maybeSoundEffect, 0);
        }
        else if (loopEntry->triggerTime == 1.0) // why use a double?!
        {
            if (animationEnded_)
                func_0203ac40(data_02108760, unknown_7a_, loopEntry->maybeSoundEffect, 0);
        }  
        else if (prevFrameNormalizedAnimationTime_ < loopEntry->triggerTime && loopEntry->triggerTime <= normalizedAnimationTime_)
            func_0203ac40(data_02108760, unknown_7a_, loopEntry->maybeSoundEffect, 0);
    }
}

void Object3D::AdvanceAnimations_v1()
{
    BattleStruct* battle = GetBattleStruct();
    fix16_t deltaTime = func_02010218(battle);
    if (activeAnimationIndex_ < 0 || activeAnimationPackage_->pAnim3Ds == NULL 
        || activeAnimationPackage_->pAnim3Ds[activeAnimationIndex_].data == NULL)
        return;
    Animation3D* anim3D = activeAnimationPackage_->pAnim3Ds[activeAnimationIndex_].data;
    if (anim3D->GetBasicAnimationData() == NULL || (flags_6c_ & (1 << OBJECT3D_FLAG_12)))
        return;
    
    BCFG::AnimationRecord* record = activeAnimationPackage_->bcfgData.GetAnimationRecord(activeAnimationIndex_);
    if (record == NULL)
        return;
    if (priorAnimationBlend_.blendTimeRemaining > 0)
    {
        unsigned int blendDeltaTime = func_02010208(battle);
        unsigned int newTimeRemaining;
        if (priorAnimationBlend_.blendTimeRemaining < blendDeltaTime)
        {
            priorAnimationBlend_.Clear();
            newTimeRemaining = 0;
        }
        else
        {
            float dt = blendDeltaTime;
            float blendWeightCurrent = priorAnimationBlend_.blendWeight / 4096.0f;
            float dw_dt = blendWeightCurrent / priorAnimationBlend_.blendTimeRemaining; // constant
            fix32_t deltaWeight = 4096.0f * (dt * dw_dt);
            priorAnimationBlend_.blendWeight -= deltaWeight;
            newTimeRemaining = priorAnimationBlend_.blendTimeRemaining - blendDeltaTime;
        }
        priorAnimationBlend_.blendTimeRemaining = newTimeRemaining;
    }
    else
    {
        animationEnded_ = false;
        animationReachedEnd_ = false;
        fix32_t timer = animationTime_;
        bool shouldAdvanceTimer = true;
        if ((flags_6c_ & (1 << OBJECT3D_FLAG_18)) && (activeAnimationRecord_ == NULL ||
            strcmp(activeAnimationRecord_->name, data_020efb08) != 0))
            shouldAdvanceTimer = false;
        prevFrameAnimationTime_ = timer;
        if (!unknown_40_bit_1_)
        {
            if (!(animationFlags_ & OBJECT_ANIMATION_REVERSE))
            {
                fix32_t duration = GetAnimationFrameCountFix32(anim3D->GetBasicAnimationData()) - 0x1000;
                fix32_t playbackDelta = FIX32_MULTIPLY(record->frameRate, deltaTime);
                playbackDelta = FIX32_MULTIPLY(playbackDelta, animationPlaybackSpeed_);
                if (shouldAdvanceTimer)
                    timer += playbackDelta;
                if (duration <= timer)
                {
                    if (animationFlags_ & OBJECT_ANIMATION_DONT_LOOP)
                    {
                        animationEnded_ = true;
                        timer = duration;
                    }
                    else
                        timer -= duration;
                    animationReachedEnd_ = true;
                }
            }
            else
            {
                fix32_t duration = GetAnimationFrameCountFix32(anim3D->GetBasicAnimationData()) - 0x1000;
                fix32_t playbackDelta = FIX32_MULTIPLY(record->frameRate, deltaTime);
                playbackDelta = FIX32_MULTIPLY(playbackDelta, animationPlaybackSpeed_);
                if (shouldAdvanceTimer)
                    timer -= playbackDelta;
                if (timer <= 0)
                {
                    if (animationFlags_ & OBJECT_ANIMATION_DONT_LOOP)
                    {
                        animationEnded_ = true;
                        timer = 0;
                    }
                    else
                        timer += duration;
                    animationReachedEnd_ = true;
                }
            }
            fix32_t duration = GetAnimationFrameCountFix32(anim3D->GetBasicAnimationData()) - 0x1000;
            prevFrameNormalizedAnimationTime_ = normalizedAnimationTime_;
            normalizedAnimationTime_ = func_020c2bf4(timer, duration);
            animationTime_ = timer;
        }
        else
            unknown_40_bit_1_ = false;

        if ((flags_6c_ & (1 << OBJECT3D_FLAG_6)) || unknown_78_ == 0xffff)
            return;
        
        for (BCFG::AltSizeCEntry* loopEntry = record->firstAlt; loopEntry != NULL; loopEntry = loopEntry->pNext)
        {
            if (loopEntry->triggerTime == 0)
            {
                if (unknown_40_bit_1_ || (animationReachedEnd_ && !(animationFlags_ & OBJECT_ANIMATION_DONT_LOOP)))
                    func_0203ac40(data_02108760, unknown_7a_, loopEntry->maybeSoundEffect, 0);
            }
            else if (loopEntry->triggerTime == 1.0) // why use a double?!
            {
                if (animationEnded_)
                    func_0203ac40(data_02108760, unknown_7a_, loopEntry->maybeSoundEffect, 0);
            }  
            else if (prevFrameNormalizedAnimationTime_ < loopEntry->triggerTime && loopEntry->triggerTime <= normalizedAnimationTime_)
                func_0203ac40(data_02108760, unknown_7a_, loopEntry->maybeSoundEffect, 0);
        }
    }
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
    if (model == NULL || activeAnimationPackage_ == NULL)
        return;

    if (activeAnimationPackage_->animationType == 0)
    {
        model->RemoveAnimations();
        AnimationPackage::Reference* animSet = activeAnimationPackage_->pAnim3Ds;
        model->AddAnimation(&animSet->data[0]);
        model->AddAnimation(&animSet->data[1]);
        model->AddAnimation(&animSet->data[3]);
        model->AddAnimation(&animSet->data[2]);
        animSet->data[0].SetAnimationTime(animationTime_);
        animSet->data[1].SetAnimationTime(animationTime_);
        animSet->data[3].SetAnimationTime(animationTime_);
        animSet->data[2].SetAnimationTime(animationTime_);
    }
    else if (activeAnimationPackage_->animationType == 1)
    {
        model->RemoveAnimations();
        fix32_t weight = 1 << 12;
        if (priorAnimationBlend_.source != NULL)
        {
            AnimationPackage::Reference* anim = &priorAnimationBlend_.source->pAnim3Ds[priorAnimationBlend_.unk_8];
            // Why bother with this loop? as animationType == 1, anim->data is
            // a single entry and not an array - everywhere else treats it as such
            for (unsigned int i = 0; i < anim->count; i++)
            {
                model->AddAnimation(&anim->data[i]);
                anim->data[i].SetAnimationTime(priorAnimationBlend_.sourceTime);
                fix32_t thisWeight = priorAnimationBlend_.blendWeight;
                anim->data[i].GetBasicAnimationData()->weight_ = thisWeight;
            }
            weight -= priorAnimationBlend_.blendWeight;
        }
        AnimationPackage::Reference* anim = &activeAnimationPackage_->pAnim3Ds[activeAnimationIndex_];
        for (unsigned int i = 0; i < anim->count; i++)
        {
            model->AddAnimation(&anim->data[i]);
            anim->data[i].SetAnimationTime(animationTime_);
            anim->data[i].GetBasicAnimationData()->weight_ = weight;
        }
    }
}

void Object3D::ApplyTextures()
{
    if ((flags_6c_ & (1 << OBJECT3D_FLAG_3)) && pModel_ != NULL)
    {
        if (noTextureTimer_ == 0)
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
    if (pPrevChild_ ? 1 : 0)
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

    if (alternativeTexturePaletteOffset_ != 0)
    {
        NSBXXTex* tex0 = pModel_->GetTEX0();
        if (tex0 != NULL)
        {
            NSBXX_Tex_WritePaletteVRAMOffset(tex0, alternativeTexturePaletteOffset_);
            NSBXX_DetachTexturePaletteFromModel(pModel_->rawInternalModel_);
            NSBXX_AttachTexturePaletteToModel(pModel_->rawInternalModel_, tex0);
        }
    }

    bool success;
    if (flags_6c_ & (1 << OBJECT3D_FLAG_13))
        success = pModel_->DrawMeshWithMaterial(applyClipping, 0, 0, true);
    else
        success = pModel_->Draw(applyClipping);

    drawSuccessful_ = (int)success;
    if (success)
    {
        if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
            ComputeShadowPosition();
        if (!(flags_6c_ & (1 << OBJECT3D_FLAG_17)) && IsParent())
            DrawChildren(this);
        flags_6c_ |= (1 << OBJECT3D_FLAG_28);
    }
    else if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
    {
        maybeShadowPosition_ = position_;
        maybeShadowPosition_.y = position_.y + (height_ / 2);
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
            ComputeShadowPosition();
        if (IsParent())
            DrawChildren(this);
    }
    else if (flags_6c_ & (1 << OBJECT3D_FLAG_2))
    {
        maybeShadowPosition_ = position_;
        maybeShadowPosition_.y = position_.y + (height_ / 2);
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

void Object3D::MaybeUpdateBonePositions()
{
    if (pModel_ == NULL)
        return;
    flags_6c_ &= ~(1 << OBJECT3D_FLAG_5);
    AdvanceEffects();
    flags_6c_ |= (1 << OBJECT3D_FLAG_5);
    ApplyAnimations(NULL);
    Vector3fix position /*= { 0, 0, 0 } */;
    func_0200f374(&position, sizeof(Vector3fix));
    RenderConfig::SetObjectPosition(&position);
    fix32_t cosine = func_02030c9c(rotation_.y);
    fix32_t sine = func_02030c68(rotation_.y);

    fix32_t rotationMatrix[9];
    func_020c1280(&rotationMatrix[0], sine, cosine);
    func_02016d8c(&rotationMatrix[0]);
    Vector3fix scale = data_020e7800;
    RenderConfig::SetObjectScale(&scale);
    RenderConfig::SubmitToFifo();

    data_02104b18.boneMatrixList = trackedBoneMatrixList_;
    pModel_->Draw(false);
    data_02104b18.boneMatrixList = NULL;
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

void Object3D::LoadType0AnimationFromFile(int slot, const char* filename, SafeAllocator* alloc) 
{
    if (filename != NULL)
    {
        BackgroundLoader::AddLockGlobal();
        unsigned int length;
        if (LoadFileIntoMemory(filename, data_0211e33c, &length))
        {
            unsigned int lengthAgain = length;
            void* allocation = alloc->Allocate(lengthAgain);
            if (allocation != NULL)
            {
                memcpy(allocation, data_0211e33c, lengthAgain);
                LoadType0AnimationFromPersistentMemory(slot, alloc, allocation, lengthAgain);
            }
        }
        BackgroundLoader::RemoveLockGlobal();
    }
}

void Object3D::LoadType0AnimationFromFileInMemory(int slot, SafeAllocator* alloc, const void* data, unsigned int len)
{
    void* copy = alloc->Allocate(len);
    if (copy != NULL)
    {
        memcpy(copy, data, len);
        LoadType0AnimationFromPersistentMemory(slot, alloc, copy, len);
    }
}

void Object3D::LoadType0AnimationFromPersistentMemory(int slot, SafeAllocator* alloc, void* rawData, unsigned int len)
{
    if (pModel_ == NULL || alloc == NULL || rawData == NULL)
        return;

    if (loadedAnimationPackageList_ == NULL)
    {
        loadedAnimationPackageList_ = CreateType0AnimationPackage(alloc);
        if (loadedAnimationPackageList_ == NULL)
            return;
    }
    Animation3D* anim3d = &loadedAnimationPackageList_->pAnim3Ds->data[slot];
    StopCurrentAnimation();
    anim3d->DefaultInitialize();
    if (!anim3d->SetRawFile((NSBXXContainer*)rawData))
        return;
    if (!anim3d->CreateData(pModel_, alloc, NULL))
        return;
}

void Object3D::LoadType0AnimationPackageFromBCFGScript(const char *filename, SafeAllocator *alloc)
{
    BackgroundLoader::AddLockGlobal();
    unsigned int length;
    const void* data = LoadFileIntoMemory(filename, data_0211e33c, &length);
    if (data != NULL)
        LoadType0AnimationPackageFromBCFGScript(alloc, data, length);
    BackgroundLoader::RemoveLockGlobal();
}

AnimationPackage* Object3D::CreateType0AnimationPackage(SafeAllocator* alloc)
{
    AnimationPackage* package = (AnimationPackage*)alloc->Allocate(sizeof(AnimationPackage));
    if (package == NULL)
        return NULL;
    package->Reset(); // package is type 0 by default
    package->pAnim3Ds = (AnimationPackage::Reference*)alloc->Allocate(sizeof(AnimationPackage::Reference));
    if (package->pAnim3Ds == NULL)
        return NULL;

    package->pAnim3Ds->count = 4;
    package->pAnim3Ds->data = (Animation3D*)alloc->Allocate(4 * sizeof(Animation3D));
    if (package->pAnim3Ds->data == NULL)
        return NULL;

    for (int i = 0; i < 4; i++)
        package->pAnim3Ds->data[i].DefaultInitialize();
    return package;
}

void AnimationPackage::Reset()
{
    animationType = 0;
    packageID = 0;
    pNext = NULL;
    pAnim3Ds = NULL;
    bcfgData.Reset();
}

void Object3D::LoadType0AnimationPackageFromBCFGScript(SafeAllocator *alloc, const void *script, unsigned int len)
{
    if (alloc == NULL || script == NULL)
        return;
    if (loadedAnimationPackageList_ == NULL)
        loadedAnimationPackageList_ = CreateType0AnimationPackage(alloc);

    AnimationPackage* package = loadedAnimationPackageList_;
    if (package == NULL)
        return;
    package->bcfgData.Reset();
    package->bcfgData.LoadFromScript(alloc, script, len);
}

bool Object3D::LoadFromCHRArchive(ObjectArchiveLoadInfo* loadInfo)
{
    LockResourceMutex();
    bool success = InternalLoadFromCHRArchive(loadInfo);
    UnlockResourceMutex();
    return success;
}

bool Object3D::InternalLoadFromCHRArchive(ObjectArchiveLoadInfo* loadInfo)
{
    if (loadInfo->fileData == NULL)
        return false;
    NarcHandle narc;
    if (!narc.Initialize(data_020efb0f, (const unsigned char*)loadInfo->fileData))
        return false;
    NitroVM machine;
    SafeAllocator* allocator = loadInfo->allocator;
    NitroVM_Initialize(&machine);
    // Search the archive for an nsbmd file
    unsigned int fileID;
    for (fileID = 0; PrepareReadFileInNARCByID(&machine, &narc, fileID); fileID++)
    {
        char filename[80];
        NitroVM_WriteOutFilePath(&machine, filename, sizeof(filename));
        if (strstr(filename, data_020efb13))
        {
            unsigned int allocSize;
            const void* fileBytes = narc.GetFileByIndex(fileID);
            unsigned int fileLength = machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;

            if (loadInfo->unk_10 != 0)
            {
                allocSize = fileLength;
                if (allocSize & 3)
                    allocSize += (4 - (fileLength & 3));
                void* allocation = allocator->Allocate(allocSize);
                if (allocation == NULL)
                {
                    NitroVM_FinishRead(&machine);
                    narc.Destroy();
                    return false;
                }
                func_020d8524(allocation, fileBytes, allocSize);
                fileBytes = allocation;
            }
            // the pointer is either allocated by us or we're promised it'll stick 
            // around as long as we need it, so casting away const is just scary
            // rather than actually dangerous
            if (!SetModelFromFile(allocator, const_cast<void*>(fileBytes), fileLength, Model3D::TextureStagingMode_Normal))
            {
                NitroVM_FinishRead(&machine);
                narc.Destroy();
                return false;
            }
            NitroVM_FinishRead(&machine);
            break;
        }
        NitroVM_FinishRead(&machine);
    }
    unsigned int fileLength;
    const void* fileBytes;
    AnimationPackage* animPackage = CreateType0AnimationPackage(allocator);
    if (animPackage == NULL)
    {
        NitroVM_FinishRead(&machine);
        narc.Destroy();
        return false;
    }
    
    animPackage->packageID = loadInfo->packageID;
     
    for (fileID = 0; PrepareReadFileInNARCByID(&machine, &narc, fileID); fileID++)
    {
        char filename[80];
        NitroVM_WriteOutFilePath(&machine, filename, sizeof(filename));
        
        int fileType = -1;
        // check for various animation file types
        if (strstr(filename, data_020efb1a))
            fileType = 0;
        else if (strstr(filename, data_020efb21))
            fileType = 1;
        else if (strstr(filename, data_020efb28))
            fileType = 3;
        else if (strstr(filename, data_020efb2f))
            fileType = 2;
        
        if (fileType != -1)
        {
            fileBytes = narc.GetFileByIndex(fileID);
            fileLength = machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;
            if (loadInfo->unk_10 != 0)
            {
                // no faffing with alignment?!
                void* allocation = allocator->Allocate(fileLength);
                if (allocation == NULL)
                {
                    NitroVM_FinishRead(&machine);
                    narc.Destroy();
                    return false;
                }
                func_020d8524(allocation, fileBytes, fileLength);
                fileBytes = allocation;
            }
            Animation3D* anim3D = &animPackage->pAnim3Ds->data[fileType];
            StopCurrentAnimation();
            anim3D->DefaultInitialize();
            // see comments on earlier const casts for why this is not totally evil
            if (!anim3D->SetRawFile(const_cast<NSBXXContainer*>((const NSBXXContainer*)fileBytes)))
            {
                NitroVM_FinishRead(&machine);
                narc.Destroy();
                return false;
            }
            if (!anim3D->CreateData(pModel_, allocator, NULL))
            {
                NitroVM_FinishRead(&machine);
                narc.Destroy();
                return false;
            }
        }
        else if (strstr(filename, data_020efb36))
        {
            const void* scriptData = narc.GetFileByIndex(fileID);
            unsigned int fileLength = machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;
            animPackage->bcfgData.Reset();
            animPackage->bcfgData.LoadFromScript(allocator, scriptData, fileLength);
        }
        NitroVM_FinishRead(&machine);
    }
    narc.Destroy();
    AnimationPackageListInsert(&loadedAnimationPackageList_, animPackage);
    return true;
}

bool Object3D::LoadFromCCHROrCMOTArchive(ObjectArchiveLoadInfo *loadInfo, int (*animCallback)(BCFG::AnimationRecord*))
{
    SafeAllocator* allocator = loadInfo->allocator;
    
    unsigned int fileLength;
    const void* fileBytes;
    if (FindFilesInNarcBySubstring(loadInfo->fileData, data_020efb13, &fileBytes, &fileLength, 1) != 0)
    {
        if (loadInfo->unk_10 != 0)
        {
            void* allocation;
            const void* source = fileBytes;
            unsigned int allocSize = (fileLength + 3) & ~3;
            allocation = allocator->Allocate(allocSize);
            if (allocation == NULL)
                allocation = NULL;
            else
                memcpy(allocation, source, allocSize);
            fileBytes = allocation;
            if (allocation == NULL)
                return false;
        }
        if (!SetModelFromFile(allocator, const_cast<void*>(fileBytes), fileLength, Model3D::TextureStagingMode_Normal))
            return false;
    }

    AnimationPackage* package = (AnimationPackage*)allocator->Allocate(sizeof(AnimationPackage));
    if (package == NULL)
        return false;
    package->Reset();
    package->animationType = 1;
    package->packageID = loadInfo->packageID;
    if (FindFilesInNarcBySubstring(loadInfo->fileData, data_020efb36, &fileBytes, &fileLength, 1) != 0)
    {
        package->bcfgData.Reset();
        package->bcfgData.LoadFromScript(allocator, fileBytes, fileLength);
    }

    const void* narcBuffer = loadInfo->fileData;
    LockResourceMutex();
    struct ChecksumLookup
    {
        int numEntries;
        struct Entry {
            unsigned short checksum;
            unsigned short length;
        } entries[80];
        bool valid;
    } lookup;
    lookup.valid = false;
    char candidateName[80];
    char filename[80];
    NitroVM machine;
    NarcHandle narc;
    if (narc.Initialize(data_020efb0f, (const unsigned char*)narcBuffer))
    {
        NitroVM_Initialize(&machine);
        lookup.valid = true;
        ChecksumLookup::Entry* entry;
        int idx = 0;
        for (idx = 0; PrepareReadFileInNARCByID(&machine, &narc, idx); idx++)
        {
            if (idx >= 80)
            {
                NitroVM_FinishRead(&machine);
                lookup.valid = false;
                break;
            }

            entry = &lookup.entries[idx];
            if (!NitroVM_WriteOutFilePath(&machine, filename, sizeof(filename)))
            {
                lookup.valid = false;
                break;
            }
            int reducedNameLength = strlen(filename) - 5;            
            entry->length = reducedNameLength;
            unsigned short invChecksum = 0;
            func_020d1d1c(&invChecksum, filename + 5, reducedNameLength);
            entry->checksum = ~invChecksum;
            NitroVM_FinishRead(&machine);
        }
        lookup.numEntries = idx;
        narc.Destroy();
    }
    UnlockResourceMutex();

    unsigned int animCount = package->bcfgData.GetNumAnimations();
    package->pAnim3Ds = (AnimationPackage::Reference*)allocator->Allocate(animCount * sizeof(AnimationPackage::Reference));
    if (package->pAnim3Ds == NULL)
        return false;

    for (int i = 0; i < animCount; i++)
    {
        AnimationPackage::Reference* ref = &package->pAnim3Ds[i];
        ref->count = 0;
        ref->data = NULL;
        BCFG::AnimationRecord* record = package->bcfgData.GetAnimationRecord(i);
        if (record == NULL || (animCallback != NULL && !animCallback(record)))
            continue;
        unsigned int numTypesFound = 0;
        void* rawAnimFilePointers[4];
        for (int j = 0; j < 4; j++)
        {
            sprintf(candidateName, data_020efb3c, record->name, data_020efaf8[j]);
            bool foundInLookup = false;
            const void* archiveData;
            bool foundInLookup_inner;
            unsigned int nameLength;
            const void* foundFile;
            unsigned int foundFileLength;
            bool lookupUsable = lookup.valid;
            const void* fileBytes2;
            unsigned int fileLength2;
            if (lookupUsable)
            {
                archiveData = loadInfo->fileData;
                if (!lookupUsable)
                    foundInLookup_inner = false;
                else
                {
                    nameLength = strlen(candidateName);
                    unsigned short invChecksum = 0;
                    func_020d1d1c(&invChecksum, candidateName, nameLength);
                    unsigned int checksum = ~invChecksum;
                    foundInLookup_inner = false;
                    for (int k = 0; k < lookup.numEntries; k++)
                    {
                        ChecksumLookup::Entry* entry = &lookup.entries[k];
                        if (entry->length != nameLength || entry->checksum != (unsigned short)checksum)
                            continue;
                        LockResourceMutex();
                        char filename[80];
                        NitroVM machine;
                        NarcHandle narc;
                        if (narc.Initialize(data_020efb0f, (const unsigned char*)archiveData))
                        {
                            NitroVM_Initialize(&machine);
                            PrepareReadFileInNARCByID(&machine, &narc, k);
                            
                            NitroVM_WriteOutFilePath(&machine, filename, sizeof(filename));
                            if (strcmp(filename + 5, candidateName) == 0)
                            {
                                foundFile = narc.GetFileByIndex(k);
                                foundFileLength = machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;
                                foundInLookup_inner = true;
                            }
                            NitroVM_FinishRead(&machine);
                            narc.Destroy();
                        }
                        UnlockResourceMutex();
                        if (foundInLookup_inner)
                            break;
                    }
                }

                if (foundInLookup_inner)
                {
                    fileBytes2 = foundFile;
                    fileLength2 = foundFileLength;
                    foundInLookup = true;
                }
            }

            if (foundInLookup || (!lookupUsable && GetFileInNarc(loadInfo->fileData, candidateName, &fileBytes2, &fileLength2, 0)))
            {
                if (loadInfo->unk_10 != 0)
                {
                    void* allocation = allocator->Allocate(fileLength2);
                    rawAnimFilePointers[numTypesFound] = allocation;
                    if (allocation == NULL)
                        return false;
                    memcpy(allocation, fileBytes2, fileLength2);
                }
                else
                    rawAnimFilePointers[numTypesFound] = const_cast<void*>(fileBytes2);
                numTypesFound++;
            }
        }

        if (numTypesFound != 0)
        {
            ref->data = (Animation3D*)allocator->Allocate(numTypesFound * sizeof(Animation3D));
            if (ref->data != NULL)
            {
                ref->count = numTypesFound;
                for (unsigned int j = 0; j < ref->count; j++)
                {
                    Animation3D* anim = &ref->data[j];
                    anim->DefaultInitialize();
                    anim->SetRawFile(static_cast<NSBXXContainer*>(rawAnimFilePointers[j]));
                    anim->CreateData(pModel_, allocator, NULL);
                }
            }
        }
    }

    AnimationPackageListInsert(&loadedAnimationPackageList_, package);
    return true;
}

void Object3D::ShallowCloneTo(Object3D *out)
{
    out->unknown_2_ = unknown_2_;
    out->radius_ = radius_;
    out->height_ = height_;
    out->pModel_ = pModel_;
    out->position_ = position_;
    out->rotation_ = rotation_;
    out->scale_[0] = scale_[0];
    out->scale_[1] = scale_[1];
    out->scale_[2] = scale_[2];
    out->inheritedAlpha_ = inheritedAlpha_;
    out->unknown_78_ = unknown_78_;
    out->unknown_7a_ = unknown_7a_;
    out->loadedAnimationPackageList_ = loadedAnimationPackageList_;
}

void Object3D::ShallowCloneModelAndAnimationsTo(Object3D *out)
{
    out->pModel_ = pModel_;
    out->loadedAnimationPackageList_ = loadedAnimationPackageList_;
}

bool Object3D::MaybeSetRegularAnimation(const char *animName, int flags)
{
    if (pModel_ == NULL || animName == NULL || (flags_6c_ & (1 << OBJECT3D_FLAG_22)) || animName[0] == '\0')
        return false;
    
    AnimationPackage* package;
    BCFG* bcfg = NULL;
    int index = -1;
    for (package = loadedAnimationPackageList_; package != NULL; package = package->pNext)
    {
        AnimationPackage::Reference* substruct = package->pAnim3Ds;
        if (substruct == NULL)
            continue;
        bcfg = &package->bcfgData;
        if (bcfg == NULL)
            continue;
        index = bcfg->SearchAnimationByName(animName);
        if (index < 0)
            continue;
        if (package->animationType != 1 || package->pAnim3Ds[index].data != NULL)
            break;
        index = -1;
    }

    if (index < 0)
        return false;

    if (!(flags & 8) && package == activeAnimationPackage_ && index == activeAnimationIndex_)
    {
        if ((flags & OBJECT_ANIMATION_REVERSE) && (animationFlags_ & OBJECT_ANIMATION_REVERSE))
            return true;
        if (!(flags & OBJECT_ANIMATION_REVERSE) && !(animationFlags_ & OBJECT_ANIMATION_REVERSE))
            return true;
    }

    if (package->animationType == 0)
    {
        BCFG::AnimationRecord* bcfgAnim = bcfg->GetAnimationRecord(index);
        fix32_t time;
        if (flags & OBJECT_ANIMATION_REVERSE)
            time = bcfgAnim->endTime;
        else
            time = bcfgAnim->startTime;
        activeAnimationIndex_ = index;
        animationTime_ = time;
        prevFrameAnimationTime_ = time;
        animationFlags_ = flags;
        normalizedAnimationTime_ = 0;
        prevFrameNormalizedAnimationTime_ = 0;
        unknown_40_bit_1_ = true;
        animationEnded_ = false;
        animationReachedEnd_ = false;
        activeAnimationRecord_ = bcfgAnim;
        activeAnimationPackage_ = package;
    }
    else if (package->animationType == 1)
    {
        BCFG::AnimationRecord* bcfgAnim = bcfg->GetAnimationRecord(index);
        Animation3D* anim3d = package->pAnim3Ds[index].data;
        fix32_t time;
        if (!(flags & OBJECT_ANIMATION_REVERSE))
            time = 0;
        else
        {
            time = GetAnimationFrameCountFix32(anim3d->GetBasicAnimationData()) - 0x1000;
        }
        if (flags & 0x10)
        {
            if (priorAnimationBlend_.blendTimeRemaining != 0)
                priorAnimationBlend_.Clear();
            else
            {
                priorAnimationBlend_.Clear();
                priorAnimationBlend_.source = activeAnimationPackage_;
                priorAnimationBlend_.unk_8 = activeAnimationIndex_;
                priorAnimationBlend_.sourceTime = animationTime_;
                priorAnimationBlend_.blendWeight = 0x1000;
                priorAnimationBlend_.blendTimeRemaining = defaultAnimBlendDuration_;
            }
        }
        activeAnimationIndex_ = index;
        animationTime_ = time;
        prevFrameAnimationTime_ = time;
        animationFlags_ = flags;
        normalizedAnimationTime_ = 0;
        prevFrameNormalizedAnimationTime_ = 0;
        unknown_40_bit_1_ = true;
        animationEnded_ = false;
        animationReachedEnd_ = false;
        activeAnimationRecord_ = bcfgAnim;
        activeAnimationPackage_ = package;
        if (priorAnimationBlend_.source == package && priorAnimationBlend_.unk_8 == activeAnimationIndex_)
            priorAnimationBlend_.Clear();
    }

    flags_6c_ &= ~(1 << OBJECT3D_FLAG_18);
    return true;
}

bool Object3D::MaybeSetBCFGAnimation(int index, int flags)
{
    if (loadedAnimationPackageList_ == NULL)
        return false;

    if (pModel_ == NULL)
        return false;

    BCFG::AnimationRecord* record = loadedAnimationPackageList_->bcfgData.GetAnimationRecord(index);
    if (record == NULL)
        return false;
    
    fix32_t time;
    if (flags & OBJECT_ANIMATION_REVERSE)
        time = record->endTime;
    else
        time = record->startTime;
    
    activeAnimationIndex_ = index;
    animationTime_ = time;
    prevFrameAnimationTime_ = time;
    animationFlags_ = flags;
    normalizedAnimationTime_ = prevFrameNormalizedAnimationTime_ = 0;
    unknown_40_bit_1_ = true;
    activeAnimationRecord_ = record;
    activeAnimationPackage_ = loadedAnimationPackageList_;
    flags_6c_ &= ~(1 << OBJECT3D_FLAG_18);

    return true;
}

void Object3D::StopCurrentAnimation()
{
    if (pModel_ != NULL)
        pModel_->RemoveAnimations();

    activeAnimationPackage_ = NULL;
    activeAnimationIndex_ = -1;
    activeAnimationRecord_ = NULL;
    priorAnimationBlend_.Clear();
}

int Object3D::HasAnimationStopped() const { return animationEnded_; }
int Object3D::HasAnimationReachedEnd() const { return animationReachedEnd_; }
int Object3D::GetOffset40Bit1() const { return unknown_40_bit_1_; }

void Object3D::SetCurrentAnimationTime(fix32_t time)
{
    // extra assignment prevents other functions from inlining calls to this
    fix32_t newTime = time;
    prevFrameAnimationTime_ = animationTime_;
    animationTime_ = newTime;
}

fix32_t Object3D::GetCurrentAnimationTime() const
{
    return animationTime_;
}

void Object3D::SetNormalizedAnimationTime(fix32_t normTime)
{
    if (activeAnimationPackage_ == NULL)
        return;
    if (normTime < 0 || normTime > (1 << 12))
        return;

    normalizedAnimationTime_ = normTime;
    AnimationPackage* package = activeAnimationPackage_;
    if (package->animationType == 0)
    {
        BCFG* bcfg = &package->bcfgData;
        if (bcfg == NULL)
            return;
        if (activeAnimationIndex_ >= 0)
        {
            BCFG::AnimationRecord* animEntry = bcfg->GetAnimationRecord(activeAnimationIndex_);
            if (animEntry != NULL)
            {
                fix32_t duration = animEntry->endTime - animEntry->startTime;
                SetCurrentAnimationTime(animEntry->startTime + FIX32_MULTIPLY(duration, normalizedAnimationTime_));
            }
        }
    }
    else if (package->animationType == 1)
    {
        if (activeAnimationIndex_ >= 0)
        {
            AnimationPackage::Reference* animSet = package->pAnim3Ds;
            if (animSet != NULL && animSet[activeAnimationIndex_].data != NULL)
            {
                Animation3D* anim3D = animSet[activeAnimationIndex_].data;
                if (anim3D->GetBasicAnimationData() != NULL)
                {
                    fix32_t number24 = normalizedAnimationTime_;
                    fix32_t endFrame = GetAnimationFrameCountFix32(anim3D->GetBasicAnimationData()) - 0x1000;
                    SetCurrentAnimationTime(FIX32_MULTIPLY(endFrame, number24));
                }
            }
        }
    }
}

void Object3D::TransitionInheritedAlpha(int alpha, int duration)
{
    alphaTransition_.current = 65535.0f * (inheritedAlpha_ / 31.0f);
    alphaTransition_.ConfigureTween(alpha, duration);
}

void Object3D::SetFlag16() { flags_6c_ |= (1 << OBJECT3D_FLAG_16); }
void Object3D::ClearFlag16() { flags_6c_ &= ~(1 << OBJECT3D_FLAG_16); }

void Object3D::Destroy()
{
    Detach();
    Detach();
    trackedBoneMatrixList_ = NULL;
    pModel_ = NULL;
    StopCurrentAnimation();
    loadedAnimationPackageList_ = NULL;
}

void Object3D::RemoveAnimationPackageByID(int id)
{
    if (loadedAnimationPackageList_ == NULL)
        return;
    StopCurrentAnimation();
    if (loadedAnimationPackageList_->packageID == id)
    {
        loadedAnimationPackageList_ = loadedAnimationPackageList_->pNext;
        return;
    }
    
    AnimationPackage* package = loadedAnimationPackageList_->pNext;
    AnimationPackage* prevPackage = loadedAnimationPackageList_;
    while (package != NULL)
    {
        if (package->packageID == id)
            prevPackage->pNext = package->pNext;
        else
            prevPackage = package;
        package = package->pNext;
    }
}

void Object3D::RemoveAllAnimationPackages()
{
    StopCurrentAnimation();
    loadedAnimationPackageList_ = NULL;
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

void Object3D::SetInheritedAlpha(int alpha)
{
    inheritedAlpha_ = alpha;
    if (pModel_ != NULL && !(flags_6c_ & (1 << OBJECT3D_FLAG_29)))
    {
        unsigned char combined = GetCombinedAlpha();
        pModel_->SetAlpha(combined);
    }
}

int Object3D::GetInheritedAlpha() const
{
    return inheritedAlpha_;
}

int Object3D::GetOwnAlpha() const
{
    return ownAlpha_;
}

int Object3D::GetCombinedAlpha() const
{
    return (inheritedAlpha_ * ownAlpha_) / 31;
}

BCFG *Object3D::GetCurrentAnimationConfig()
{
    if (activeAnimationPackage_ != NULL)
        return &activeAnimationPackage_->bcfgData;
    return NULL;
}

void Object3D::SetScale(const Vector3fix *vec)
{
    scale_[0] = vec->x;
    scale_[1] = vec->y;
    scale_[2] = vec->z;
}

void Object3D::SetScale(fix32_t x, fix32_t y, fix32_t z)
{
    scale_[0] = x;
    scale_[1] = y;
    scale_[2] = z;
}

Vector3fix Object3D::GetScale() const
{
    Vector3fix ret = { scale_[0], scale_[1], scale_[2] };
    return ret;
}

void Object3D::SetDiffuseColor(unsigned int bgrCol)
{
    if (pModel_ != NULL)
        pModel_->SetMaterialDiffuseColor(bgrCol);
}

void Object3D::UseRenderConfigDiffuseColor()
{
    if (pModel_ != NULL)
        pModel_->ApplyRenderConfigMaterialDiffuseColor();
}

BCFG::AnimationRecord* Object3D::GetLoadedAnimationRecord(const char* targetName)
{
    for (AnimationPackage* package = loadedAnimationPackageList_; package != NULL; package = package->pNext)
    {
        BCFG* bcfg = &package->bcfgData;
        if (bcfg == NULL)
            continue;
        for (int i = 0; i < bcfg->GetNumAnimations(); i++)
        {
            BCFG::AnimationRecord* record = bcfg->GetAnimationRecord(i);
            if (record == NULL)
                continue;
            if (strcmp(record->name, targetName) == 0)
                return record;
        }
    }
    return NULL;
}

AnimationPackage* Object3D::GetAnimationPackageByIndex(int idx)
{
    AnimationPackage* package = loadedAnimationPackageList_;
    while (package != NULL)
    {
        if (idx == 0)
            break;
        package = package->pNext;
        idx--;
    }
    return package;
}

void Object3D::SetField06(unsigned short value) { unknown_6_ = value; }
unsigned short Object3D::GetField06() const { return unknown_6_; }
void Object3D::SetField78(unsigned short value) { unknown_78_ = value; }
unsigned short Object3D::GetField78() const { return unknown_78_; }
void Object3D::SetField7a(unsigned short value) { unknown_7a_ = value; }
unsigned short Object3D::GetField7a() const { return unknown_7a_; }

void Object3D::SetAnimationPlaybackSpeed(fix16_t speed, int transitionTime)
{
    if (transitionTime == 0)
    {
        animationPlaybackSpeed_ = speed;
        targetAnimationPlaybackSpeed_ = speed;
        animationPlaybackSpeedTransitionRemainingTicks_ = 0;
    }
    else
    {
        targetAnimationPlaybackSpeed_ = speed;
        animationPlaybackSpeedTransitionRemainingTicks_ = transitionTime;
    }
}

void Object3D::StageTextureData()
{
    if (pModel_ == NULL)
        return;
    NSBXXTex* tex0 = pModel_->GetTEX0();
    if (tex0 != NULL)
    {
        StageTexFilePaletteData(tex0, false);
        StageTexFileImageData(tex0, false);
    }
}

void Object3D::ComputeShadowPosition()
{    
    if (pModel_ == NULL)
        return;
    AnimationPackage* package;
    ModelRenderContext* context;
    Model3D* model = pModel_;
    
    if (!(context = GetModel3DContext(pModel_)))
        return;
    if (shadowBoneIndex_ < 0)
    {
        for (package = loadedAnimationPackageList_; package != NULL; package = package->pNext)
        {
            if (package->bcfgData.MaybeGetRootBoneName() == NULL)
                continue;
            shadowBoneIndex_ = model->GetBoneIndex(package->bcfgData.MaybeGetRootBoneName());
            if (shadowBoneIndex_ >= 0)
                return ComputeShadowPosition();
        }
    }
    else
    {
        if (GetModelBonePositionAndDirectionMatrices(context, NULL, NULL, shadowBoneIndex_))
        {
            fix32_t boneWorldTransform4x3[12];
            func_020c1840(&boneWorldTransform4x3[0]);
            const fix32_t* inverseView = RenderConfig::GetInverseViewMatrix();
            GetCurrentPositionAndDirectionMatrices(&boneWorldTransform4x3[0], NULL);
            func_020c1d60(boneWorldTransform4x3, inverseView, boneWorldTransform4x3);
            fix32_t z = boneWorldTransform4x3[11];
            fix32_t y = boneWorldTransform4x3[10];
            fix32_t x = boneWorldTransform4x3[9];
            maybeShadowPosition_.x = x;
            maybeShadowPosition_.y = y;
            maybeShadowPosition_.z = z;
        }
    }
}

ModelRenderContext* GetModel3DContext(Model3D* model)
{
    if (!model->unknown_flags_a8_0_)
        return NULL;
    return &model->renderContext_;
}

Vector3fix Object3D::MaybeGetShadowSource() const
{
    Vector3fix ret = position_;
    ret.x = maybeShadowPosition_.x;
    ret.z = maybeShadowPosition_.z;
    return ret;
}

void Object3D::SetHeight(fix32_t height) { height_ = height; }
fix32_t Object3D::GetHeight() const { return height_; }
void Object3D::SetRadius(fix32_t radius) { radius_ = radius; }
fix32_t Object3D::GetRadius() const { return radius_; }

bool Object3D::IsTransitioningAnimations() const
{
    return priorAnimationBlend_.blendTimeRemaining != 0;
}

void Object3D::SkipAnimationTransition()
{
    priorAnimationBlend_.Clear();
    priorAnimationBlend_.blendTimeRemaining = 0;
}

void AnimationPackageListInsert(AnimationPackage** pListStart, AnimationPackage* entry)
{
    if (*pListStart == NULL)
    {
        *pListStart = entry;
        entry->pNext = NULL;
    }
    else
    {
        AnimationPackage* listEnd = *pListStart;
        while (listEnd->pNext != NULL)
            listEnd = listEnd->pNext;
        listEnd->pNext = entry;
        entry->pNext = NULL;
    }
}

void Object3D::ClearChildListData() 
{
    pPrevChild_ = NULL;
    pNextChild_ = NULL;
    attachmentBoneIndex_ = -1;
}

void Object3D::Attach(Object3D *parent, short boneIndex)
{
    // can't attach this if it's already attached elsewhere
    if (pPrevChild_ != NULL)
        return;

    attachmentBoneIndex_ = boneIndex;
    Object3D* listEnd = parent;
    while (listEnd->pNextChild_ != NULL)
        listEnd = listEnd->pNextChild_;
    listEnd->pNextChild_ = this;
    this->pPrevChild_ = listEnd;
    this->pNextChild_ = NULL;
}

void Object3D::Attach(Object3D *parent, const char *boneName)
{
    if (pPrevChild_ != NULL)
        return;
    if (boneName != NULL)
    {
        if (parent->pModel_ == NULL)
            return;
        attachmentBoneIndex_ = parent->pModel_->GetBoneIndex(boneName);
    }
    Attach(parent, attachmentBoneIndex_);
}

void Object3D::Detach()
{
    if (pPrevChild_ != NULL)
    {
        pPrevChild_->pNextChild_ = this->pNextChild_;
        if (pNextChild_ != NULL)
            pNextChild_->pPrevChild_ = this->pPrevChild_;
        this->ClearChildListData();
    }
    else
    {
        Object3D* child = pNextChild_;
        while (child != NULL)
        {
            Object3D* next = child->pNextChild_;
            child->ClearChildListData();
            child = next;
        }
        this->ClearChildListData();
    }
}

void Object3D::DrawChildren(Object3D *topLevelObject)
{
    if (pNextChild_ == NULL)
        return;

    if (pNextChild_->attachmentBoneIndex_ < 0)
    {
        pNextChild_->DrawChildren(topLevelObject);
        topLevelObject->PopulateRenderConfigWorld();
        unsigned char combinedAlpha = this->GetCombinedAlpha();
        pNextChild_->SetInheritedAlpha(combinedAlpha);
        pNextChild_->Draw(false);
    }
    else
    {
        bool drawDone = false;
        if (topLevelObject->pModel_ != NULL)
        {
            fix32_t positionMatrix[12];
            fix32_t directionMatrix[9];
            ModelRenderContext* context = GetModel3DContext(topLevelObject->pModel_);
            if (context != NULL && GetModelBonePositionAndDirectionMatrices(context,
                positionMatrix, directionMatrix, pNextChild_->attachmentBoneIndex_))
            {
                // bug? flags is 32-bit, we lose upper 16 bits...
                unsigned short oldFlags = pNextChild_->flags_6c_;
                pNextChild_->flags_6c_ |= (1 << OBJECT3D_FLAG_11) | (1 << OBJECT3D_FLAG_14);
                unsigned char combinedAlpha = (inheritedAlpha_ * ownAlpha_) / 31;
                pNextChild_->SetInheritedAlpha(combinedAlpha);
                pNextChild_->Draw(true);
                pNextChild_->flags_6c_ = oldFlags;
                pNextChild_->DrawChildren(topLevelObject);
                drawDone = true;
            }
        }
        if (!drawDone)
            pNextChild_->DrawChildren(topLevelObject);
    }
}

bool Object3D::IsChild() const
{
    return pPrevChild_ != NULL;
}

bool Object3D::IsParent() const
{
    return pNextChild_ != NULL && pPrevChild_ == NULL;
}

int Object3D::GetFlag(int mask) const { return flags_6c_ & mask; }
void Object3D::EnableFlag(int mask) { flags_6c_ |= mask; }
void Object3D::DisableFlag(int mask) { flags_6c_ &= ~mask; }
int Object3D::GetFlags() const { return flags_6c_; }

void Object3D::SetNoTextureTimer(int timer) { noTextureTimer_ = timer; }

void Object3D::SetTexturePaletteOffset(unsigned int offset) { alternativeTexturePaletteOffset_ = offset; }
unsigned int Object3D::GetTexturePaletteOffset() const { return alternativeTexturePaletteOffset_; }

void Object3D::SetAlphaScaleFactor(fix32_t value) { alphaScaleFactor_ = value; }
fix32_t Object3D::GetAlphaScaleFactor() const { return alphaScaleFactor_; }

void Object3D::SetField9e(unsigned char value) { unknown_9e_ = value; }
unsigned char Object3D::GetField9e() const { return unknown_9e_; }

void Object3D::TrackBone(TrackedBoneMatrix *trackingEntry)
{
    trackingEntry->pNext = NULL;
    func_020c1840(&trackingEntry->matrix[0]);
    if (trackedBoneMatrixList_ != NULL)
    {
        TrackedBoneMatrix* listEnd = trackedBoneMatrixList_;
        while (listEnd->pNext != NULL)
            listEnd = listEnd->pNext;
        listEnd->pNext = trackingEntry;
    }
    else
        trackedBoneMatrixList_ = trackingEntry;
}

void Object3D::UntrackBoneByIndex(int idx)
{
    TrackedBoneMatrix* loopEntry = trackedBoneMatrixList_;
    TrackedBoneMatrix* prev = NULL;
    for (; loopEntry != NULL; prev = loopEntry, loopEntry = loopEntry->pNext)
    {
        if (loopEntry->boneIndex != idx)
            continue;

        if (prev != NULL)
            prev->pNext = loopEntry->pNext;
        else
            trackedBoneMatrixList_ = loopEntry->pNext;
        break;
    }
}

void Object3D::UntrackAllBones() { trackedBoneMatrixList_ = NULL; }

Object3D::TrackedBoneMatrix* Object3D::GetTrackedBoneMatrix(int idx)
{
    for (TrackedBoneMatrix* loopEntry = trackedBoneMatrixList_; loopEntry != NULL; loopEntry = loopEntry->pNext)
    {
        if (loopEntry->boneIndex == idx)
            return loopEntry;
    }
    return NULL;
}

void BoneTrackingRenderCommandHook(RenderCommandHandler*);

void Object3D::EnableBoneTracking()
{
    if (pModel_ == NULL)
        return;

    ModelRenderContext* context = GetModel3DContext(pModel_);
    if (context == NULL)
        return;
    SetModelRenderContextRenderCommandHook(context, &BoneTrackingRenderCommandHook, 0, 6, 3);
}

void BoneTrackingRenderCommandHook(RenderCommandHandler* handler)
{
    for (Object3D::TrackedBoneMatrix* tracker = data_02104b18.boneMatrixList;
        tracker != NULL; tracker = tracker->pNext)
    {
        if (!(handler->flags_ & (1 << RCH_FLAG_4)) || handler->currentBoneMatrix_ != tracker->boneIndex)
            continue;
        
        GetCurrentPositionAndDirectionMatrices(&tracker->matrix[0], NULL);
        // why does this work? copying bytes gets optimized to copying dwords? Not complaining though...
        COPY_ARRAY(data_02104b18.boneMatrixWithView, tracker->matrix);
        func_020c1d60(&tracker->matrix[0], RenderConfig::GetInverseViewMatrix(), &tracker->matrix[0]);
    }
}

Vector3fix Object3D::GetPointInFront(fix32_t distance) const
{
    Vector3fix ownPos = position_;
    Vector3fix ownRot = rotation_;
    Vector3fix forward /* = { 0, 0, 0 }*/;
    func_0200f374(&forward, sizeof(Vector3fix));
    forward.x = func_02030c68(ownRot.y);
    forward.z = func_02030c9c(ownRot.y);
    Vector3fix output;
    func_02030e2c(&forward, distance, &output);
    func_020c2d90(&output, &ownPos, &output);
    return output;
}