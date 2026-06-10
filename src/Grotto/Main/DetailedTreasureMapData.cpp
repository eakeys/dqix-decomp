#include "Grotto/Main/TreasureMapDataStructs.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "Filesystem/FileIO.h"
#include "System/Memory.h"
#include "std_library_functions.h"
#include <asmhacks.h>

#ifdef jpn
#define func_020a1df8 func_020a3b70
#define func_020a1e54 func_020a3bcc

#define func_0200fdcc func_0200fc28

#define func_0202f7c8 func_0202f338
#define func_0202f7e8 func_0202f358

#define func_02075098 func_02076224
#define func_02075248 func_02076378

#define data_020f1ae4 data_020f1c5c
#define data_020f1af8 data_020f1c70
#define data_0211e33c data_0211fb64
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

    void func_0202f7c8();
    void func_0202f7e8();
}

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GetTreasureMapLanguageData(GetBattleStruct()), offset, into, len)

#ifdef jpn

// JPN: func_020a5c20
void RemoveFurigana(const char* src, char* dst)
{  
    if (dst == NULL)
        return;

    VectorizedMemset(dst, 0, 4);

    unsigned int mode = 0;
    int srcIndex;
    int dstIndex = 0;
    
    for (srcIndex = 0; src[srcIndex] != '\0'; srcIndex++)
    {
        int inChar = src[srcIndex];

        if (inChar == '[')
        {
            mode = 1;
            continue;
        }
        
        if (inChar == '/')
        {
            mode = 2;
            continue;
        }
        
        if (inChar == ']')
        {
            mode = 0;
            continue;
        }
        
        if (mode <= 1)
        {
            unsigned char castChar = inChar;
            bool twoBytes;
            if ((castChar >= 0x81 && castChar <= 0x9F) || (castChar >= 0xE0 && castChar <= 0xFC))
                twoBytes = true;
            else
                twoBytes = false;

            if (twoBytes)
            {
                // totally normal code, nothing to see here
                // (we need it to force some register placements)
                int srcIndexPlusOne = srcIndex + 1;
                dst[dstIndex] = src[srcIndexPlusOne - 1];
                srcIndex++; // note srcIndex = srcIndexPlusOne doesn't work
                dstIndex++;
            }

            dst[dstIndex] = src[srcIndex];
            dstIndex++;
        }
    }
    
    dst[dstIndex] = 0;
}

#endif

// USA: func_020a3ee4
// JPN: func_020a5cfc
bool IsMonsterIDLegacyBoss(unsigned short id)
{
    // v1 form of Dragonlord (500) through Mortamor (508)
    if (id >= 500 && id <= 508)
        return true;

    // v1 form of Orgodemir, Dhoulmagus, Rhapthorne, Nokturnus
    if (id >= 511 && id <= 514)
        return true;

    // alternate patterns for all bosses except Nokturnus
    if (id >= 600 && id <= 660)
        return true;

    // alternate patterns for Nokturnus
    if (id >= 769 && id <= 775)
        return true;

    return false;
}

struct MapTypeEntry
{
    unsigned short itemID;
    unsigned char legacyBossID;
    bool isLegacy;
};

extern const MapTypeEntry treasureMapItemIDs[];
const MapTypeEntry treasureMapItemIDs[] = {
    { 1000, 0, false },  // regular map
    { 1001, 1, true },   // Dragonlord
    { 1002, 2, true },   // Malroth
    { 1003, 3, true },   // Baramos
    { 1004, 4, true },   // Zoma
    { 1005, 5, true },   // Psaro (comes before Estark here)
    { 1006, 6, true },   // Estark
    { 1007, 7, true },   // Nimzo
    { 1008, 8, true },   // Murdaw
    { 1009, 9, true },   // Mortamor
    { 1010, 10, true },  // Nokturnus (this time he's not at the end)
    { 1011, 11, true },  // Orgodemir
    { 1012, 12, true },  // Dhoulmagus
    { 1013, 13, true },  // Rhapthorne
    { 0 }
};

