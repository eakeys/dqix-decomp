#include "Grotto/Main/DetailedTreasureMapData.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Grotto/Main/TreasureMapStructConversions.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "System/Memory.h"
#include "Grotto/Main/RandATRangeModular.h"
#include "std_library_functions.h"

extern "C"
{
    void func_020a1df8(unsigned int);
    void func_020a1e54(int);

    // generates the name string for a regular map
    void func_020a51b0(DetailedTreasureMapData::RegularMapData*);
    // generates the popup name for a regular map
    void func_020a54d0(DetailedTreasureMapData::RegularMapData*);

    unsigned int func_0200fdcc(BattleStruct*);
    // zeroes out memory
    void func_0200f374(void* where, unsigned int len);
    // copies character name into the buffer?
    void func_020426bc(void*, char* buffer, int);

    // looks like sprintf. It seems like variadic arguments still
    // place the 2nd, 3rd, 4th parameter into r1, r2, r3
    int func_02003ce8(char* buffer, const char* fmt, ...);

    // Based on where it's called, this is probably returning a language-
    // dependent string for "Lv. " (at least, if called with 1011 as arg)
    const char* func_020e51cc(int);
}

#define TMAPLANGDATA_READ(offset, into, len) \
    (VectorizedInvertedMemcpy(GetTreasureMapLanguageData(GetBattleStruct()) + (offset), (into), (len)), offset += (len))

void DetailedTreasureMapData::RegularMapData::Populate(unsigned short newseed, unsigned char newquality)
{
    VectorizedMemset(this, 0, sizeof(RegularMapData));
    seed = newseed;
    srand(seed);
    quality = newquality;
    func_020a1df8(4);

    GenerateUnknownData();
    GenerateEnviron();
    GenerateFloorCount();
    GenerateMonsterRank();
    GenerateBoss();
    GenerateUnusedChestRanks();
    GeneratePrefix();
    GenerateSuffix();
    GenerateLocaleRank();

    int levelSum = floorCount + startingMonsterRank + bossID;
    int unclampedLevel = (rand() % 11) - 5 + (levelSum - 4) * 3;
    if (unclampedLevel <= 0)
        level = 1;
    else if (unclampedLevel > 99)
        level = 99;
    else
        level = unclampedLevel;

    func_020a51b0(this);
    func_020a54d0(this);
    
    func_020a1e54(1);
}

unsigned short DetailedTreasureMapData::LegacyBossMapData::MaybeGetCurrentAlternateID() const
{
    int idx = whichAlternateVersion;
    if (idx >= 1 && idx <= 3)
        return alternateVersionIDs[idx - 1];
    return 0;
}

void DetailedTreasureMapData::Clear()
{
    VectorizedMemset(this, 0, sizeof(DetailedTreasureMapData));
}

void DetailedTreasureMapData::BlankFunction() const {}

bool DetailedTreasureMapData::UpdateFollowingCompletion(bool levelledUp, unsigned short numTurns)
{
    if (mapType != TreasureMapType_Legacy)
        return false;

    // Some stuff to get the player character name, most likely
    void* playerRelatedPtr = *(void**)(func_0200fdcc(GetBattleStruct()) + 0x134);
    char nameBuffer[10]; // maybe 12
    func_0200f374(nameBuffer, 10);
    func_020426bc(playerRelatedPtr, nameBuffer, 1);

    discoveryState = DiscoveryState_Cleared;

    VectorizedMemset(clearedBy, 0, 12);
    VectorizedInvertedMemcpy(nameBuffer, clearedBy, 10);

    if (levelledUp && legacy.newDropListAtNextLevel)
    {
        discoveredTreasures[0] = true;
        discoveredTreasures[1] = false;
        discoveredTreasures[2] = false;
    }

    if (levelledUp)
    {
        legacy.level++;
        if (legacy.level > 99)
            legacy.level = 99;
        legacy.minTurns = 0;   
    }
    else
    {
        if (numTurns < 1000 && (legacy.minTurns == 0 || numTurns < legacy.minTurns))
        {
            legacy.minTurns = numTurns;
            return true;
        }
    }

    legacy.WriteMapLevelString();
    GrottoStruct* grotto = GetGrottoStruct(GetBattleStruct());
    strcpy(grotto->activeMapNameNoLevel, legacy.mapNameNoLevel);
    grotto->activeMapLevel = legacy.level;
    return false;
}

