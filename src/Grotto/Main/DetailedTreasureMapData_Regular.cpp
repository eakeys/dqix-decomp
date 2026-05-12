#include "Grotto/Main/TreasureMapDataStructs.h"
#include "Combat/Main/BattleList.h"
#include "Grotto/Main/GrottoStruct.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "System/Memory.h"
#include "std_library_functions.h"

// also counts for unsigned char arrays, e.g. data_020e9074 & data_020e9077
#pragma pool_strings on
// need this so the blank string in data_020f1ac3 (which is effectively a 
// single char equal to 0) doesn't go into .bss
#pragma explicit_zero_data on
// We *do* reuse strings within a translation unit
#pragma dont_reuse_strings off

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

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GetTreasureMapLanguageData(GetBattleStruct()), offset, into, len)

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

// USA: func_020a4e08data start:0x020f1ac0 end:0x020f1ad0
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

    static const unsigned char offsetIndexLookup[] = { 4, 13, 11 };
    
    for (int i = 0; i < 3; i++)
    {
        int currentNamePart = partOrder[i];
        int ptrListIndex = offsetIndexLookup[currentNamePart];
        
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

    sprintf(levelString, "%s%d", func_020e51cc(1011), level);
    sprintf(topScreenName, "%s %s", nameNoLevel, levelString);
}

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

    static const unsigned char offsetIndexLookup[] = { 4, 13, 11 };
    
    for (int i = 0; i < 3; i++)
    {
        int currentNamePart = partOrder[i];
        int ptrListIndex = offsetIndexLookup[currentNamePart];
        
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

    sprintf(tempBuffer, " %s%d", func_020e51cc(1011), level);
    strcat(popupName, tempBuffer);
}

#elif defined(jpn)

#define JPChar_Lv "\xEA\x40"

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
    strcat(nameUndecorated, "\x92\x6E" "\x90\x7D"); // "地図"
    strcpy(nameNoLevel, nameUndecorated);
    sprintf(levelString, JPChar_Lv "%d", level);
    sprintf(topScreenName, "%s<W=3>%s", nameNoLevel, levelString);
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

    sprintf(tempBuffer, JPChar_Lv "%d", level);
    strcat(popupName, tempBuffer);
}

#endif