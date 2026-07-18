#include "World/Zone3D.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/BackgroundLoader.h"
#include "Grotto/Overlay_17/Struct44C8.h"

extern "C"
{
    void* func_02011584(BattleStruct*);
    void* func_0200fdcc(BattleStruct*);
    void* func_0200fddc(BattleStruct*);

    void* func_02053c6c(void*);

    void func_0207a5b8(void*);
    void func_0207b9cc(void*);
    void* func_0207df50(void*);
    void* func_0208a9b4();
    void func_02094d00(void*);
    Zone3D_StructPtr_8* func_02099950(void*, unsigned short id);
    void func_020de848(void*);

    void func_02013490(void*);
    void func_02013750(Zone3D*, bool);
    void func_0201e248(void*);

    // checks if zone id corresponds to a main floor of a grotto
    bool func_0201b5b0(int id);
}

extern char data_020ef0f0[];

void Zone3D::SwitchZone(unsigned short newID)
{
    BattleStruct* battle = GetBattleStruct();
    BackgroundLoader* loader = BackgroundLoader::GetInstance();

    void* uVar3 = func_02011584(battle);
    (void)func_ov017_0218b5b0();
    void* iVar4 = func_0200fddc(battle);

    pAllocator_68_ = pAllocator_4c_;
    pAllocator_68_->Reset();

    func_0207df50(unknown_ptr_50_);
    func_02013750(this, true);

    previousZoneID_ = currentZoneID_;
    currentZoneID_ = newID;

    unknown_838_ = 0;
    unknown_83c_ = 0;
    unknown_424_ = 1;
    unknown_41c_ = 0;
    unknown_418_ = 0;
    unknown_476_ = 0;
    unknown_477_ = 0;
    unknown_82c_ = 0;
    unknown_474_ = 0;
    unknown_82c_ = 0; // why zero it twice?
    unknown_42c_ = 0;

    mapListLoadHandle_ = -1;
    unknown_434_ = -1;
    unknown_438_ = -1;
    unknown_43c_ = -1;
    unknown_440_ = -1;

    unknown_478_ = 0;
    unknown_47c_ = 0;
    unknown_834_ = 0;
    unknown_2820_ = 0;

    func_0201e248(&unknown_struct_6c_[0]);

    unknown_c_ = 0;
    unknown_16_ = 0;
    unknown_26_ = 0;
    unknown_36_ = 0x7fff;
    unknown_38_ = 0;
    unknown_3c_ = 10;
    unknown_40_ = 0;
    unknown_44_ = 0;
    unknown_48_ = 0;

    func_0207a5b8(&unknown_struct_f4_[0]);
    func_0207b9cc(&unknown_struct_10c_[0]);
    func_020de848(&unknown_struct_2754_[0]);

    pUnknownStruct_8_ = func_02099950(uVar3, newID);
    unknown_4_ = pUnknownStruct_8_->unknown_2_;
    if (pUnknownStruct_8_->unknown_c_low_ == 0)
    {
        void* iVar5 = func_0200fdcc(battle);
        if (iVar5 != NULL)
        {
            void* iVar6 = func_02053c6c(iVar5);
            if (iVar6 != NULL)
                *(unsigned short*)((int)iVar6 + 0x566) = pUnknownStruct_8_->unknown_0_;
        }
    }

    *(bool*)((int)func_0208a9b4() + 0x9c) = pUnknownStruct_8_->unknown_c_high_ != 0;
    func_02094d00(&unknown_struct_2724_[0]);

    grottoTileMapData_420_ = NULL;

    if (func_0201b5b0(previousZoneID_))
    {
        grotto_.floorMap.Clear();
    }

    if (func_0201b5b0(newID))
    {
        isInMainGrottoFloor_23b8_ = true;
        currentGrottoFloor_23ba_ = newID % 20;
        copyOfCurrentGrottoFloor_23bb_ = currentGrottoFloor_23ba_;
        int width = grotto_.CalculateAndStoreFloorWidth(currentGrottoFloor_23ba_);
        int height = grotto_.CalculateAndStoreFloorHeight(currentGrottoFloor_23ba_);

        grottoTileMapData_420_ = pAllocator_68_->Allocate(0x4800);
        for (int i = 0; i < 0x100; i++)
        {
            func_02013490((void*)((int)grottoTileMapData_420_ + i * 0x48));
        }
        grotto_.ClearGenerator(false);
        grotto_.AllocateGenerator(pAllocator_68_, false);
        grotto_.CalculateFloorMap(currentGrottoFloor_23ba_, width, height, NULL);
    }
    else
    {
        if (currentGrottoFloor_23ba_ != -1)
        {
            copyOfCurrentGrottoFloor_23bb_ = currentGrottoFloor_23ba_;
            position_23c0_ = *(Vector3i*)((int)iVar4 + 0x44);
            unknown_23cc_ = *(short*)((int)iVar4 + 0xae);
        }
        isInMainGrottoFloor_23b8_ = false;
        currentGrottoFloor_23ba_ = -1;
    }

    mapListLoadHandle_ = loader->QueueLoadFile(data_020ef0f0, NULL);
}