#pragma once

#include "Grotto/Main/ActiveGrottoClass.h"
#include "Memory/SafeAllocator.h"

struct Zone3D_StructPtr_8
{
    unsigned short unknown_0_;
    unsigned short unknown_2_ : 15;
    int unk_4;
    int unk_8;
    unsigned char unknown_c_low_ : 4;
    unsigned char unknown_c_high_ : 1;
};

struct Vector3i
{
    int x;
    int y;
    int z;
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
    char unknown_c_;
    char unk_d[0x16 - 0xd];
    char unknown_16_;
    char unk_17[0x26 - 0x17];
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
    void* unknown_ptr_50_;
    char unk_54[0x14];
    SafeAllocator* pAllocator_68_;
    char unknown_struct_6c_[0x88];
    char unknown_struct_f4_[0x18]; //
    char unknown_struct_10c_[0x30c]; // passed to func_0207b9cc
    int unknown_418_;
    int unknown_41c_;
    void* grottoTileMapData_420_; // pointer to array of stride 0x48
    int unknown_424_;
    char unk_428[4];
    char unknown_42c_;
    char unk_42d[3];
    int mapListLoadHandle_; // for loading data/map/maplist9.bin
    int unknown_434_;
    int unknown_438_;
    int unknown_43c_;
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
    void SwitchZone(unsigned short newID);
};