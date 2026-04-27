#pragma once

#include <globaldefs.h>
#include "TreasureMapMetadata.h"

class DetailedTreasureMapData
{
public:
    class RegularMapData
    {
    public:
        unsigned short seed;
        unsigned char quality;
        // Related to unknown_66. It's always 0 but the code allows a random chance
        // to be 1-12 based on the seed (but in practice the chance is 0%).
        // I don't know what it does if it's nonzero.
        char unknown_4F;
        unsigned char environ; // caves, ruins, ice, water, fire as 1,2,3,4,5 resp.
        unsigned char floorCount;
        unsigned char startingMonsterRank;
        unsigned char bossID; // 1 to 12
        unsigned short bossEnemyId; // the number in square brackets in yabd's bestiary
        // looks like it gets populated with a random valid chest rank
        // for each monster rank, but never gets used
        char maybeUnusedChestRanks[12];
        unsigned char prefix;
        unsigned char suffix;
        unsigned char localeRank;
        unsigned char level;
        char unknown_66; // was always 1 in the grottos I checked

#ifndef jpn
        // This is still pretty speculative, but from a quick glance at
        // func_020a51b0 and looking at memory while in a grotto, I think
        // it's something like:
        char nameNoLevel[64];
        char levelString[8];
        char fullName[64];
        // This is identical to fullName, but is used in a call along lines of
        //     sprintf(output, "%s<PAD_WAIT>", popupName)
        // (see func_ov017_021bec00)
        char popupName[64];
#else
        // this is probably not what's actually going on, but it makes
        // this object the right size for memset calls. Probably either the
        // buffers are resized or there are some extra ones.
        char unknownJpnBuffers[0xC8];
        char popupName[64];
        char extraUnknownJpnBuffer[64];
#endif

    public:
        void Populate(unsigned short seed, unsigned char quality);

    private:
        void GenerateUnknownData();
        void GenerateEnviron();
        void GenerateFloorCount();
        void GenerateMonsterRank();
        void GenerateBoss();
        void GenerateUnusedChestRanks();
        void GeneratePrefix();
        void GenerateSuffix();
        void GenerateLocaleRank();

        void GenerateNameBuffers();
        void GeneratePopupName();
    };

    class LegacyBossMapData
    {
    public:
        unsigned char bossID;
        unsigned short bossEnemyID; 
        // holds the ids of the 1A, 2A, 3A versions in yabd bestiary
        unsigned short alternateVersionIDs[3];
        unsigned char level;
        unsigned short minTurns;
        char bossName[26]; // unsure of length, could be zeros at end for another reason
        // amazingly these are actually used quantities, if you modify them
        // on entering the grotto with a cheat, it changes their stats
        // when you fight them 
        int bossHP;
        int bossMP;
        unsigned short bossAgility;
        unsigned short bossAttack;
        unsigned short bossDefense;
        char padding_82[2];
        int whichAlternateVersion; // 1, 2 or 3
        int rewardExp;
        // might be an int, but only the last 16 bytes are used
        unsigned short rewardGold;
        unsigned char unknown_8E[2];
        unsigned char dropListIndex;
        bool newDropListAtNextLevel;
        unsigned char numLevelUpMoves;
        struct LevelUpMove
        {
            unsigned short moveID;
            unsigned char level;
            bool announceLearn;
        } levelUpMoves[10];

#ifndef jpn
        // All stored in the 'markup' encoding, e.g. using <1> for apostrophe.
        // Speculative based on looking at memory in a legacy grotto and
        // a glance at func_020a425c
        char mapNameNoLevel[64]; // e.g. "Estark<1>s Map"
        char mapNameNoLevel_v2[32]; // same as above, not sure what the difference is
        char seeminglyEmptyBuffer[32];
        char mapLevelString[8]; // e.g. "Lv. 99"
        char mapName[64]; // "Estark<1>s Map Lv. 99"
        char popupName[64]; // e.g. "Estark Lv. 99" (what pops up on entering the grotto)
#else
        // really no idea, I just looked at func_020a6050 (JPN) to guess
        // the sizes and positions
        char jpnBuffer1[32];
        char jpnBuffer2[32];
        char jpnBuffer3[32];
        char jpnBuffer4[8];
        char jpnBuffer5[64];
        char popupName[64];
        char jpnBuffer6[64];
#endif

    public:
        unsigned short MaybeGetCurrentAlternateID() const;
        void Populate(unsigned char bossID, unsigned char level, unsigned short minTurns);

        void WriteMapLevelString();
        bool CanUseLevelUpMove(unsigned short moveID);
        // if filter is 0 or 1, filters based on announceLearn
        unsigned short GetLearnedMove(unsigned char atLevel, int filter);
    };

    unsigned char discoveryState;
    unsigned char mapType;
    // Stored in the DQ9 string encoding. 12 bytes get zeroed out but
    // only 10 bytes get copied in
    char discoveredBy[12];
    char clearedBy[12];
    char probablyPadding_1A[2];
    // coordinates of the model (you spawn at offset (0, 0, +8192) on exiting)
    int entranceZoneID;
    int entranceX;
    int entranceY;
    int entranceZ;
    unsigned char mapLocation;
    char mapImageName[16]; // e.g. "mapt_005"
    bool discoveredTreasures[3];
    short treasureItemIDs[3]; // might be unsigned?
    unsigned char treasureDropRates[3];
    char unknown_49[3]; // might just be padding

    union
    {
        RegularMapData regular;
        LegacyBossMapData legacy;
    };

public:
    void Clear();
    void BlankFunction() const; // possibly returns this

    // Called on the overall struct, but only does anything for a legacy
    // boss map (and maybe only gets called then?)
    bool UpdateFollowingCompletion(bool levelledUp, unsigned short numTurns);

    unsigned int GetLevel() const;
};