unsigned int DetailedTreasureMapData::GetLevel() const
{
    unsigned int ret = 1;
    if (mapType == TreasureMapType_Regular)
        ret = regular.level;
    else if (mapType == TreasureMapType_Legacy)
        ret = legacy.level;
    return ret;
}

void GrottoStruct::LoadActiveMetadataFromDetailed(DetailedTreasureMapData* from)
{
    ExportTreasureMapMetadata(from, &activeMapData);
}

extern const char data_020f1ac0[]; // "%s"
extern const char data_020f1ac3[]; // ""
extern const char data_020f1ac4[]; // "%s%d"
extern const char data_020f1ac9[]; // "%s %s"

// USA: func_020a425c
void DetailedTreasureMapData::LegacyBossMapData::Populate(
    unsigned char newBossID, unsigned char newLevel, unsigned short newMinTurns)
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == 0)
        return;

    bossEnemyID = 0;
    bossID = newBossID;

    for (int i = 0; i < 3; i++)
        alternateVersionIDs[i] = 0;

    unsigned short numEntries = 0;

    unsigned char readBossID = 0;
    unsigned short readEnemyID = 0;
    unsigned short readAlternates[3];
    unsigned short readUnknown = 0; // seems to precede every string and always = 8
    unsigned short readStringLen = 0;

    TreasureMapLanguageDataOffsets* langData = func_ov017_0218b5b0()->pTMapLanguageOffsets;
    unsigned char* dataPtr = GetTreasureMapLanguageData(GetBattleStruct());

    int readOffset = langData->legacyBossData;
    
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (int i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readBossID, 2);
        TMAPLANGDATA_READ(readOffset, &readEnemyID, 2);
        for (int j = 0; j < 3; j++)
        {
            TMAPLANGDATA_READ(readOffset, &readAlternates[j], 2);
        }

        TMAPLANGDATA_READ(readOffset, &readUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &readStringLen, 2);

        if (newBossID == readBossID)
        {
            bossEnemyID = readEnemyID;
            for (int j = 0; j < 3; j++)
                alternateVersionIDs[j] = readAlternates[j];
            VectorizedInvertedMemcpy(dataPtr + readOffset, bossName, readStringLen);
            bossName[readStringLen] = '\0';
        }

        readOffset += readStringLen;
        TMAPLANGDATA_READ(readOffset, &readUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &readStringLen, 2);

        if (newBossID == readBossID)
        {
            VectorizedInvertedMemcpy(dataPtr + readOffset, mapNameNoLevel, readStringLen);
            mapNameNoLevel[readStringLen] = '\0';
            break;
        }
        readOffset += readStringLen;
    }

    if (bossEnemyID == 0)
        return;

    level = newLevel;
    minTurns = newMinTurns;
    func_02003ce8(mapNameNoLevel_v2, data_020f1ac0, mapNameNoLevel);
    strcpy(seeminglyEmptyBuffer, data_020f1ac3);
    func_02003ce8(mapLevelString, data_020f1ac4, func_020e51cc(1011), level);
    func_02003ce8(mapName, data_020f1ac9, mapNameNoLevel, mapLevelString);
    func_02003ce8(popupName, data_020f1ac9, bossName, mapLevelString);
}

// USA: func_020a451c
void DetailedTreasureMapData::LegacyBossMapData::WriteMapLevelString()
{
    const char* lvlPrefix = func_020e51cc(1011);
    func_02003ce8(mapLevelString, data_020f1ac4, lvlPrefix, level);
}

// USA: func_020a454c
bool DetailedTreasureMapData::LegacyBossMapData::CanUseLevelUpMove(unsigned short id)
{
    for (int i = 0; i < numLevelUpMoves; i++)
    {
        LevelUpMove* move = &levelUpMoves[i];
        if (move->moveID == id)
            return move->level <= this->level;
    }

    // If not a level up move, treat as having it by default
    return true;
}

