#pragma once

#include "../NitroSDK/Graphics2D/Graphics2D.h"
#include <globaldefs.h>

// sizeof == 0x60, instance at 02105254 (usa)
// responsible for the graphics on the top screen map, possibly other things
// but I can't seem to find any
class MapGraphics
{
public:
    char unknown_0_[0x20];
    char unknown_20_[0x20];
    int status_40_[8];

public:
    static MapGraphics* GetInstance();

    void Reset();
    void Reset2(); // should be Destroy() or similar but looks used in initialization?

    void Present();
};

class OBJManager
{
public:
    int unknown_0_;
    int unknown_4_;
    int unknown_8_;
    bool oamEnabled_c_[2]; // index by engine
    char unk_e[2];

    struct TextEntry
    {
        char unknown_0_[0x20];
        unsigned char unknown_20_;
        unsigned char unknown_21_;
        short oamTileNumber_22_;
        unsigned char unknown_24_;
        char pad_25[3];

        void Reset();

        void WriteToOAM(OAMEntryEXT* oam, int priority, int x, int y, int palette);
    };

    struct Substruct_10
    {
        OAMEntryEXT* oamBuffer_0_;
        TextEntry textEntries_4_[32];
        int unknown_504_;
        int unknown_508_;
        unsigned int unknown_50c_;
        unsigned short oamWriteIndex_510_;
        unsigned short unknown_512_;

        void Reset();
        void LoadPalettes();
        void Destroy();
        void DrawTextEntry(short idx, int priority, int x, int y, int basePalette);

    } substruct_10_;

public:

    static OBJManager* GetInstance();

    void Reset();

    void FlushCache();
    void Present();

    OAMEntryEXT* GetMainOAMEntries() const;
    OAMEntryEXT* GetSubOAMEntries() const;
    void SetOAMEngineEnabled(int engine, bool to);
};