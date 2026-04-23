#include "Grotto/Main/TreasureMapStructConversions.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Combat/Main/BattleList.h"
#include "System/Memory.h"
#include "Grotto/Overlay_17/Struct44C8.h"

#ifdef jpn
#define func_020a3fd8 func_020a5df0
#define func_020a425c func_020a6050
#define func_020a1df8 func_020a3b70
#define func_020a57e4 func_020a7588
#define func_020a5b1c func_020a78c0
#define func_020a1e54 func_020a3bcc
#endif

extern "C"
{
    // populates regular map data
    void func_020a3fd8(void*, unsigned short, unsigned char);
    // populates legacy map data
    void func_020a425c(void*, unsigned int, unsigned int, unsigned int);

    // unknown
    void func_020a1df8(unsigned int);
    // if the data is for a legacy map, loads the boss' stats
    void func_020a57e4(DetailedTreasureMapData*, int, int);
    // loads loot data for the map
    void func_020a5b1c(DetailedTreasureMapData*);
    // unknown
    void func_020a1e54(int);
}

#define TMAPLANGDATA_READ(offset, into, len) \
    (VectorizedInvertedMemcpy(GetTreasureMapLanguageData(GetBattleStruct()) + (offset), (into), (len)), offset += (len))

bool ExportDetailedTreasureMapData(const TreasureMapMetadata* from, DetailedTreasureMapData* to, int p3, int p4)
{
    Struct_ov017_44C8* oddStruct = func_ov017_0218b5b0();

    if (from == NULL || to == NULL)
        return false;

    if (from->DiscoveryStateAndMapTypeAndUnknown == 0)
        return false;

    VectorizedMemset(to, 0, sizeof(DetailedTreasureMapData));
    if (from->DiscoveryStateAndMapTypeAndUnknown & 0x01)
        to->discoveryState = DiscoveryState_Undiscovered;
    else if (from->DiscoveryStateAndMapTypeAndUnknown & 0x02)
        to->discoveryState = DiscoveryState_Discovered;
    else if (from->DiscoveryStateAndMapTypeAndUnknown & 0x04)
        to->discoveryState = DiscoveryState_Cleared;

    if (from->DiscoveryStateAndMapTypeAndUnknown & 0x08)
        to->mapType = TreasureMapType_Regular;
    else if (from->DiscoveryStateAndMapTypeAndUnknown & 0x10)
        to->mapType = TreasureMapType_Legacy;

    VectorizedMemset(to->discoveredBy, 0, 12);
    VectorizedMemset(to->clearedBy, 0, 12);

    VectorizedInvertedMemcpy(from->DiscoveredBy, to->discoveredBy, 10);
    VectorizedInvertedMemcpy(from->ClearedBy, to->clearedBy, 10);

    if (GetTreasureMapLanguageData(GetBattleStruct()) == 0)
        return false;

    int readOffset = oddStruct->pTMapLanguageOffsets->mapLocations;
    
    unsigned short numEntries;
    TMAPLANGDATA_READ(readOffset, &numEntries, sizeof(numEntries));
    
    for (int i = 1; i <= numEntries; i++)
    {
        unsigned short unused8;
        unsigned short stringLength;
        TMAPLANGDATA_READ(readOffset, &unused8, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        TMAPLANGDATA_READ(readOffset, to->mapImageName, stringLength);
        to->mapImageName[stringLength] = '\0';

        TMAPLANGDATA_READ(readOffset, &to->entranceZoneID, 4);
        TMAPLANGDATA_READ(readOffset, &to->entranceX, 4);
        TMAPLANGDATA_READ(readOffset, &to->entranceY, 4);
        TMAPLANGDATA_READ(readOffset, &to->entranceZ, 4);

        if (i == from->Location)
            break;
    }

    to->mapLocation = from->Location;
    if (to->mapType == TreasureMapType_Regular)
    {
        VectorizedMemset(&to->regular, 0, sizeof(to->regular));
        func_020a3fd8(&to->regular, from->SeedOrMinTurns, from->QualityOrLegacyBossID);
    }
    else if (to->mapType == TreasureMapType_Legacy)
    {
        VectorizedMemset(&to->legacy, 0, sizeof(to->legacy));
        func_020a425c(&to->legacy, from->QualityOrLegacyBossID, from->LegacyBossLevel, from->SeedOrMinTurns);
    }

    func_020a1df8(4);
    func_020a57e4(to, p3, p4);
    func_020a5b1c(to);
    func_020a1e54(1);

    for (int i = 0; i < 3; i++)
        to->discoveredTreasures[i] = false;

    if (from->TreasureDiscoveryFlags & 1)
        to->discoveredTreasures[0] = true;
    if (from->TreasureDiscoveryFlags & 2)
        to->discoveredTreasures[1] = true;
    if (from->TreasureDiscoveryFlags & 4)
        to->discoveredTreasures[2] = true;

    for (int i = 0; i < 3; i++)
    {
        if (to->treasureDropRates[i] == 100 && to->discoveryState != DiscoveryState_Undiscovered)
            to->discoveredTreasures[i] = true;
    }
    
    return true;
}