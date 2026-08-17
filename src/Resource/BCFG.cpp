#include "Resource/BCFG.h"
#include "Resource/Script.h"

struct Struct_02104b10
{
    SafeAllocator* allocator;
    BCFG* pBCFG;
} extern data_02104b10;

extern Script::OpcodeLookupEntry data_020efa48[];
extern char data_020efaa0[]; // "waist"


int BCFGScript_Opcode_64(Script::Parameter* params, int paramCount)
{
    data_02104b10.pBCFG->AllocateAnimationEntries(data_02104b10.allocator, params[0].ToInt());
    return 1;
}

int BCFGScript_Opcode_65(Script::Parameter* params, int paramCount)
{
    return 1;
}

int BCFGScript_Opcode_66(Script::Parameter* params, int paramCount)
{
    const char* name = params[0].ToString();
    int success;
    if (name == NULL)
        success = false;
    else
    {
        BCFG::AnimationEntry entry;
        strcpy(entry.name, name);
        entry.unknown_vector.x = 4096 * params[1].ToFloat();
        entry.unknown_vector.y = 4096 * params[2].ToFloat();
        entry.unknown_vector.z = 4096 * params[3].ToFloat();
        entry.firstSizeC = NULL;
        entry.firstAlt = 0;
        data_02104b10.pBCFG->InsertAnimationEntry(&entry);
        success = true;
    }
    return success;
}

int BCFGScript_Opcode_6a(Script::Parameter* params, int paramCount)
{
    data_02104b10.pBCFG->AllocateSizeCEntries(data_02104b10.allocator, params[0].ToInt());
    return 1;
}

int BCFGScript_Opcode_6b(Script::Parameter* params, int paramCount)
{
    return 1;
}

int BCFGScript_Opcode_6c(Script::Parameter* params, int paramCount)
{
    BCFG::SizeCEntry entry;

    entry.animationIndex = data_02104b10.pBCFG->SearchAnimationByName(params[0].ToString());
    entry.unk_4 = 4096 * params[1].ToFloat();
    entry.unk_0 = params[2].ToInt();
    data_02104b10.pBCFG->InsertSizeCEntry(&entry);
    return 1;
}

int BCFGScript_Opcode_6d(Script::Parameter* params, int paramCount)
{
    data_02104b10.pBCFG->AllocateAltSizeCEntries(data_02104b10.allocator, params[0].ToInt());
    return 1;
}

int BCFGScript_Opcode_6e(Script::Parameter* params, int paramCount)
{
    return 1;
}

int BCFGScript_Opcode_6f(Script::Parameter* params, int paramCount)
{
    BCFG::AltSizeCEntry entry;

    entry.animationIndex = data_02104b10.pBCFG->SearchAnimationByName(params[0].ToString());
    entry.unk_4 = 4096 * params[1].ToFloat();
    entry.unk_0 = params[2].ToInt();
    data_02104b10.pBCFG->InsertAltSizeCEntry(&entry);
    return 1;
}

int BCFGScript_Opcode_71(Script::Parameter* params, int paramCount)
{
    data_02104b10.pBCFG->MaybeSetRootBoneName(data_02104b10.allocator, params[0].ToString());
    return 1;
}

void BCFG::Reset()
{
    animations_ = NULL;
    maybeAnimationCount = 0;
    maybeAnimationCapacity = 0;
    sizeCEntries_ = NULL;
    sizeCCapacity_ = 0;
    sizeCCount_ = 0;
    altSizeCEntries_ = NULL;
    altSizeCCount_ = 0;
    altSizeCCapacity_ = 0;
    unknown_18 = 0xffff;
    maybeRootBoneName_ = NULL;
}

void BCFG::LoadFromScript(SafeAllocator* alloc, const void *data, unsigned int length)
{
    if (length != 0 && data != NULL)
    {
        data_02104b10.pBCFG = this;
        data_02104b10.allocator = alloc;
        Script script;           
        script.Initialize();
        script.SetOpcodeLookup(data_020efa48);
        script.Load(data, length);
        script.Execute();
        if (data_02104b10.pBCFG->maybeRootBoneName_ == NULL)
            data_02104b10.pBCFG->MaybeSetRootBoneName(alloc, data_020efaa0);
        data_02104b10.pBCFG = NULL;
        data_02104b10.allocator = NULL;
    }
}

