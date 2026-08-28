#include "Graphics/Lighting.h"
#include "Resource/Script.h"

#if defined(jpn)
#define func_0205111c func_020525ec
#define data_020f0ed8 data_020f0fa0
#define data_02108e60 data_02108da4
#endif

extern Script::OpcodeLookupEntry data_020f0ed8[];

extern LightingManager* data_02108e60;

extern "C"
{
    // this is defined right after the first function to use it, and it
    // just copies data so it's probably an implicit operator=
    LightInfo0205111c* func_0205111c(LightInfo0205111c*, const LightInfo0205111c*);
}

int LightingScript_Opcode_64(Script::Parameter* params, int numParams)
{
    if (numParams > 0)
        data_02108e60->unknown_308_ = params[0].ToFloat();
    data_02108e60->maybeMode_ = 2;
    return 1;
}

int LightingScript_Opcode_65(Script::Parameter* params, int numParams) { return 1; }

// Never used
int LightingScript_Opcode_66(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();

    if (index > 6)
        return 1;

    LightInfo0205111c secondaryEntry;
    secondaryEntry.unk_10 = (params++)->ToInt();
    secondaryEntry.unk_4 = (params++)->ToFloat();
    secondaryEntry.unk_8 = (params++)->ToFloat();
    secondaryEntry.unk_c = (params++)->ToFloat();
    secondaryEntry.unk_0 = (params++)->ToInt();
    func_0205111c(&data_02108e60->big_.unk_8c[index], &secondaryEntry);

    LightInfo0205111c primaryEntry;
    primaryEntry.unk_10 = (params++)->ToInt();
    primaryEntry.unk_4 = 0.5f;
    primaryEntry.unk_8 = -1.0f;
    primaryEntry.unk_c = 0.5f;
    primaryEntry.unk_0 = (params++)->ToInt();
    primaryEntry.unk_10 = true;
    func_0205111c(&data_02108e60->big_.unk_0[index], &primaryEntry);

    int col1 = (params++)->ToInt();
    int col2 = (params++)->ToInt();
    int col3 = (params++)->ToInt();
    int col4 = (params++)->ToInt();
    int col5 = (params++)->ToInt();
    int col6 = (params++)->ToInt();
    int col7 = (params++)->ToInt();
    data_02108e60->big_.unk_126[index] = col1;
    data_02108e60->big_.unk_134[index] = col2;
    data_02108e60->big_.unk_118[index] = col3;
    data_02108e60->big_.unk_142[index] = col4;
    data_02108e60->big_.unk_150[index] = col5;
    data_02108e60->big_.unk_15e[index] = col6;
    data_02108e60->big_.unk_16c[index] = col7;

    return 1;
}

int LightingScript_Opcode_67(Script::Parameter* params, int numParams)
{
    if (numParams > 0)
        data_02108e60->unknown_308_ = params[0].ToFloat();
    data_02108e60->maybeMode_ = 1;
    return 1;
}

// Used in conjunction with opcode 67, modifies the small struct
int LightingScript_Opcode_68(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    float argX = (params++)->ToFloat();
    float argY = (params++)->ToFloat();
    float argZ = (params++)->ToFloat();
    int col1 = (params++)->ToInt();
    int col2 = (params++)->ToInt();
    int col3 = (params++)->ToInt();
    float arg8 = (params++)->ToFloat();
    float arg9 = (params++)->ToFloat();
    int col4 = (params++)->ToInt();
    int col5 = (params++)->ToInt();
    int col6 = (params++)->ToInt();

    Vector3float& storageVec = data_02108e60->small_.unk_0[index];

    storageVec.x = argX;
    storageVec.y = argY;
    storageVec.z = argZ;
    data_02108e60->small_.unk_8c[index] = col1;
    data_02108e60->small_.unk_9a[index] = col2;
    data_02108e60->small_.unk_a8[index] = col3;
    data_02108e60->small_.unk_70[index] = arg8;
    data_02108e60->small_.unk_54[index] = arg9;
    data_02108e60->small_.unk_e0[index] = 1;
    data_02108e60->small_.unk_b6[index] = col4;
    data_02108e60->small_.unk_c4[index] = col5;
    data_02108e60->small_.unk_d2[index] = col6;

    return 1;
}

