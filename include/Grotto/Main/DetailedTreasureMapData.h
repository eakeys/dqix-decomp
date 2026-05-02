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
        // to be 1-12 based on the seed (but in practice the chance is 0% for each).
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

#if defined(usa)
        char nameNoLevel[64];
        char levelString[8];
        char topScreenName[64];
        // Does not include the <PAD_WAIT> command (that's added externally)
        char popupName[64];
#elif defined(jpn)
        char nameNoLevel[32];
        char prefixString[32];
        char suffixString[32];
        char localeString[32];
        char levelString[8];
        char topScreenName[64];
        char popupName[128];
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
        // could be e.g. 24 bytes with padding
        // in JPN version, this is stored *with* furigana decorations
        char bossName[26];
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

#if defined(usa)
        // All stored in the 'markup' encoding, e.g. using <1> for apostrophe
        char mapNameNoLevel[64]; // e.g. "Baramos<1>s Map"
        char mapNameNoLevel_v2[32]; // same as above, not sure what the difference is
        char seeminglyEmptyBuffer[32];
        char mapLevelString[8]; // e.g. "Lv. 99"
        char topScreenName[64]; // "Baramos<1>s Map Lv. 99"
        char popupName[64]; // e.g. "Baramos Lv. 99" (what pops up on entering the grotto)
#elif defined(jpn)
        // The sizes are correct, but the interpretation is potentially dodgy -
        // someone who actually knows Japanese should probably take a look (I
        // deduced the linguistic purpose of these from Google Translate/Wikipedia)
        char mapNameNoLevel[32]; // e.g. Baramos no chizu
        char bossNameGenitive[32]; // e.g. Baramos no
        char fixedStringChizu[32]; // always holds "chizu", which I gather means map
        char mapLevelString[8]; // e.g. "Lv 99"
        char topScreenName[64]; // e.g. Baramos no chizu Lv 99
        char popupName[128]; // e.g. Baramos Lv 99 no chizu (pops up on entering the grotto)
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