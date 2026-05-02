#include "Grotto/Main/DetailedTreasureMapData.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Grotto/Main/TreasureMapStructConversions.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "System/Memory.h"
#include "Grotto/Main/RandATRangeModular.h"
#include "std_library_functions.h"

#ifdef jpn
#define func_020a1df8 func_020a3b70
#define func_020a1e54 func_020a3bcc

#define func_0200fdcc func_0200fc28
#endif

extern "C"
{
    void func_020a1df8(unsigned int);
    void func_020a1e54(unsigned int);

    unsigned int func_0200fdcc(BattleStruct*);
    // zeroes out memory (not used in jpn version)
    void func_0200f374(void* where, unsigned int len);
    // copies character name into the buffer? (not used in jpn version)
    void func_020426bc(void*, char* buffer, int);

    // Based on where it's called, this is probably returning a language-
    // dependent string for "Lv. " (at least, if called with 1011 as arg).
    // Not used in jpn version
    const char* func_020e51cc(int);

    // seems to get the game language. In the USA version, if it would
    // return a value other than 2 or 5, it returns 1, which seems to reflect
    // lack of support for German & Italian.
    // not used in jpn version
    int func_0200fb08(BattleStruct*);

    // jpn exclusive:
    // seems to remove Furigana decorations from a string. These are stored
    // in the following format: [a/b] denotes the symbol a with b over the top
    void func_020a5c20(const char* input, char* output);
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

    GenerateNameBuffers();
    GeneratePopupName();
    
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

    // Retrieving the name of the player character I guess?
#ifndef jpn
    // Based on how the jpn version works, I would guess this is undoing the
    // custom text encoding (e.g. lowercase a is 0x2A vs ascii 0x61)
    void* playerRelatedPtr = *(void**)(func_0200fdcc(GetBattleStruct()) + 0x134);
    char asciiName[10]; // maybe 12
    func_0200f374(asciiName, 10);
    func_020426bc(playerRelatedPtr, asciiName, 1);
#else
    // 0200fc28 is the address in the japanese version
    char* asciiName = *(char**)(func_0200fc28(GetBattleStruct()) + 0x134);
#endif

    discoveryState = DiscoveryState_Cleared;

    VectorizedMemset(clearedBy, 0, 12);
    VectorizedInvertedMemcpy(asciiName, clearedBy, 10);

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

#if defined(usa)

extern const char data_020f1ac0[]; // "%s"
extern const char data_020f1ac3[]; // ""
extern const char data_020f1ac4[]; // "%s%d"
extern const char data_020f1ac9[]; // "%s %s"

#elif defined(jpn)

extern const char data_020f1c0c[]; // "%s no chizu"
extern const char data_020f1c15[]; // "%s no"
extern const char data_020f1c1a[]; // "[chizu/chizu]" (second is Furigana)
extern const char data_020f1c26[]; // ""
extern const char data_020f1c27[]; // "Lv %d"
extern const char data_020f1c2c[]; // "%s<W=3>%s"
extern const char data_020f1c36[]; // "%s Lv %d no [ma/ma]"

#endif

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
#if defined(jpn)
            break;
#endif
        }

        readOffset += readStringLen;
// in international versions, the binary file also stores strings such as
// "Baramos's Map", presumably because grammatical differences between multiple
// languages would be painful to implement in code
#if defined(usa) 
        TMAPLANGDATA_READ(readOffset, &readUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &readStringLen, 2);

        if (newBossID == readBossID)
        {
            VectorizedInvertedMemcpy(dataPtr + readOffset, mapNameNoLevel, readStringLen);
            mapNameNoLevel[readStringLen] = '\0';
            break;
        }
        readOffset += readStringLen;
#endif
    }

    if (bossEnemyID == 0)
        return;

    level = newLevel;
    minTurns = newMinTurns;

#if defined(usa)
    sprintf(mapNameNoLevel_v2, data_020f1ac0, mapNameNoLevel);
    strcpy(seeminglyEmptyBuffer, data_020f1ac3);
    sprintf(mapLevelString, data_020f1ac4, func_020e51cc(1011), level);
    sprintf(topScreenName, data_020f1ac9, mapNameNoLevel, mapLevelString);
    sprintf(popupName, data_020f1ac9, bossName, mapLevelString);
#elif defined(jpn)
    char bossNameUndecorated[256];

    func_020a5c20(bossName, bossNameUndecorated);
    sprintf(mapNameNoLevel, data_020f1c0c, bossNameUndecorated);
    sprintf(bossNameGenitive, data_020f1c15, bossName);
    strcpy(fixedStringChizu, data_020f1c1a);
    strcpy(mapLevelString, data_020f1c26);
    sprintf(mapLevelString, data_020f1c27, level);
    sprintf(topScreenName, data_020f1c2c, mapNameNoLevel, mapLevelString);
    sprintf(popupName, data_020f1c36, bossName, level);
#endif
}

