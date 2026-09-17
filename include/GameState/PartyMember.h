#pragma once

#include "Combatant.h"

struct PartyMemberStats
{
    char unk_0[0x16c];
    unsigned short vocationLevels[13];
    unsigned char revocationCounts[13];
    char unk_193[0x49c - 0x193];
    unsigned char gender_ : 1; // 0 = m, 1 = f (matches nouns)

};

class PartyMember : public Combatant
{
public:
    // i don't know where the difference in bytes comes from. might be in combatant
#if defined(usa)
    char unk_140[0x10];
#elif defined(jpn)
    char unk_140[4];
#endif
    // offset 0x150 from the start
    PartyMemberStats* partyMemberStats_;
};