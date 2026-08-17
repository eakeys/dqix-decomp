#pragma once

#include "../Memory/SafeAllocator.h"
#include "../Graphics/Vector.h"

// Struct/class populated using a *.bcfg file, which holds data about...
// what animations a model should use? 
class BCFG
{
public:
    struct SizeCEntry
    {
        unsigned short unk_0;
        unsigned short animationIndex;
        short unk_4;
        char unk_6[2];
        SizeCEntry* pNext;
    };

    // these could be the same struct
    struct AltSizeCEntry
    {
        unsigned short unk_0;
        unsigned short animationIndex;
        short unk_4;
        char unk_6[2];
        AltSizeCEntry* pNext;
    };

    struct AnimationEntry
    {
        char name[16];
        // the first two entries in this vector are times
        Vector3fix unknown_vector;
        SizeCEntry* firstSizeC;
        AltSizeCEntry* firstAlt;
    };

    AnimationEntry* animations_;
    unsigned short maybeAnimationCount;
    unsigned short maybeAnimationCapacity;
    SizeCEntry* sizeCEntries_;
    unsigned short sizeCCapacity_;
    unsigned short sizeCCount_;
    
    AltSizeCEntry* altSizeCEntries_;
    unsigned short altSizeCCount_;
    unsigned short altSizeCCapacity_;
    unsigned short unknown_18;
    char unk_1a[2];
    char* maybeRootBoneName_;

    void Reset();
    void LoadFromScript(SafeAllocator* alloc, const void* data, unsigned int length);

    // ought to be private tbh
public:
    void AllocateAnimationEntries(SafeAllocator* alloc, int count);
    void InsertAnimationEntry(const AnimationEntry* entry);
    int SearchAnimationByName(const char* name);
    AnimationEntry* GetAnimationEntry(int idx);
    int GetNumAnimations() const;
    
    void AllocateSizeCEntries(SafeAllocator* alloc, int count);
    void InsertSizeCEntry(const SizeCEntry* entry);

    void AllocateAltSizeCEntries(SafeAllocator* alloc, int count);
    void InsertAltSizeCEntry(const AltSizeCEntry* entry);

    void MaybeSetRootBoneName(SafeAllocator* alloc, const char* name);
    const char* MaybeGetRootBoneName();
};