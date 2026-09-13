#include <globaldefs.h>
#include "GameState/GameState.h"

GameObject* GameState::GetGameObjectByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetMaybeWanderingMonsterByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 2))
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetCartridgeProtagonist()
{
    return GetGameObjectByIndex(cartridgeProtagonistObjectIndex_);
}

GameObject* GameState::GetPartyLeader()
{
    return GetGameObjectByIndex(partyLeaderObjectIndex_);
}

GameObject* GameState::GetPartyMemberByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 0x800))
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetMaybeFieldMonsterByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 0x20))
        return NULL;
    return objects_[idx];
}

GameObject* GameState::GetCombatantByIndex(int idx)
{
    if (idx < 0)
        return NULL;
    if (idx >= 0xe9)
        return NULL;
    if (objects_[idx] == NULL)
        return NULL;
    if (!(objects_[idx]->obj3D_.unknown_0_ & 0x80))
        return NULL;
    return objects_[idx];
}