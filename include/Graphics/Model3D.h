#pragma once

#include "std_library_functions.h"
#include "Vector.h"
#include "Memory/SafeAllocator.h"
#include "NSBXX/NSBXX.h"

class Model3D
{
public:
    // probably a substruct from 0 to 0x54
    int unknown_0_;
    char unk_4[0x50];
    NSBXXInternalModel* modelData_54_;
    void* texFileData_58_;
    void* rawFileData_;
    unsigned int rawFileSize_;
    // should make these six numbers a struct
    fix32_t xMax_;
    fix32_t yMax_;
    fix32_t zMax_;
    fix32_t xMin_;
    fix32_t yMin_;
    fix32_t zMin_;
    fix32_t xMiddle_;
    fix32_t maybeYBase_;
    fix32_t zMiddle_;
    fix32_t maybeApproxRadius_;
    fix32_t copyOfHeight_;
    int unknown_90_;
    int unknown_94_;
    int unknown_98_;
    int unknown_9c_;
    unsigned short unknown_a0_;
    unsigned short unknown_a2_;
    short imageStagingTaskID_;
    short paletteStagingTaskID_;
    int unknown_flags_a8_0_ : 1;
    int unknown_flags_a8_1_ : 1;
    int unknown_flags_a8_2_ : 1;

    // the class has what seems to be a constructor (no arguments) and a
    // destructor at 0207e23c and 0207e250 (usa) respectively. But I'm
    // not including it because the compiler generates two of each which breaks
    // the build process. There's probably a way around this, maybe we can 
    // explicitly mark a symbol to not be included at link time? But for now,
    // I'm just leaving them out.
    // 
    // (For what it's worth, we know it's a constructor and destructor instead
    // of just an Init() / Destroy() pair because their pointers get passed
    // to a call to func_0200ee94, which is used to default-initialize an
    // array of non-trivially constructible objects).

    void Clear();

    void Func0207e2e0();
    void LoadFromFile(const char* path, AllocatorUnion* alloc, int arg);

    void CopyAndProcessRawFile(AllocatorUnion* alloc, void* data, unsigned int len, int arg);
    void SetAndProcessRawFile(void* data, unsigned int len, int arg);

    void CopyRawFile(AllocatorUnion* alloc, void* data, unsigned int len);
    void SetRawFile(void* rawData, unsigned int length);
    void ClearRawFileCache();
    void ProcessRawFile(int arg);

    bool Draw(bool applyClipping);
    bool DrawShadow(bool applyClipping, int unknown2, int unknown3, int unknown4);
    int TestVisible();
};