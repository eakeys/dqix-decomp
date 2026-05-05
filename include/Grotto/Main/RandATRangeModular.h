#pragma once

// Includes the endpoints.
// Gets a random number from AT via the usual rand() % range + min trick.
// Doesn't perform any range checks: AT always advances.
// Looks like a more generic function, but this version is only used
// by grotto functions so we put it here.
unsigned int RandATRangeModular(unsigned int minimum, unsigned int maximum);

#ifdef jpn
void RemoveFurigana(const char* src, char* dst);
#endif

bool IsMonsterIDLegacyBoss(unsigned short id);

// The output format here is pretty weird, it's a 4 byte struct of the form
// u8 mapType (1 = regular, 2 = legacy)
// u8 legacyBossID (0 if not a legacy map)
// u16 isLegacy (0 = regular, 1 = legacy but it's 16 bit...?)
bool GetTreasureMapTypeFromItemID(unsigned short itemID, unsigned char* out);