// USA: func_020a3f54
// JPN: func_020a5d6c
bool GetTreasureMapTypeFromItemID(unsigned short itemID, unsigned char* out)
{
    if (out == NULL)
        return false;

    for (int i = 0; treasureMapItemIDs[i].itemID != 0; i++)
    {
        const MapTypeEntry* entry = &treasureMapItemIDs[i];
        
        unsigned short thisID = entry->itemID;
        if (thisID == 0 || itemID != thisID)
            continue;
        
        unsigned char mapType;
        unsigned char thisLegacyBossID;
        unsigned short isLegacy;

        if (thisID == 1000)
        {
            isLegacy = entry->isLegacy;
            thisLegacyBossID = entry->legacyBossID;
            mapType = TreasureMapType_Regular;
        }
        else
        {
            isLegacy = entry->isLegacy;
            thisLegacyBossID = entry->legacyBossID;
            mapType = TreasureMapType_Legacy;
        }

        out[0] = mapType;
        out[1] = thisLegacyBossID;
        *(unsigned short*)(out + 2) = isLegacy;
        
        return true;
    }
    
    return false;
}

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
    int idx = stats.alternateVersion;
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

    if (levelledUp && legacy.stats.newDropListAtNextLevel)
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

    bossMonsterID = 0;
    bossID = newBossID;

    for (int i = 0; i < 3; i++)
        alternateVersionIDs[i] = 0;

    unsigned short numEntries = 0;

    unsigned char readBossID = 0;
    unsigned short readMonsterID = 0;
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
        TMAPLANGDATA_READ(readOffset, &readMonsterID, 2);
        for (int j = 0; j < 3; j++)
        {
            TMAPLANGDATA_READ(readOffset, &readAlternates[j], 2);
        }

        TMAPLANGDATA_READ(readOffset, &readUnknown, 2);
        TMAPLANGDATA_READ(readOffset, &readStringLen, 2);

        if (newBossID == readBossID)
        {
            bossMonsterID = readMonsterID;
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

    if (bossMonsterID == 0)
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

    RemoveFurigana(bossName, bossNameUndecorated);
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
    for (int i = 0; i < stats.numLevelUpMoves; i++)
    {
        LegacyBossStats::LevelUpMove* move = &stats.levelUpMoves[i];
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
    for (int i = 0; i < stats.numLevelUpMoves; i++)
    {
        LegacyBossStats::LevelUpMove* move = &stats.levelUpMoves[i];
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
        unsigned short readMonsterID = 0;
        unsigned char readBossWeight = 0;

        for (unsigned short i = readMinBoss; i <= readMaxBoss; i++)
        {
            weightReadOffset = (i - 1) * 4 + 2 + offsets->bossIDsAndWeights;
            TMAPLANGDATA_READ(weightReadOffset, &readBossNumber, 1);
            TMAPLANGDATA_READ(weightReadOffset, &readMonsterID, 2);
            TMAPLANGDATA_READ(weightReadOffset, &readBossWeight, 1);
            bossTotalWeight += readBossWeight;
        }

        unsigned int rng = rand() % bossTotalWeight;

        unsigned short weightAccumulator = 0;
        for (unsigned short i = readMinBoss; i <= readMaxBoss; i++)
        {
            weightReadOffset = 4 * (i - 1) + 2 + offsets->bossIDsAndWeights;
            TMAPLANGDATA_READ(weightReadOffset, &readBossNumber, 1);
            TMAPLANGDATA_READ(weightReadOffset, &readMonsterID, 2);
            TMAPLANGDATA_READ(weightReadOffset, &readBossWeight, 1);

            if (rng < readBossWeight + weightAccumulator)
            {
                bossID = i;
                bossMonsterID = readMonsterID;
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

extern char data_020f1ad0[]; // "%s%d"
extern char data_020f1ad5[]; // "%s %s"

extern const unsigned char data_020e9074[]; // { 4, 13, 11 }
extern const unsigned char data_020e9077[]; // { 4, 13, 11 }

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

extern char data_020f1adb[]; // "%s%d"

// USA: func_020a54d0
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

extern char data_020f1c48[];
extern char data_020f1c4d[];
extern char data_020f1c52[];

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
    RemoveFurigana(nameNoLevel, nameUndecorated);
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

extern char data_020f1ae4[];
extern char data_020f1af8[];

extern unsigned char data_0211e33c[];

void DetailedTreasureMapData::LoadLegacyBossStats(bool compute, const unsigned char* providedArchive)
{
    if (mapType != TreasureMapType_Legacy)
        return;

    if (!compute)
    {
        legacy.stats.dropListIndex = 0;
        legacy.stats.newDropListAtNextLevel = false;
        return;
    }
    
    func_0202f7c8();
    unsigned int archiveSize = 0;
    const unsigned char* usedArchive = data_0211e33c;
    
    if (providedArchive)
        usedArchive = providedArchive;
    else
    {
        if (!LoadFileIntoMemory(data_020f1ae4, const_cast<unsigned char*>(usedArchive), &archiveSize))
        {
            func_0202f7e8();
            return;
        }
    }

    char innerFileName[256];
    sprintf(innerFileName, data_020f1af8, legacy.bossMonsterID);
    const unsigned char* innerFileData;
    unsigned int innerFileSize;
    if (!GetFileInNarc(usedArchive, innerFileName, reinterpret_cast<const void**>(&innerFileData), &innerFileSize, 0))
    {
        func_0202f7e8();
        return;
    }

    func_0202f7e8();

    const unsigned char* copyOfInnerFilePtr;
    unsigned int loadlevel = legacy.level;
    // I would very much like to see this horrible hack removed, but it does the trick
    // and without it, the compiler optimises out the constant 0x18. This way it gets
    // kept in a register until the end.
    unsigned int stride;
    __asm("mov stride, 0x18");
    copyOfInnerFilePtr = innerFileData; // forces innerFileData to get loaded into a register here
    __asm("sub loadlevel, loadlevel, 1");
    DECLARE_ASM_NOP();
    
    unsigned int offset = stride * loadlevel;
    
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.maxHP, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.maxMP, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.agility, 2);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.attack, 2);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.defense, 2);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.alternateVersion, 1);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.rewardExp, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.rewardGold, 4);
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.dropListIndex, 1);
    
    legacy.stats.newDropListAtNextLevel = false;
    if (legacy.level < 99)
    {
        LegacyBossStats next;
        offset = stride * legacy.level;
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.maxHP, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.maxMP, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.agility, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.attack, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.defense, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.alternateVersion, 1);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.rewardExp, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.rewardGold, 4);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &next.dropListIndex, 1);
        
        if (legacy.stats.dropListIndex < next.dropListIndex)
            legacy.stats.newDropListAtNextLevel = true;
    }

    offset = stride * 99;
    BINARY_READ_AND_ADVANCE(innerFileData, offset, &legacy.stats.numLevelUpMoves, 1);
    for (int i = 0; i < legacy.stats.numLevelUpMoves; i++)
    {
        LegacyBossStats::LevelUpMove* dst = &legacy.stats.levelUpMoves[i];
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &dst->moveID, 2);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &dst->level, 1);
        BINARY_READ_AND_ADVANCE(innerFileData, offset, &dst->announceLearn, 1);
    }
}

