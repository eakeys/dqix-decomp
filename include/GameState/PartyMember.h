#pragma once

#include "Combatant.h"

struct PartyMemberStats
{
    char unk_0[0x49c];
    unsigned char gender_ : 1; // 0 = m, 1 = f (matches nouns)

};

class PartyMember : public Combatant
{
public:
    char unk_140[0x10];
    // offset 0x150 from the start
    PartyMemberStats* partyMemberStats_;
};