// USA: func_020a45a0
unsigned short DetailedTreasureMapData::LegacyBossMapData::GetLearnedMove(unsigned char atLevel, int filter)
{
    for (int i = 0; i < numLevelUpMoves; i++)
    {
        LevelUpMove* move = &levelUpMoves[i];
        if (move->level != atLevel)
            continue;

        if (filter == 0)
        {
            if (move->announceLearn != false)
                continue;
            return move->moveID;
        }
        else if (filter == 1)
        {
            if (move->announceLearn != true)
                continue;
            return move->moveID;
        }
        return move->moveID;
    }

    return 0;
}

// USA: func_020a4618
void DetailedTreasureMapData::RegularMapData::GenerateUnknownData()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numValues = 0;
    unsigned char index = 0;
    unsigned char value = 1;
    unsigned char chance = 0;

    unknown_66 = 1;
    unknown_4F = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->unknown_18;

    TMAPLANGDATA_READ(readOffset, &numValues, 2); // numValues = 12
    for (unsigned short i = 0; i < numValues; i++)
    {
        // index = 1, 2, ..., 12
        TMAPLANGDATA_READ(readOffset, &index, 1);
        // values seem to be: 5,5,3,2,3,3,2,2,3,2,2,2
        TMAPLANGDATA_READ(readOffset, &value, 1);
        // all zero
        TMAPLANGDATA_READ(readOffset, &chance, 1);

        if (rand() % 100 < (int)chance)
        {
            unknown_66 = value;
            unknown_4F = index;
            return;
        }
    }
}

// USA: func_020a4738
void DetailedTreasureMapData::RegularMapData::GenerateEnviron()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readEnviron = 0;
    unsigned char readChance = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->environs;

    unsigned char percentile = 0;

    unsigned char rngValue = rand() % 100;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readEnviron, 1);
        TMAPLANGDATA_READ(readOffset, &readChance, 1);

        if (rngValue < readChance + percentile)
        {
            this->environ = readEnviron;
            return;
        }

        percentile += readChance;
    }
}

// USA: func_020a4824
void DetailedTreasureMapData::RegularMapData::GenerateFloorCount()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinQuality = 0;
    unsigned char readMaxQuality = 0;
    unsigned char readMinFloors = 0;
    unsigned char readMaxFloors = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->floorRangesByQuality;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMinFloors, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxFloors, 1);

        if (readMinQuality <= quality && quality <= readMaxQuality)
        {
            floorCount = RandATRangeModular(readMinFloors, readMaxFloors);
            return;
        }
    }
}

// USA: func_020a495c
void DetailedTreasureMapData::RegularMapData::GenerateMonsterRank()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinQuality = 0;
    unsigned char readMaxQuality = 0;
    unsigned char readMinRank = 0;
    unsigned char readMaxRank = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->startingMonsterRanksByQuality;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMinRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxRank, 1);

        if (readMinQuality <= quality && quality <= readMaxQuality)
        {
            startingMonsterRank = RandATRangeModular(readMinRank, readMaxRank);
            return;
        }
    }
}

