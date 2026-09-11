#pragma once

#include "../../World/Object3D.h"

struct PrimaryCombatStats {
    unsigned short currHP;
    unsigned short currMP;
    unsigned short maxHP;
    unsigned short maxMP;
    unsigned short attack;
    unsigned short defense;
    unsigned short agility;
    unsigned short unk;
    unsigned int charm : 10;
    unsigned int magicalMight : 10;
    unsigned int magicalMending : 10;
};

struct BaseCombatStats {
    char unk[0x2C];
    struct PrimaryCombatStats primaryStats;
};

struct ModifiableCombatStats {
    struct PrimaryCombatStats primaryStats; // 0xE
    char unk1[0x44];
    signed int attackBuff : 3;
    signed int defenseBuff : 3;
    signed int agilityBuff : 3;
    signed int charmBuff : 3;
    signed int magicalMightBuff : 3;
    signed int magicalMendingBuff : 3;
};

struct CombatantStruct {
    Object3D object_;
    char unk_ac[0x86];
    struct BaseCombatStats* baseStats; // TODO: holds more general info than just stats
    struct ModifiableCombatStats* currentStats; // includes things like buffs being applied
};

struct BattleStruct {
    int unk0;
    int unk4;
    struct CombatantStruct* combatantList[0xe9]; // TODO: validate this size as well as this struct as a whole
    int playerCombatantIndex;
};
struct BattleStruct* GetBattleStruct();
CombatantStruct* GetPlayerCombatant(BattleStruct* battle);
struct CombatantStruct* GetCombatantFromList(struct BattleStruct* battleStruct, int id);
