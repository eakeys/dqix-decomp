#pragma once

#include "Grotto/Main/ActiveGrottoClass.h"
#include "Memory/SafeAllocator.h"
#include "Graphics/Vector.h"

struct Zone3D_StructPtr_8
{
    unsigned short unknown_0_;
    unsigned short unknown_2_ : 15;
    char unk_4;
     // e.g. "F02" or "B01M13", corresponding to file data/map/%s.ambl and similar
    char mapShortName_[7];
    unsigned char unknown_c_low_ : 4;
    unsigned char unknown_c_high_ : 1;
};

// To be moved once we figure out what it is.
// sizeof == 0xb4
struct Zone3D_TextureStruct
{
    char unk_0[0xac];
    const char* filename_;
    Zone3D_TextureStruct* pNext_;
};

// sizeof == 0x58
struct Zone3D_BMDJStruct
{
    int unknown_0_;
    char unk_4[0x44];
    Vector3i vec_48_;
    Zone3D_BMDJStruct* pNext_;
};

// sizeof == 0x2824, as seen in the dynamic allocation of one
// of these in func_ov001_02163b14 (usa).
// In JPN version, sizeof == 0x2864.
// Represents a 3D zone such as a town, field or grotto floor,
// but also a battlefield.
class Zone3D
{
public:
    unsigned short currentZoneID_;
    unsigned short previousZoneID_;

    short unknown_4_;
    char unk_6[2];
    Zone3D_StructPtr_8* pUnknownStruct_8_;
    char unknown_c_[10]; // holds e.g. "Z02M0100" while in a grotto
    char unknown_16_[0x10];
    char unknown_26_;
    char unk_27[0x36 - 0x27];
    short unknown_36_;
    short unknown_38_;
    int unknown_3c_;
    short unknown_40_;
    char unk_42[2];
    int unknown_44_;
    int unknown_48_;
    SafeAllocator* pAllocator_4c_;
    void* unknown_ptr_50_; // referenced in the nsbtx processor, so something graphical
    SafeAllocator internalAllocator_;
    SafeAllocator* pAllocator_68_;

    struct BData
    {
        struct BStruct // might be the same as Zone3D_BMDJStruct but not sure
        {
            int unknown_0_;
            Vector3i vec_4_;
            char buffer_10_[0x48];
        };

        BStruct* entries_;
        int arraySize_;
        int arrayCapacity_;
        char unk_8[0x7c];
    } unknownBData_;
    char unknown_struct_f4_[0x18]; //
    char unknown_struct_10c_[0x30c]; // passed to func_0207b9cc
    Zone3D_TextureStruct* firstTextureStruct_418_;
    Zone3D_BMDJStruct* firstBMDJStruct_41c_;
    void* grottoTileMapData_420_; // pointer to array of stride 0x48
    int unknown_424_;
    char unk_428[4];
    unsigned char unknown_42c_;
    char unk_42d[3];
    int mapListLoadHandle_; // for loading data/map/maplist9.bin
    int unknown_434_;
    int mapAMBLLoadHandle_; // loads things like data/map/Z02M01.ambl
    int mapAMDJLoadHandle_;
    int unknown_440_;

    char unk_444[0x474 - 0x444];

    short unknown_474_;
    char unknown_476_;
    char unknown_477_;
    int unknown_478_;
    int unknown_47c_;

    char unk_480[0x82c - 0x480];

    int unknown_82c_;

    char unk_830[4];
    char unknown_834_;
    char unk_835[3];

    // these might be sth like number of vertices / indices for draw commands
    int unknown_838_;
    int unknown_83c_;

    char unk_840[0x23b8 - 0x840];


    // 0x20 extra bytes unaccounted for in JPN version
    bool isInMainGrottoFloor_23b8_;
    char unknown_23b9_;
    char currentGrottoFloor_23ba_;
    char copyOfCurrentGrottoFloor_23bb_;
    char unk_23bc[4];
    // when you change zones such that you are no longer in a main
    // grotto floor (e.g. you go to boss zone, leave at the top or cast evac)
    // this stores your character position right before you left
    Vector3i position_23c0_;
    unsigned short unknown_23cc_;
    char unk_23ce[0x23ec - 0x23ce];
    ActiveGrottoClass grotto_; // offset 23ec in USA. this is 0x20 bytes larger in JPN
    char unk_2664[0x2724 - 0x2664];
    char unknown_struct_2724_[0xc];
    char unk_2730[0x2754 - 0x2730];
    char unknown_struct_2754_[0x18];
    char unk_276c[0x2820 - 0x276c];
    char unknown_2820_;
    char unk_2821[3];
public:
    // usa: func_0201383c
    void SwitchZone(unsigned short newID);

    // usa: func_0201403c
    // The ambl is a NARC containing nsbtx, bmbl, dat and bpos files.
    void LoadMapAMBL();
    // usa: func_02014108
    bool UnpackMapAMBL();
    // usa: func_02014390
    bool ProcessBMBLFile(const void* filedata, unsigned int filesize);
    // usa: func_020143d8
    bool ProcessBPOSFile(const void* filedata, unsigned int filesize);
    
    // usa: func_0201445c
    bool ProcessNSBTXFile(const void* filedata, unsigned int filesize, const char* filename);

    // usa: func_020145a8
    // The amdj is a narc containing nsbmd, nsbma (?), col2 and bmdj files.
    void LoadMapAMDJ();
    // usa: func_020146fc
    bool UnpackMapAMDJ();
    // usa: func_02014900
    bool ProcessBMDJFile(const void* filedata, unsigned int filesize, BData::BStruct* misc);
};