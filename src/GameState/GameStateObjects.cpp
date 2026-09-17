#include <globaldefs.h>
#include "GameState/GameState.h"

Object3D* GameState::GetGameObjectByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    return objects_[idx];
}

Object3D* GameState::GetMaybeWanderingMonsterByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->unknown_0_ & 2))
        return NULL;
    return objects_[idx];
}

PartyMember* GameState::GetCartridgeProtagonist()
{
    return (PartyMember*)GetGameObjectByIndex(cartridgeProtagonistObjectIndex_);
}

PartyMember* GameState::GetPartyLeader()
{
    return (PartyMember*)GetGameObjectByIndex(partyLeaderObjectIndex_);
}

PartyMember* GameState::GetPartyMemberByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->unknown_0_ & 0x800))
        return NULL;
    return (PartyMember*)objects_[idx];
}

Object3D* GameState::GetMaybeFieldMonsterByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->unknown_0_ & 0x20))
        return NULL;
    return objects_[idx];
}

Combatant* GameState::GetCombatantByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->unknown_0_ & 0x80))
        return NULL;
    return (Combatant*)objects_[idx];
}

CombatEnemy* GameState::GetEnemyByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->unknown_0_ & 0x400))
        return NULL;
    return (CombatEnemy*)objects_[idx];
}