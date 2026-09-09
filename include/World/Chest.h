#pragma once

#include "../Graphics/Vector.h"

// sizeof == 0x24. Represents an instance of a chest in the loaded zone.
// Note that the chest models (body and lid) are stored within the zone itself.
class Chest
{
public:
    Vector3fix position_;
    fix16_t rotation_;
    fix16_t lidRotation_; // 0 or 2pi when closed, 4 when open (counts down from 2pi)
    short unk_10;
    short unk_12;
    char unk_14;
    char unk_15;
    unsigned char maybeFlags_;
    bool isBlue_;
    char unk_18;
    char unk_19;
    short unk_1a;
    short uniqueID_; // populated from LootableContainerManager::Container
    char pad_1e[2];
    int unk_20;
};