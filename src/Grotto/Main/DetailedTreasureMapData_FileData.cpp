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

#ifdef jpn
#define func_0202f7c8 func_0202f338
#define func_0202f7e8 func_0202f358

#define func_02075098 func_02076224
#define func_02075248 func_02076378

#define data_0211e33c data_0211fb64
#endif

extern "C"
{
    void func_0202f7c8();
    void func_0202f7e8();

    // Seems to load an arbitrary file into a buffer, then return that buffer
    unsigned char* func_02075098(const char* file, const void* buffer, unsigned int* outLength);

    // Seems to extract a file from a NARC buffer
    bool func_02075248(const unsigned char* narcBuffer, const char* filename,
        const unsigned char** ppFileData, unsigned int* pFileSize, unsigned int startFileIndex);
}

#define BINARY_READ_AND_ADVANCE(buffer, offset, dst, len) \
    (VectorizedInvertedMemcpy((buffer) + (offset), (dst), (len)), offset += (len))

#define TMAPLANGDATA_READ(offset, into, len) \
    BINARY_READ_AND_ADVANCE(GetTreasureMapLanguageData(GetBattleStruct()), offset, into, len)


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
        if (!func_02075098("data/tmap/param.pac", usedArchive, &archiveSize))
        {
            func_0202f7e8();
            return;
        }
    }

    char innerFileName[256];
    sprintf(innerFileName, "p_%d.dat", legacy.bossMonsterID);
    const unsigned char* innerFileData;
    unsigned int innerFileSize;
    if (!func_02075248(usedArchive, innerFileName, &innerFileData, &innerFileSize, 0))
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
    // This gets optimized out, but not immediately. Without it, the 4s, 2s and 1s
    // in subsequent reads get loaded from the literal pool (e.g ldr r2, [pc, blah]
    // instead of mov r2, #4). A branch/if/goto seems to restore normal behaviour
    // but trivial if/goto statements get optimized out too early in the process
    // to be viable as a fix.
    __asm("b right_here\nright_here:");
    
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