// USA: func_020a451c
// JPN: func_020a62c8
void DetailedTreasureMapData::LegacyBossMapData::WriteMapLevelString()
{
#if defined(usa)
    const char* lvlPrefix = func_020e51cc(1011);
    sprintf(mapLevelString, data_020f1ac4, lvlPrefix, level);
#elif defined(jpn)
    sprintf(mapLevelString, data_020f1c27, level);
#endif
}

// USA: func_020a454c
// JPN: func_020a62e4
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
// JPN: func_020a6338
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
// JPN: func_020a63b0
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
// JPN: func_020a64d0
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
// JPN: func_020a65bc
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
// JPN: func_020a66f4
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

// JPN: func_020a682c
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
// JPN: func_020a6aa0
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
// JPN: func_020a5ba0
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
// JPN: func_020a6cd8
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
// JPN: func_020a6e10
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

#if defined(usa)

extern const char data_020f1ad0[]; // "%s%d"
extern const char data_020f1ad5[]; // "%s %s"
extern const unsigned char data_020e9074[]; // { 4, 13, 11 }

// USA: func_020a51b0
void DetailedTreasureMapData::RegularMapData::GenerateNameBuffers()
{    
    if (prefix == 0 || suffix == 0 || localeRank == 0 || level == 0)
        return;

    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    nameNoLevel[0] = '\0';

    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;

    // Important for register nonsense to declare these here
    int nameIdx;
    int readOffset;
    
    unsigned char* langData = GetTreasureMapLanguageData(GetBattleStruct());
    int* offsetArray = (int*)(func_ov017_0218b5b0()->pTMapLanguageOffsets);   

    // choose the order of the words based on the language
    unsigned char partOrder[3]; 
    switch (func_0200fb08(GetBattleStruct()))
    {
    // English & German
    // e.g. Granite (0) Tunnel (2) of Woe (1)
    case 1:
    case 3:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    // French, Italian, Spanish
    // e.g. Tunnel (2) of Granite (0) of Woe (1)
    case 2:
    case 4:
    case 5:
        partOrder[0] = 2;
        partOrder[1] = 0;
        partOrder[2] = 1;
        break;
    case 0:
    default:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    }
    
    unsigned char indices[3];
    indices[0] = prefix;
    indices[1] = suffix;
    indices[2] = localeRank;

    char tempBuffer[256];
    
    for (int i = 0; i < 3; i++)
    {
        int currentNamePart = partOrder[i];
        int ptrListIndex = data_020e9074[currentNamePart];
        
        readOffset = offsetArray[ptrListIndex];
        
        TMAPLANGDATA_READ(readOffset, &numEntries, 2);
         
        for (unsigned short entryLoop = 0; entryLoop < numEntries; entryLoop++)
        {
            nameIdx = indices[currentNamePart];
            if (ptrListIndex == 11)
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                for (unsigned short loopEnviron = 1; loopEnviron <= 5; loopEnviron++)
                {
                    TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                    TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                    if (environ == loopEnviron && nameIdx == readIndex)
                    {
                        VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                        tempBuffer[stringLength] = '\0';
                        strcat(nameNoLevel, tempBuffer);
                        entryLoop = numEntries; // hack to escape the j-level loop
                        break;
                    }
                    readOffset += stringLength;
                }
            }
            else
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                if (nameIdx == readIndex)
                {
                    VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                    tempBuffer[stringLength] = '\0';
                    strcat(nameNoLevel, tempBuffer);
                    break;
                }
                readOffset += stringLength;
            }
        }
    }

    sprintf(levelString, data_020f1ad0, func_020e51cc(1011), level);
    sprintf(topScreenName, data_020f1ad5, nameNoLevel, levelString);
}

extern const char data_020f1adb[]; // "%s%d"
extern const unsigned char data_020e9077[]; // { 4, 13, 11 }