int LightingScript_Opcode_69(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    if (index > 6)
        return 1;
    
    LightingManager::Struct_17c entry;

    entry.unk_0 = (params++)->ToInt() != 0;
    entry.unk_10 = (params++)->ToInt();
    entry.unk_4 = (params++)->ToInt();
    entry.unk_8 = (params++)->ToInt();
    entry.unk_c = (params++)->ToInt();
    for (int i = 0; i < 8; i++)
    {
        int packed = (params++)->ToInt();
        entry.buffer_18[4 * i] = packed & 0xff;
        entry.buffer_18[4 * i + 1] = (packed & 0xff00) >> 8;
        entry.buffer_18[4 * i + 2] = (packed & 0xff0000) >> 16;
        entry.buffer_18[4 * i + 3] = (packed & 0xff000000) >> 24;
    }
    entry.unk_14 = (params++)->ToInt();

    LightingManager* mgr = data_02108e60;
    mgr->unk_17c[index].unk_0 = entry.unk_0;
    mgr->unk_17c[index].unk_4 = entry.unk_4;
    mgr->unk_17c[index].unk_8 = entry.unk_8;
    mgr->unk_17c[index].unk_c = entry.unk_c;
    mgr->unk_17c[index].unk_10 = entry.unk_10;
    mgr->unk_17c[index].unk_14 = entry.unk_14;
    COPY_ARRAY(mgr->unk_17c[index].buffer_18, entry.buffer_18);

    return 1;
}

int LightingScript_Opcode_6a(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    LightInfo0205111c secondaryEntry;
    secondaryEntry.unk_10 = (params++)->ToInt();
    secondaryEntry.unk_4 = (params++)->ToFloat();
    secondaryEntry.unk_8 = (params++)->ToFloat();
    secondaryEntry.unk_c = (params++)->ToFloat();
    secondaryEntry.unk_0 = (params++)->ToInt();
    func_0205111c(&data_02108e60->big_.unk_8c[index], &secondaryEntry);

    LightInfo0205111c primaryEntry;
    primaryEntry.unk_10 = (params++)->ToInt();
    primaryEntry.unk_4 = (params++)->ToFloat();
    primaryEntry.unk_8 = (params++)->ToFloat();
    primaryEntry.unk_c = (params++)->ToFloat();
    primaryEntry.unk_0 = (params++)->ToInt();
    func_0205111c(&data_02108e60->big_.unk_0[index], &primaryEntry);

    int col1 = (params++)->ToInt();
    int col2 = (params++)->ToInt();
    int col3 = (params++)->ToInt();
    int col4 = (params++)->ToInt();
    int col5 = (params++)->ToInt();
    int col6 = (params++)->ToInt();
    int col7 = (params++)->ToInt();
    data_02108e60->big_.unk_126[index] = col1;
    data_02108e60->big_.unk_134[index] = col2;
    data_02108e60->big_.unk_118[index] = col3;
    data_02108e60->big_.unk_142[index] = col4;
    data_02108e60->big_.unk_150[index] = col5;
    data_02108e60->big_.unk_15e[index] = col6;
    data_02108e60->big_.unk_16c[index] = col7;

    return 1;
}


void ExecuteLightingScript(const void* script, unsigned int length, LightingManager* manager, SafeAllocator* alloc)
{
    if (length == 0)
        return;
    data_02108e60 = manager;
    Script runner;
    runner.Initialize();
    runner.SetOpcodeLookup(data_020f0ed8);
    runner.Load(script, length);
    runner.Execute();
    data_02108e60 = NULL;
}

void LightingManager::Initialize()
{
    maybeMode_ = 0;
    unknown_308_ = 0.0f;
    memset(&small_, 0, sizeof(small_));
    memset(&big_, 0, sizeof(big_));
    memset(&unk_17c[0], 0, sizeof(unk_17c));
}

void LightingManager::Reset()
{
    maybeMode_ = 0;
    unknown_308_ = 0.0f;
    memset(&small_, 0, sizeof(small_));
    memset(&big_, 0, sizeof(big_));
    memset(&unk_17c[0], 0, sizeof(unk_17c));
}

void LightingManager::LoadFromScript(const void* script, unsigned int length, SafeAllocator* alloc)
{
    ExecuteLightingScript(script, length, this, alloc);
}
