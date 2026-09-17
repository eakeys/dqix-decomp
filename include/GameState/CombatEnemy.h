#pragma once

#include "Combatant.h"

struct EnemyNamingData
{
    char* speciesName; // "slime"
    char* modelName; // "z000a"
    char unk_8[12];
    char* pluralName; // "slimes"
    unsigned int indefiniteSingularOffset_ : 6;
    unsigned int indefinitePluralOffset_ : 6;
    unsigned int definiteSingularOffset_ : 6;
    unsigned int definitePluralOffset_ : 6;
    unsigned int gender_ : 2; // 0 = m, 1 = f, 2 = n
    unsigned int bit_26 : 1;
    unsigned int bit_27 : 1;
    unsigned int bit_28 : 1;
};

class CombatEnemy : public Combatant
{
public:
    char unk_140[4];
    EnemyNamingData* namingData_;
    char unk_148[4];
    // offset 0x14c. Not sure about length, but at least 21 to fit
    // Tyrannosaurus Wrecks, and at most 57 by viewing memory
    char individualName_[24]; 

};