// USA: func_
void DetailedTreasureMapData::RegularMapData::GeneratePopupName()
{    
    if (prefix == 0 || suffix == 0 || localeRank == 0 || level == 0)
        return;

    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    popupName[0] = '\0';

    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;

    // Important for register nonsense to declare these here
    int nameIdx;
    int readOffset;
    
    unsigned char* langData = GetTreasureMapLanguageData(GetBattleStruct());
    int* offsetArray = (int*)(func_ov017_0218b5b0()->pTMapLanguageOffsets);   

    // choose the order of the words based on the language
    unsigned char partOrder[3]; 
    switch (func_0200fb08(GetBattleStruct()))
    {
    // English & German
    // e.g. Granite (0) Tunnel (2) of Woe (1)
    case 1:
    case 3:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    // French, Italian, Spanish
    // e.g. Tunnel (2) of Granite (0) of Woe (1)
    case 2:
    case 4:
    case 5:
        partOrder[0] = 2;
        partOrder[1] = 0;
        partOrder[2] = 1;
        break;
    // Japanese
    case 0:
    default:
        partOrder[0] = 0;
        partOrder[1] = 2;
        partOrder[2] = 1;
        break;
    }
    
    unsigned char indices[3];
    indices[0] = prefix;
    indices[1] = suffix;
    indices[2] = localeRank;

    char tempBuffer[64];
    
    for (int i = 0; i < 3; i++)
    {
        int currentNamePart = partOrder[i];
        int ptrListIndex = data_020e9077[currentNamePart];
        
        readOffset = offsetArray[ptrListIndex];
        
        TMAPLANGDATA_READ(readOffset, &numEntries, 2);
         
        for (unsigned short entryLoop = 0; entryLoop < numEntries; entryLoop++)
        {
            nameIdx = indices[currentNamePart];
            if (ptrListIndex == 11)
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                for (unsigned short loopEnviron = 1; loopEnviron <= 5; loopEnviron++)
                {
                    TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                    TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                    if (environ == loopEnviron && nameIdx == readIndex)
                    {
                        VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                        tempBuffer[stringLength] = '\0';
                        strcat(popupName, tempBuffer);
                        entryLoop = numEntries; // hack to escape the j-level loop
                        break;
                    }
                    readOffset += stringLength;
                }
            }
            else
            {
                TMAPLANGDATA_READ(readOffset, &readIndex, 1);
                TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
                TMAPLANGDATA_READ(readOffset, &stringLength, 2);
                if (nameIdx == readIndex)
                {
                    VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                    tempBuffer[stringLength] = '\0';
                    strcat(popupName, tempBuffer);
                    break;
                }
                readOffset += stringLength;
            }
        }
    }

    sprintf(tempBuffer, data_020f1adb, func_020e51cc(1011), level);
    strcat(popupName, tempBuffer);
}

#elif defined(jpn)

extern const char data_020f1c48[];
extern const char data_020f1c4d[];
extern const char data_020f1c52[];

// JPN: func_020a6f48
void DetailedTreasureMapData::RegularMapData::GenerateNameBuffers()
{    
    if (prefix == 0 || suffix == 0 || localeRank == 0 || level == 0)
        return;

    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    nameNoLevel[0] = '\0';

    int readOffset;
    
    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;
    char tempBuffer[256];
    
    unsigned char* langData = GetTreasureMapLanguageData(GetBattleStruct());
    TreasureMapLanguageDataOffsets* offsetArray = func_ov017_0218b5b0()->pTMapLanguageOffsets;
    
    readOffset = offsetArray->prefixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (prefix == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(nameNoLevel, tempBuffer);
            strcpy(prefixString, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    readOffset = offsetArray->suffixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (suffix == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(nameNoLevel, tempBuffer);
            strcpy(suffixString, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    char nameUndecorated[256];
    func_020a5c20(nameNoLevel, nameUndecorated);
    strcat(nameUndecorated, data_020f1c48);
    strcpy(nameNoLevel, nameUndecorated);
    sprintf(levelString, data_020f1c4d, level);
    sprintf(topScreenName, data_020f1c52, nameNoLevel, levelString);
}

// JPN: func_020a71f8
void DetailedTreasureMapData::RegularMapData::GeneratePopupName()
{    
    if (prefix == 0 || suffix == 0 || localeRank == 0 || level == 0)
        return;

    if (GetTreasureMapLanguageData(GetBattleStruct()) == NULL)
        return;

    popupName[0] = '\0';

    int readOffset;
    
    // quantities read repeatedly from the binary file
    unsigned short numEntries = 0;
    unsigned char readIndex = 0;
    unsigned short stringUnknown = 0;
    unsigned short stringLength = 0;
    char tempBuffer[64];
    
    unsigned char* langData = GetTreasureMapLanguageData(GetBattleStruct());
    TreasureMapLanguageDataOffsets* offsetArray = func_ov017_0218b5b0()->pTMapLanguageOffsets;

    readOffset = offsetArray->prefixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (prefix == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(popupName, tempBuffer);
            strcpy(prefixString, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    readOffset = offsetArray->suffixNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &stringLength, 2);
        if (suffix == readIndex)
        {
            VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
            tempBuffer[stringLength] = '\0';
            strcat(popupName, tempBuffer);
            strcpy(suffixString, tempBuffer);
            break;
        }
        readOffset += stringLength;
    }

    readOffset = offsetArray->localeNames;
    TMAPLANGDATA_READ(readOffset, &numEntries, 2);
    for (unsigned short i = 0; i < numEntries; i++)
    {
        TMAPLANGDATA_READ(readOffset, &readIndex, 1);
        for (unsigned short loopEnviron = 1; loopEnviron <= 5; loopEnviron++)
        {
            TMAPLANGDATA_READ(readOffset, &stringUnknown, 2);
            TMAPLANGDATA_READ(readOffset, &stringLength, 2);
            if (environ == loopEnviron && localeRank == readIndex)
            {
                VectorizedInvertedMemcpy(langData + readOffset, tempBuffer, stringLength);
                tempBuffer[stringLength] = '\0';
                strcat(popupName, tempBuffer);
                strcpy(localeString, tempBuffer);
                i = numEntries; // hack to escape the outer loop
                break;
            }
            readOffset += stringLength;
        }
    }

    sprintf(tempBuffer, data_020f1c4d, level);
    strcat(popupName, tempBuffer);
}

#endif