void DetailedTreasureMapData::RegularMapData::GenerateBoss()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinQuality = 0;
    unsigned char readMaxQuality = 0;
    unsigned char readMinBoss = 0;
    unsigned char readMaxBoss = 0;

    TreasureMapLanguageDataOffsets* offsets = func_ov017_0218b5b0()->pTMapLanguageOffsets;
    int readOffset = offsets->bossRangesByQuality;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxQuality, 1);
        TMAPLANGDATA_READ(readOffset, &readMinBoss, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxBoss, 1);

        if (readMinQuality > quality || quality > readMaxQuality)
            continue;

        int weightReadOffset;
        unsigned short bossTotalWeight = 0;
        unsigned char readBossNumber = 0;
        unsigned short readBossEnemyID = 0;
        unsigned char readBossWeight = 0;

        for (unsigned short i = readMinBoss; i <= readMaxBoss; i++)
        {
            weightReadOffset = (i - 1) * 4 + 2 + offsets->bossIDsAndWeights;
            TMAPLANGDATA_READ(weightReadOffset, &readBossNumber, 1);
            TMAPLANGDATA_READ(weightReadOffset, &readBossEnemyID, 2);
            TMAPLANGDATA_READ(weightReadOffset, &readBossWeight, 1);
            bossTotalWeight += readBossWeight;
        }

        unsigned int rng = rand() % bossTotalWeight;

        unsigned short weightAccumulator = 0;
        for (unsigned short i = readMinBoss; i <= readMaxBoss; i++)
        {
            weightReadOffset = 4 * (i - 1) + 2 + offsets->bossIDsAndWeights;
            TMAPLANGDATA_READ(weightReadOffset, &readBossNumber, 1);
            TMAPLANGDATA_READ(weightReadOffset, &readBossEnemyID, 2);
            TMAPLANGDATA_READ(weightReadOffset, &readBossWeight, 1);

            if (rng < readBossWeight + weightAccumulator)
            {
                bossID = i;
                bossEnemyId = readBossEnemyID;
                return;
            }

            weightAccumulator += readBossWeight;
        }
    }
}

// USA: func_020a4d08
void DetailedTreasureMapData::RegularMapData::GenerateUnusedChestRanks()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->seeminglyChestRanksByMonsterRank;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);

    if (numEntries != 12)
        return;

    for (int i = 0; i < 12; i++)
    {
        unsigned char readMonsterRank = 0;
        unsigned char readMinChestRank = 0;
        unsigned char readMaxChestRank = 0;

        TMAPLANGDATA_READ(readOffset, &readMonsterRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMinChestRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxChestRank, 1);

        maybeUnusedChestRanks[i] = RandATRangeModular(readMinChestRank, readMaxChestRank);
    }
}

// USA: func_020a4e08
void DetailedTreasureMapData::RegularMapData::GeneratePrefix()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinMonsterRank = 0;
    unsigned char readMaxMonsterRank = 0;
    unsigned char readMinPrefixIdx = 0;
    unsigned char readMaxPrefixIdx = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->prefixRangesByMonsterRank;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinMonsterRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxMonsterRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMinPrefixIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxPrefixIdx, 1);

        if (readMinMonsterRank <= startingMonsterRank &&
            startingMonsterRank <= readMaxMonsterRank)
        {
            prefix = RandATRangeModular(readMinPrefixIdx, readMaxPrefixIdx);
            return;
        }
    }
}

// USA: func_020a4f40
void DetailedTreasureMapData::RegularMapData::GenerateSuffix()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinBossIdx = 0;
    unsigned char readMaxBossIdx = 0;
    unsigned char readMinSuffixIdx = 0;
    unsigned char readMaxSuffixIdx = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->suffixRangesByBoss;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinBossIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxBossIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMinSuffixIdx, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxSuffixIdx, 1);

        if (readMinBossIdx <= bossID &&
            bossID <= readMaxBossIdx)
        {
            suffix = RandATRangeModular(readMinSuffixIdx, readMaxSuffixIdx);
            return;
        }
    }
}

// USA: func_020a5078
void DetailedTreasureMapData::RegularMapData::GenerateLocaleRank()
{
    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    unsigned short numEntries = 0;
    unsigned char readMinFloorCount = 0;
    unsigned char readMaxFloorCount = 0;
    unsigned char readMinLocaleRank = 0;
    unsigned char readMaxLocaleRank = 0;

    int readOffset = func_ov017_0218b5b0()->pTMapLanguageOffsets->localeRankRangesByFloorCount;

    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readMinFloorCount, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxFloorCount, 1);
        TMAPLANGDATA_READ(readOffset, &readMinLocaleRank, 1);
        TMAPLANGDATA_READ(readOffset, &readMaxLocaleRank, 1);

        if (readMinFloorCount <= floorCount &&
            floorCount <= readMaxFloorCount)
        {
            localeRank = RandATRangeModular(readMinLocaleRank, readMaxLocaleRank);
            return;
        }
    }
}