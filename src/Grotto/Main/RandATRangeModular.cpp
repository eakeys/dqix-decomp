#include <globaldefs.h>
#include "std_library_functions.h"
#include "System/Memory.h"
#include "Grotto/Main/RandATRangeModular.h"
#include "Grotto/Main/TreasureMapMetadata.h"

// USA: func_020a3ec0
// JPN: func_020a5bfc
unsigned int RandATRangeModular(unsigned int minimum, unsigned int maximum)
{
    return minimum + ((unsigned int)rand() % (maximum - minimum + 1));
}

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