void BCFG::AllocateAnimationEntries(SafeAllocator *alloc, int count)
{
    if (alloc == NULL)
        return;

    animations_ = (AnimationEntry*)alloc->Allocate(count * sizeof(AnimationEntry));
    if (animations_ != NULL)
    {
        maybeAnimationCount = 0;
        maybeAnimationCapacity = count;
    }
    else
    {
        maybeAnimationCount = 0;
        maybeAnimationCapacity = 0;
    }
}

void BCFG::InsertAnimationEntry(const AnimationEntry *entry)
{
    if (maybeAnimationCapacity > maybeAnimationCount)
    {
        AnimationEntry* dest = &animations_[maybeAnimationCount++];
        COPY_ARRAY(dest->name, entry->name);
        dest->unknown_vector.x = entry->unknown_vector.x;
        dest->unknown_vector.y = entry->unknown_vector.y;
        dest->unknown_vector.z = entry->unknown_vector.z;
        dest->firstSizeC = entry->firstSizeC;
        dest->firstAlt = entry->firstAlt;
    }
}

int BCFG::SearchAnimationByName(const char* name)
{
    for (int i = 0; i < maybeAnimationCount; i++)
    {
        if (strcmp(animations_[i].name, name) == 0)
            return i;
    }
    return -1;
}

BCFG::AnimationEntry* BCFG::GetAnimationEntry(int idx)
{
    if (idx < 0 || maybeAnimationCount <= idx)
        return NULL;
    return &animations_[idx];
}

int BCFG::GetNumAnimations() const
{
    return maybeAnimationCount;
}

void BCFG::AllocateSizeCEntries(SafeAllocator *alloc, int count)
{
    sizeCEntries_ = (SizeCEntry*)alloc->Allocate(count * sizeof(SizeCEntry));
    if (sizeCEntries_ != NULL)
    {
        sizeCCapacity_ = count;
        sizeCCount_ = 0;
    }
    else
    {
        sizeCCapacity_ = 0;
        sizeCCount_ = 0;
    }
}

void BCFG::InsertSizeCEntry(const SizeCEntry *entry)
{
    if (sizeCCapacity_ > sizeCCount_)
    {
        AnimationEntry* anim = GetAnimationEntry(entry->animationIndex);
        if (anim == NULL)
            return;
        SizeCEntry* dest = &sizeCEntries_[sizeCCount_];
        dest->unk_0 = entry->unk_0;
        dest->animationIndex = entry->animationIndex;
        dest->unk_4 = entry->unk_4;
        dest->pNext = entry->pNext;
        sizeCEntries_[sizeCCount_].pNext = anim->firstSizeC;
        anim->firstSizeC = &sizeCEntries_[sizeCCount_];
        sizeCCount_++;
    }
}

void BCFG::AllocateAltSizeCEntries(SafeAllocator *alloc, int count)
{
    altSizeCEntries_ = (AltSizeCEntry*)alloc->Allocate(count * sizeof(AltSizeCEntry));
    if (altSizeCEntries_ != NULL)
    {
        altSizeCCapacity_ = count;
        altSizeCCount_ = 0;
    }
    else
    {
        altSizeCCapacity_ = 0;
        altSizeCCount_ = 0;
    }
}

void BCFG::InsertAltSizeCEntry(const AltSizeCEntry *entry)
{
    if (altSizeCCapacity_ > altSizeCCount_)
    {
        AnimationEntry* anim = GetAnimationEntry(entry->animationIndex);
        if (anim == NULL)
            return;
        AltSizeCEntry* dest = &altSizeCEntries_[altSizeCCount_];
        dest->unk_0 = entry->unk_0;
        dest->animationIndex = entry->animationIndex;
        dest->unk_4 = entry->unk_4;
        dest->pNext = entry->pNext;
        altSizeCEntries_[altSizeCCount_].pNext = anim->firstAlt;
        anim->firstAlt = &altSizeCEntries_[altSizeCCount_];
        altSizeCCount_++;
    }
}

void BCFG::MaybeSetRootBoneName(SafeAllocator *alloc, const char *name)
{
    maybeRootBoneName_ = (char*)alloc->Allocate(strlen(name) + 1);
    if (maybeRootBoneName_ != NULL)
        strcpy(maybeRootBoneName_, name);
}

const char *BCFG::MaybeGetRootBoneName()
{
    return maybeRootBoneName_;
}