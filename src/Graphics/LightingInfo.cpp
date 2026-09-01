#include "Graphics/LightingInfo.h"
#include "Resource/Script.h"

#if defined(jpn)
#define data_020f0ed8 data_020f0fa0
#define data_02108e60 data_02108da4
#endif

extern Script::OpcodeLookupEntry data_020f0ed8[];

extern LightingInfo* data_02108e60;

int LightingScript_Opcode_64(Script::Parameter* params, int numParams)
{
    if (numParams > 0)
        data_02108e60->gradientCenterOffset_ = params[0].ToFloat();
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

    LightingManager::Light3DConfig secondaryEntry;
    secondaryEntry.maybeEnabled = (params++)->ToInt();
    secondaryEntry.direction[0] = (params++)->ToFloat();
    secondaryEntry.direction[1] = (params++)->ToFloat();
    secondaryEntry.direction[2] = (params++)->ToFloat();
    secondaryEntry.color = (params++)->ToInt();
    data_02108e60->advanced_.light1[index] = secondaryEntry;

    LightingManager::Light3DConfig primaryEntry;
    primaryEntry.maybeEnabled = (params++)->ToInt();
    primaryEntry.direction[0] = 0.5f;
    primaryEntry.direction[1] = -1.0f;
    primaryEntry.direction[2] = 0.5f;
    primaryEntry.color = (params++)->ToInt();
    primaryEntry.maybeEnabled = true;
    data_02108e60->advanced_.light0[index] = primaryEntry;

    int col1 = (params++)->ToInt();
    int col2 = (params++)->ToInt();
    int col3 = (params++)->ToInt();
    int col4 = (params++)->ToInt();
    int col5 = (params++)->ToInt();
    int col6 = (params++)->ToInt();
    int col7 = (params++)->ToInt();
    data_02108e60->advanced_.backgroundColor[index] = col1;
    data_02108e60->advanced_.backgroundSecondColor[index] = col2;
    data_02108e60->advanced_.maybeAmbientColor[index] = col3;
    data_02108e60->advanced_.unk_142[index] = col4;
    data_02108e60->advanced_.spriteDiffuseColor[index] = col5;
    data_02108e60->advanced_.modelDiffuseColor[index] = col6;
    data_02108e60->advanced_.edgeColor[index] = col7;

    return 1;
}

int LightingScript_Opcode_67(Script::Parameter* params, int numParams)
{
    if (numParams > 0)
        data_02108e60->gradientCenterOffset_ = params[0].ToFloat();
    data_02108e60->maybeMode_ = 1;
    return 1;
}

// Used in conjunction with opcode 67, configures basic lighting
// this function isn't a match yet
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

    Vector3float& storageVec = data_02108e60->basic_.unk_0[index];

    storageVec.x = argX;
    storageVec.y = argY;
    storageVec.z = argZ;
    data_02108e60->basic_.backgroundColor[index] = col1;
    data_02108e60->basic_.backgroundSecondColor[index] = col2;
    data_02108e60->basic_.potBarrelDiffuseColor[index] = col3;
    data_02108e60->basic_.unk_70[index] = arg8;
    data_02108e60->basic_.unk_54[index] = arg9;
    data_02108e60->basic_.unk_e0[index] = 1;
    data_02108e60->basic_.spriteDiffuseColor[index] = col4;
    data_02108e60->basic_.modelDiffuseColor[index] = col5;
    data_02108e60->basic_.edgeColor[index] = col6;

    return 1;
}

// Set fog data
int LightingScript_Opcode_69(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    if (index > 6)
        return 1;
    
    LightingManager::FogInfo entry;

    entry.unk_0 = (params++)->ToInt() != 0;
    entry.color = (params++)->ToInt();
    entry.type = (params++)->ToInt();
    entry.depthShift = (params++)->ToInt();
    entry.offset = (params++)->ToInt();
    for (int i = 0; i < 8; i++)
    {
        int packed = (params++)->ToInt();
        entry.densityTable[4 * i] = packed & 0xff;
        entry.densityTable[4 * i + 1] = (packed & 0xff00) >> 8;
        entry.densityTable[4 * i + 2] = (packed & 0xff0000) >> 16;
        entry.densityTable[4 * i + 3] = (packed & 0xff000000) >> 24;
    }
    entry.alpha = (params++)->ToInt();

    LightingInfo* mgr = data_02108e60;
    mgr->fogList.entries[index].unk_0 = entry.unk_0;
    mgr->fogList.entries[index].type = entry.type;
    mgr->fogList.entries[index].depthShift = entry.depthShift;
    mgr->fogList.entries[index].offset = entry.offset;
    mgr->fogList.entries[index].color = entry.color;
    mgr->fogList.entries[index].alpha = entry.alpha;
    COPY_ARRAY(mgr->fogList.entries[index].densityTable, entry.densityTable);

    return 1;
}

int LightingScript_Opcode_6a(Script::Parameter* params, int numParams)
{
    int index = (params++)->ToInt();
    LightingManager::Light3DConfig secondaryEntry;
    secondaryEntry.maybeEnabled = (params++)->ToInt();
    secondaryEntry.direction[0] = (params++)->ToFloat();
    secondaryEntry.direction[1] = (params++)->ToFloat();
    secondaryEntry.direction[2] = (params++)->ToFloat();
    secondaryEntry.color = (params++)->ToInt();
    data_02108e60->advanced_.light1[index] = secondaryEntry;

    LightingManager::Light3DConfig primaryEntry;
    primaryEntry.maybeEnabled = (params++)->ToInt();
    primaryEntry.direction[0] = (params++)->ToFloat();
    primaryEntry.direction[1] = (params++)->ToFloat();
    primaryEntry.direction[2] = (params++)->ToFloat();
    primaryEntry.color = (params++)->ToInt();
    data_02108e60->advanced_.light0[index] = primaryEntry;

    int col1 = (params++)->ToInt();
    int col2 = (params++)->ToInt();
    int col3 = (params++)->ToInt();
    int col4 = (params++)->ToInt();
    int col5 = (params++)->ToInt();
    int col6 = (params++)->ToInt();
    int col7 = (params++)->ToInt();
    data_02108e60->advanced_.backgroundColor[index] = col1;
    data_02108e60->advanced_.backgroundSecondColor[index] = col2;
    data_02108e60->advanced_.maybeAmbientColor[index] = col3;
    data_02108e60->advanced_.unk_142[index] = col4;
    data_02108e60->advanced_.spriteDiffuseColor[index] = col5;
    data_02108e60->advanced_.modelDiffuseColor[index] = col6;
    data_02108e60->advanced_.edgeColor[index] = col7;

    return 1;
}


void ExecuteLightingScript(const void* script, unsigned int length, LightingInfo* manager, SafeAllocator* alloc)
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

void LightingInfo::Initialize()
{
    maybeMode_ = 0;
    gradientCenterOffset_ = 0.0f;
    memset(&basic_, 0, sizeof(basic_));
    memset(&advanced_, 0, sizeof(advanced_));
    memset(&fogList, 0, sizeof(fogList));
}

void LightingInfo::Reset()
{
    maybeMode_ = 0;
    gradientCenterOffset_ = 0.0f;
    memset(&basic_, 0, sizeof(basic_));
    memset(&advanced_, 0, sizeof(advanced_));
    memset(&fogList, 0, sizeof(fogList));
}

void LightingInfo::LoadFromScript(const void* script, unsigned int length, SafeAllocator* alloc)
{
    ExecuteLightingScript(script, length, this, alloc);
}