void DetailedTreasureMapData::LoadTreasures()
{
    if (!GetTreasureMapLanguageData(GetBattleStruct()))
        return;

    TreasureMapLanguageDataOffsets* offsets = func_ov017_0218b5b0()->pTMapLanguageOffsets;
    unsigned int offset;
    bool foundBoss = false;

    if (mapType == TreasureMapType_Regular)
    {
        if (treasureItemIDs[0] != 0 && treasureItemIDs[1] != 0 && treasureItemIDs[2] != 0)
            return;
        offset = offsets->grottoBossDrops;
        do
        {
            unsigned char bossIndex;
            unsigned short regularID;
            unsigned int innerOffset = offset;
            TMAPLANGDATA_READ(innerOffset, &bossIndex, 1);
            TMAPLANGDATA_READ(innerOffset, &regularID, 2);
            if (regularID == regular.bossMonsterID)
            {
                rand();
                foundBoss = true;
                offset = innerOffset;
                break;
            }
            else
                offset += 12;
        } while (offset < offsets->legacyBossDrops);
    }
    else
    {
        offset = offsets->legacyBossDrops;
        do
        {
            unsigned short legacyID;
            TMAPLANGDATA_READ(offset, &legacyID, 2);
            if (legacyID == legacy.bossMonsterID)
            {
                foundBoss = true;
                offset += (legacy.stats.dropListIndex - 1) * 9;
                break;
            }
            else
                offset += 90;
        } while (offset < offsets->legacyBossData);
    }

    if (foundBoss)
    {
        for (int i = 0; i < 3; i++)
        {
            TMAPLANGDATA_READ(offset, &treasureItemIDs[i], 2);
            TMAPLANGDATA_READ(offset, &treasureDropRates[i], 1);
        }
    }
}