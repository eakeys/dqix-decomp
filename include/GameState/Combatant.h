#pragma once

#include "../World/Object3D.h"
#include "../Combat/Main/BattleList.h"

class Combatant : public Object3D
{
public: 
    char unk_ac[0x130 - 0xac];
    void* unknown_ptr_130;
    BaseCombatStats* baseStats_;
    ModifiableCombatStats* currentStats_;
    void* unknown_ptr_13c;
    // After this, player and enemy seem to diverge
};
