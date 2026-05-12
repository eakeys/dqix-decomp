#include "Grotto/Main/TreasureMapDataStructs.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "System/Memory.h"
#include "std_library_functions.h"

// also counts for unsigned char arrays
#pragma pool_strings on
// need this so blank strings (which are effectively a single char equal to 0)
// don't go into .bss
#pragma explicit_zero_data on
// We *do* reuse strings within a translation unit
#pragma dont_reuse_strings off

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GetTreasureMapLanguageData(GetBattleStruct()), offset, into, len)

#if defined(jpn)
#define JPChar_Lv "\xEA\x40"
#endif

extern "C"
{
    // Based on where it's called, this is probably returning a language-
    // dependent string for "Lv. " (at least, if called with 1011 as arg).
    // Not used in jpn version
    const char* func_020e51cc(int);

    // seems to get the game language. In the USA version, if it would
    // return a value other than 2 or 5, it returns 1, which seems to reflect
    // lack of support for German & Italian.
    // not used in jpn version
    int func_0200fb08(BattleStruct*);
}

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
    sprintf(mapNameNoLevel_v2, "%s", mapNameNoLevel);
    strcpy(seeminglyEmptyBuffer, "");
    sprintf(mapLevelString, "%s%d", func_020e51cc(1011), level);
    sprintf(topScreenName, "%s %s", mapNameNoLevel, mapLevelString);
    sprintf(popupName, "%s %s", bossName, mapLevelString);
#elif defined(jpn)
    char bossNameUndecorated[256];

    RemoveFurigana(bossName, bossNameUndecorated);
    // format = "%sの地図"
    sprintf(mapNameNoLevel, "%s" "\x82\xCC" "\x92\x6E" "\x90\x7D", bossNameUndecorated);
    // format = "%sの"
    sprintf(bossNameGenitive, "%s" "\x82\xCC", bossName);
    strcpy(fixedStringChizu, "[" "\x92\x6E" "\x90\x7D" "/" "\x82\xBF" "\x82\xB8" "]"); // "[地図/ちず]"
    strcpy(mapLevelString, "");
    sprintf(mapLevelString, JPChar_Lv "%d", level);
    sprintf(topScreenName, "%s<W=3>%s", mapNameNoLevel, mapLevelString);
    sprintf(popupName, "%s" JPChar_Lv "%d" "\x82\xCC" "[" "\x8A\xD4" "/" "\x82\xDC" "]\0\0", bossName, level); // "%sLv%dの[間/ま]"
#endif
}

// USA: func_020a451c
// JPN: func_020a62c8
void DetailedTreasureMapData::LegacyBossMapData::WriteMapLevelString()
{
#if defined(usa)
    const char* lvlPrefix = func_020e51cc(1011);
    sprintf(mapLevelString, "%s%d", lvlPrefix, level);
#elif defined(jpn)
    sprintf(mapLevelString, JPChar_Lv "%d", level);
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

/* End of LegacyBossDetailedData, start of RegularMapDetailedData */
