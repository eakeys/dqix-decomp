#pragma once

#include "RenderCommands.h"

// Used to reference some kind of animation, specifically the innermost part
// of the NSBxx files. For example in a .nsbca file, the header has signature
// BCA0, the first file within this has signature JNT0, and the objects within
// that have signature J.AC (. = 0x00), and this references the J.AC object.
// Also known to be used for M.AM, M.AT, M.PT. Theoretically usable for something
// of form V.AV but it doesn't seem to be used anywhere in practice.
// Note: the V.?? is completely undocumented as far as I know, and the fact it's
// called V.AV just comes from looking at 0x020f1ca8 in the binary (USA)
// Note: these might be different structures for each type, but this format
// fits every type of animation in all currently known code
struct AnimationData
{
    fix32_t time_;
    // used for e.g. combining effects of multiple bones
    fix32_t weight_;
    void* pRawData_;
    // call this to populate renderData as appropriate
    void (*callback_)(void* renderData, AnimationData* animData, int arg);
    AnimationData* pNext_;
    NSBXXTex* pTex0_; // we always supply this, but only M.PT animations use it
    char unk_18;
    unsigned char numEntries_;
    // Not sure about max array length.
    // Each entry combines the following values:
    // bits 0-7: some index, seems to always just be 0,1,2,3...
    // bit 8: bool indicating whether this is 'used' / relevant in some way - 
    // the ModelRenderData bitfields get populated based on this
    // bit 9: ???, sometimes tested in conjunction with bit 8
    //
    // In the case of a joint animation, each entry is a track, and bits 0-7
    // specify the index of the bone matrix that the track adjusts. However, 
    // there is some weirdness/ambiguity between 'track' and 'bone index'. This 
    // struct's numEntries is set to the model's number of bone matrices,
    // while this array only gets populated in entries 0 to (#tracks in JAC - 1).
    // Also ProcessJointAnimationsOnBoneMatrix() takes what seems to be a bone
    // index as its third parameter, but then that gets used to index into this.
    // I think there's some implicit assumption in the code that this goes
    // 0, 1, 2, ... up to some point
    //
    // M.xx: index into this with entries_[x], where x is the index of the material
    // you care about within NSBXXModelMaterialData's NameList. The bottom eight
    // bits of entries_[x] then hold the track within the M.AM file that modify
    // said material.
    //
    // V.AV: entry k is set to k | 0x100, for k = 0, 1, ..., #boneMatrices - 1.
    // I have no idea what bone matrices have to do with this. 
    unsigned short entries_[64];
};