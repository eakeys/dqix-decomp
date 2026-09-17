#include "Grotto/Main/TreasureMapDataStructs.h"
#include "System/Memory.h"
#include "std_library_functions.h"
#include "GameState/GameState.h"
#include <globaldefs.h>

#ifdef jpn
#define func_020100a8 func_0200ff04
#define func_0200ff1c func_0200fd78
#define func_02012fe4 func_02012dac
#endif

extern "C"
{

unsigned int func_020100a8(GameState*);
char* func_0200ff1c(GameState*, unsigned int);

// returns the overland zone instance
void* func_02012fe4();
}

// USA: func_020a5cb8
// JPN: func_020a7a5c
unsigned short GenerateNewMapQuality()
{
    GameState* gameState = GameState::GetInstance();
    PartyMember* protag = (PartyMember*)gameState->GetObject100(func_020100a8(gameState));
    // Another pointless function call
    (void)func_02012fe4();
    GrottoStruct* grotto = gameState->GetGrottoStruct();

    unsigned short maxCharLevel = 0;
    unsigned short maxNumRevocs = 0;
    for (int i = 0; i < 13; i++)
    {
        // this cast is necessary lol
        unsigned char j = i;
        unsigned short level = protag->partyMemberStats_->vocationLevels[j];
        unsigned short revocCount = protag->partyMemberStats_->revocationCounts[j];
        if (level > maxCharLevel)
            maxCharLevel = level;
        if (revocCount > maxNumRevocs)
            maxNumRevocs = revocCount;
    }

    if (maxCharLevel > 99)
        maxCharLevel = 99;

    if (maxNumRevocs > 10)
        maxNumRevocs = 10;

    unsigned short quality;
    // Probably checking if regular map or legacy boss map?
    if (grotto->unknown_9 == 2)
    {
        quality = maxCharLevel + maxNumRevocs * 5 + grotto->activeMapLevel;
    }
    else
    {
        quality = (unsigned short)(1.5f * (float)maxCharLevel + 5.0f * (float)maxNumRevocs);
    }
    grotto->unknown_9 = 0;
    float tenth = 0.1f * (float)quality;
    int quotient = 2 * (int)tenth + 1;
    quality += (int)((float)(rand() % quotient) - tenth);
    if (quality < 2)
        quality = 2;
    if (quality > 248)
        quality = 248;
    return quality;
}

// USA: func_020a5e10
// JPN: func_020a7bb4
unsigned short GenerateMapLocation(unsigned int quality)
{
    if (quality >= 81 && quality <= 248)
        return RandATRangeModular(1, 150);
    if (quality >= 51 && quality <= 80)
        return RandATRangeModular(1, 131);
    return RandATRangeModular(1, 47);
}

// USA: func_020a5e7c
// JPN: func_020a7c20
void TreasureMapMetadata::InitialiseAsNonLegacyMap(unsigned int quality, int seed)
{
    VectorizedMemset(this, 0, 28);
    SetDiscoveryState(DiscoveryState_Undiscovered);
    SetMapType(TreasureMapType_Regular);
    if (quality >= 2 && quality <= 248)
    {
        QualityOrLegacyBossID = (unsigned char)quality;
    }
    else
    {
        QualityOrLegacyBossID = (unsigned char)GenerateNewMapQuality();
    }

    if (seed == 0)
    {
        SeedOrMinTurns = rand() % 0xffff;
    }
    else
    {
        SeedOrMinTurns = seed;
    }

    Location = (unsigned char)GenerateMapLocation(QualityOrLegacyBossID);
}

// USA: func_020a5efc
// JPN: func_020a7ca0
void TreasureMapMetadata::InitialiseAsLegacyBossMap(unsigned int bossID, unsigned int level)
{
    VectorizedMemset(this, 0, 28);
    SetDiscoveryState(DiscoveryState_Undiscovered);
    SetMapType(TreasureMapType_Legacy);
    if (bossID == 0 || bossID > 13)
    {
        QualityOrLegacyBossID = 1;
    }
    else
    {
        QualityOrLegacyBossID = (unsigned char)bossID;
    }

    if (level == 0)
    {
        LegacyBossLevel = 1;
    }
    else if (level > 99)
    {
        LegacyBossLevel = 99;
    }
    else
    {
        LegacyBossLevel = (unsigned char)level;
    }
    SeedOrMinTurns = 0;
    
    int finalQuality = GenerateNewMapQuality();
    Location = (unsigned char)GenerateMapLocation(finalQuality);
}

// USA: func_020a5f88
// JPN: func_020a7d2c
void TreasureMapMetadata::SetDiscoveryState(eDiscoveryState state)
{
    DiscoveryStateAndMapTypeAndUnknown &= 0xF8;
    if (state == 1)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x01;
    }
    else if (state == 2)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x02;
    }
    else if (state == 3)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x04;
    }
}

// USA: func_020a5fd0
// JPN: func_020a7d74
eDiscoveryState TreasureMapMetadata::GetDiscoveryState() const
{
    if (DiscoveryStateAndMapTypeAndUnknown & 0x01)
        return DiscoveryState_Undiscovered;
    if (DiscoveryStateAndMapTypeAndUnknown & 0x02)
        return DiscoveryState_Discovered;
    if (DiscoveryStateAndMapTypeAndUnknown & 0x04)
        return DiscoveryState_Cleared;
    return DiscoveryState_Invalid;
}

// USA: func_020a5ffc
// JPN: func_020a7da0
void TreasureMapMetadata::SetMapType(eTreasureMapType type)
{
    DiscoveryStateAndMapTypeAndUnknown &= 0xe7;
    if (type == TreasureMapType_Regular)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x08;
    }
    else if (type == TreasureMapType_Legacy)
    {
        DiscoveryStateAndMapTypeAndUnknown |= 0x10;
    }
}

// USA: func_020a6030
// JPN: func_020a7dd4
eTreasureMapType TreasureMapMetadata::GetMapType() const
{
    if (DiscoveryStateAndMapTypeAndUnknown & 0x08)
        return TreasureMapType_Regular;
    if (DiscoveryStateAndMapTypeAndUnknown & 0x10)
        return TreasureMapType_Legacy;
    return TreasureMapType_Invalid;
}

// USA: func_020a6050
// JPN: func_020a7df4
void TreasureMapMetadata::SetInitialByteUnknownBit()
{
    DiscoveryStateAndMapTypeAndUnknown |= 0x20;
}

// USA: func_020a6060
// JPN: func_020a7e04
void TreasureMapMetadata::ClearInitialByteUnknownBit()
{
    DiscoveryStateAndMapTypeAndUnknown &= 0xdf;
}

// USA: func_020a6070
// JPN: func_020a7e14
bool TreasureMapMetadata::GetInitialByteUnknownBit() const
{
    return DiscoveryStateAndMapTypeAndUnknown & 0x20;
}