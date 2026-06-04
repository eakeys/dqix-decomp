#pragma once

#include "FSStructs.h"

// sizeof == 0x2C == 44.
// Held as an array in 0x01FFD394
struct OverlayMetadata
{
    typedef void (*PFNStaticInitializer)();

    unsigned int overlayIndex;
    unsigned int loadAddress;
    unsigned int uncompressedSize;
    unsigned int bssSectionSize;
    // From 020cd3a0 this seems to be start and end pointers
    // for an array of constructors (i.e. functions of signature void())
    PFNStaticInitializer* staticInitStart;
    PFNStaticInitializer* staticInitEnd;
    unsigned int nitroFileID; // seems to match the overlay index, but is used with file loading stuff

    // GBATEK says this is reserved and should be zero, but in our case
    // it definitely isn't!
    unsigned int compressedSize : 24;
    // bit 0: is compressed
    // bit 1: ???, something with download play
    unsigned int overlayFlags : 8;

    CBool isArm7;
    unsigned int unknown_24;
    unsigned int unknown_28;
};

unsigned int GetOverlaySizeOnCartridge(const OverlayMetadata& overlay);
void InvalidateCacheAndZeroOverlay(const OverlayMetadata& overlay);
void CreateFileAccessorForOverlay(NitroFileAccessor* outAccessor, const OverlayMetadata& overlay);

bool LoadOverlayMetadataFromNitro(OverlayMetadata* into, bool isArm7,
    unsigned int overlayIdx, NitroHandle* romHandle,
    unsigned int arm9OverlayTableStart, unsigned int arm9OverlayTableSize,
    unsigned int arm7OverlayTableStart, unsigned int arm7OverlayTableSize);

bool LoadOverlayMetadata(OverlayMetadata* into, bool isArm7, unsigned int idx);

bool LoadCompressedOverlay(const OverlayMetadata& overlay, NitroVM* machine);