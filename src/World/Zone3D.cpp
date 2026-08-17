#include "World/Zone3D.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/NarcHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileIO.h"
#include "Grotto/Overlay_17/Struct44C8.h"
#include "Graphics/NSBXX/NSBXX.h"

extern "C"
{
    void* func_02011584(BattleStruct*);
    void func_02013454(void*);
    void* func_0200fdcc(BattleStruct*);
    void* func_0200fddc(BattleStruct*);

    void* func_02053c6c(void*);
    void func_0205e104(const char*, SafeAllocator*, const void*, unsigned int);
    void func_0207a5b8(void*);
    void func_0207a614(void*, const char*);
    void func_0207b9cc(void*);

    // Texture functions
    void* func_0207df50(void*);
    void func_0207df90(void*);
    void func_0207dfac(void*);

    void* func_0208a9b4();
    void func_02094d00(void*);
    Zone3D_StructPtr_8* func_02099950(void*, unsigned short id);

    void func_020c9be0(); // abort() or similar
    void func_020de848(void*);

    void func_02013490(void*);
    void func_02013750(Zone3D*, bool);
    // handle bmbl file
    void func_02014390(Zone3D*, const void*, unsigned int);
    // handle nsbtx file
    void func_0201445c(Zone3D*, const void*, unsigned int, const char*);
    // handle bpos file
    void func_020143d8(Zone3D*, const void*, unsigned int);
    void func_02014a24(Zone3D*, void*);
    void func_02014b04(Zone3D*);
    void func_0201e248(void*);
    void func_0201e2b4(void*, SafeAllocator*, const void*, unsigned int);
    Zone3D::BData::BStruct* func_0201e2d8(Zone3D::BData*, int);
    void func_0201f040(void*, SafeAllocator*, const void*, unsigned int);

    // checks if zone id corresponds to a main floor of a grotto
    bool func_0201b5b0(int id);
    // checks if zone id corresponds to boss floor of a grotto
    bool func_0201b5d8(int id);
}

extern char data_020ef0f0[]; // "data/map/maplist9.bin"
extern char data_020ef106[]; // "%s/Z0%dM01.ambl"
extern char data_020ef116[]; // "data/map"
extern char data_020ef11f[]; // "%s/Z0%dM99.ambl"
extern char data_020ef12f[]; // "%s/%s.ambl"
extern char data_020ef13a[]; // "ARC"
extern char data_020ef13e[]; // ".nsbtx"
extern char data_020ef145[]; // ".bmbl"
extern char data_020ef14b[]; // ".dat"
extern char data_020ef150[]; // ".bpos"
extern char data_020ef156[]; // "%s/Z0%dM01.amdj"
extern char data_020ef166[]; // "%s/Z0%dM99.amdj"
extern char data_020ef176[]; // "%s/%sb.amdj"
extern char data_020ef182[]; // "%s/%sa.amdj"
extern char data_020ef18e[]; // "%s/%s.amdj"
extern char data_020ef199[]; // ".bmdj"
extern char data_020ef19f[]; // "Z0%dM0100"
extern char data_020ef1a9[]; // "Z0%dM0101"
extern char data_020ef1b3[]; // "Z0%dM0102"
extern char data_020ef1bd[]; // "Z0%dM0103"

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

    textureImageMemory_ = 0;
    texturePaletteMemory_ = 0;
    unknown_424_ = 1;
    firstBMDJStruct_41c_ = 0;
    firstModel_418_ = NULL;
    unknown_476_ = 0;
    unknown_477_ = 0;
    unknown_82c_ = 0;
    unknown_474_ = 0;
    unknown_82c_ = 0; // why zero it twice?
    unknown_42c_ = 0;

    mapListLoadHandle_ = -1;
    unknown_434_ = -1;
    mapAMBLLoadHandle_ = -1;
    mapAMDJLoadHandle_ = -1;
    unknown_440_ = -1;

    unknown_478_ = 0;
    unknown_47c_ = 0;
    unknown_834_ = 0;
    unknown_2820_ = 0;

    func_0201e248(&unknownBData_);

    unknown_c_[0] = 0;
    unknown_16_[0] = 0;
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

// implicitly defined Vector3i::operator=(const Vector3i&)

#if false

void Zone3D::LoadMapAMBL()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();

    char filenameBuffer[20];

    if (func_0201b5b0(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        if (environ == 0)
            environ = 1;
        if (environ > 5)
            environ = 5;
        sprintf(filenameBuffer, data_020ef106, data_020ef116, environ);
    }
    else if (func_0201b5d8(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        sprintf(filenameBuffer, data_020ef11f, data_020ef116, environ);
    }
    else
    {
        sprintf(filenameBuffer, data_020ef12f, data_020ef116, pUnknownStruct_8_->mapShortName_);
    }
    mapAMBLLoadHandle_ = loader->QueueLoadFile(filenameBuffer, NULL);
}

bool Zone3D::UnpackMapAMBL()
{
    if (mapAMBLLoadHandle_ < 0)
        return true;

    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(mapAMBLLoadHandle_) == 0)
        return false;

    // If we get here, the loading finished but was not successful
    if (loader->GetDetailedTaskStatus(mapAMBLLoadHandle_) != BackgroundLoader::TaskStatus_Complete)
    {
        loader->RemoveTask(mapAMBLLoadHandle_);
        mapAMBLLoadHandle_ = -1;
        return true;
    }
    
    void* amblData;
    unsigned int amblFilesize;
    
    loader->GetLoadedFileByID(mapAMBLLoadHandle_, &amblData, &amblFilesize);

    for (int pass = 0; pass < 2; pass++)
    {
        NarcHandle narc;
        if (narc.Initialize(data_020ef13a, (const unsigned char*)amblData))
        {
            NitroVM vm;
            unsigned int fileID = 0;
            NitroVM_Initialize(&vm);
            while (PrepareReadFileInNARCByID(&vm, &narc, fileID))
            {
                char innerFilePath[80];
                NitroVM_WriteOutFilePath(&vm, innerFilePath, 80);
                
                const char* extension = strrchr(innerFilePath, '.');
                if (extension == NULL)
                {
                    NitroVM_FinishRead(&vm);
                    fileID++;
                    continue;
                }
                unsigned int innerFilesize = vm.regbase_abc.c.u32 - vm.regbase_abc.b.u32;
                NitroVM_FinishRead(&vm);
                const void* innerFilePtr = narc.GetFileByIndex(fileID);
                if (pass == 0)
                {
                    // nsbtx file (we can have multiple of these)
                    if (strcmp(data_020ef13e, extension) == 0)
                        ProcessNSBTXFile(innerFilePtr, innerFilesize, innerFilePath);
                }
                else if (pass == 1)
                {
                    // bmbl file
                    if (strcmp(data_020ef145, extension) == 0)
                        ProcessBMBLFile(innerFilePtr, innerFilesize);
                    // dat file
                    else if (strcmp(data_020ef14b, extension) == 0)
                    {
                        SafeAllocator* alloc = pAllocator_68_;
                        unsigned int decompressedSize;
                        const void* decompressed = DecompressLZ77FileIntoScratchSpace(*alloc, innerFilePtr, decompressedSize);
                        func_0205e104(unknown_c_, alloc, decompressed, decompressedSize);
                    }
                    // bpos file. From testing these seem to be a grotto thing
                    else if (strcmp(data_020ef150, extension) == 0)
                        ProcessBPOSFile(innerFilePtr, innerFilesize);
                }
                fileID++;
            }
            narc.Destroy();
        }
        if (pass == 0)
        {
            unsigned int allocSize = pAllocator_4c_->GetMaxPossibleAllocation();
            void* memory = pAllocator_4c_->Allocate(allocSize);
            if (memory == NULL)
                func_020c9be0();
            internalAllocator_.ResetAllocatorPointer();
            internalAllocator_.CreateTypeA(memory, allocSize);
            pAllocator_68_ = &internalAllocator_;
            internalAllocator_.Reset();
        }
    }
    loader->RemoveTask(mapAMBLLoadHandle_);
    mapAMBLLoadHandle_ = -1;
    func_02014b04(this);
    return true;
}

bool Zone3D::ProcessBMBLFile(const void* filedata, unsigned int /*filesize*/)
{
    SafeAllocator* allocator = pAllocator_68_;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    Zone3D_StructPtr_8* ptr8 = pUnknownStruct_8_;

    func_0201e248(&unknownBData_);
    func_0201e2b4(&unknownBData_, allocator, decompressed, decompressedLength);
    pUnknownStruct_8_ = ptr8; // why?
    return true;
}

bool Zone3D::ProcessBPOSFile(const void* filedata, unsigned int /*filesize*/)
{
    SafeAllocator* allocator = pAllocator_68_;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    Zone3D_StructPtr_8* ptr8 = pUnknownStruct_8_;

    func_0201e2b4(&unknownBData_, allocator, decompressed, decompressedLength);
    pUnknownStruct_8_ = ptr8; // why?
    return true;
}

bool Zone3D::ProcessNSBTXFile(const void* filedata, unsigned int filesize, const char* filename)
{
    SafeAllocator* allocator = pAllocator_68_;
    void* graphicsPtr = unknown_ptr_50_;

    Model3DListNode* modelNode = (Model3DListNode*)allocator->Allocate(sizeof(Model3DListNode));
    if (modelNode != NULL)
    {
        modelNode->model_.Clear();
        modelNode->filename_ = NULL;
        modelNode->pNext_ = NULL;
        char* newFilenameBuffer = (char*)allocator->Allocate(strlen(filename) + 1);
        modelNode->filename_ = newFilenameBuffer;
        if (newFilenameBuffer != NULL)
        {
            strcpy(newFilenameBuffer, filename);
            modelNode->pNext_ = firstModel_418_;
            firstModel_418_ = modelNode;
            unsigned int decompressedLength;
            void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
            if (decompressed != NULL)
            {
                func_0207df90(graphicsPtr);
                modelNode->model_.SetRawFile(decompressed, decompressedLength);
                modelNode->model_.ClearRawFileCache();
                modelNode->model_.ProcessRawFile(2);
                func_0207dfac(graphicsPtr);
                NSBXXTex* texture = modelNode->model_.GetTEX0();
                if (texture != NULL)
                {
                    textureImageMemory_ += NSBXX_Tex_GetBlock1Length(texture);
                    texturePaletteMemory_ += NSBXX_Tex_GetBlock4Length(texture);
                }
                bool success = false;
                if (texture != 0)
                {
                    // bit weird, but I guess block 1 starts right after the metadata ends
                    unsigned int textureMetadataLength = texture->block1Offset_;
                    NSBXXTex* copyOfpVar5 = (NSBXXTex*)allocator->Allocate(textureMetadataLength);
                    if (copyOfpVar5 != NULL)
                    {
                        memcpy(copyOfpVar5, texture, textureMetadataLength);
                        modelNode->model_.SetTEX0(copyOfpVar5);
                        success = true;
                    }
                }
                if (!success)
                    modelNode->model_.Clear();
            }
        }
    }
    return true;
}

void Zone3D::LoadMapAMDJ()
{
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    char filenameBuffer[20];
    if (func_0201b5b0(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        if (environ == 0)
            environ = 1;
        if (environ > 5)
            environ = 5;
        sprintf(filenameBuffer, data_020ef156, data_020ef116, environ);
    }
    else if (func_0201b5d8(currentZoneID_))
    {
        int environ = grotto_.GetActiveGrottoEnviron();
        sprintf(filenameBuffer, data_020ef166, data_020ef116, environ);
    }
    else
    {
        if (currentZoneID_ == 10000 || currentZoneID_ == 10100)
        {
            if (unknown_42c_ == 0)
            {
                sprintf(filenameBuffer, data_020ef176, data_020ef116, pUnknownStruct_8_->mapShortName_);
                unknown_42c_++;
            }
            else if (unknown_42c_ == 1)
            {
                sprintf(filenameBuffer, data_020ef182, data_020ef116, pUnknownStruct_8_->mapShortName_);
                unknown_42c_++;
            }
        }
        else
        {
            sprintf(filenameBuffer, data_020ef18e, data_020ef116, pUnknownStruct_8_->mapShortName_);
        }
    }
    mapAMDJLoadHandle_ = loader->QueueLoadFile(filenameBuffer, NULL);
}

bool Zone3D::UnpackMapAMDJ()
{
    if (mapAMDJLoadHandle_ < 0)
        return true;

    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (loader->GetTaskStatus(mapAMDJLoadHandle_) == 0)
        return false;

    // If we get here, loading finished but was not successful
    if (loader->GetDetailedTaskStatus(mapAMDJLoadHandle_) != BackgroundLoader::TaskStatus_Complete)
    {
        loader->RemoveTask(mapAMDJLoadHandle_);
        mapAMDJLoadHandle_ = -1;
        return true;
    }

    void* amdjData;
    unsigned int amdjFilesize;
    loader->GetLoadedFileByID(mapAMDJLoadHandle_, &amdjData, &amdjFilesize);
    NarcHandle narc;
    if (narc.Initialize(data_020ef13a, (unsigned char*)amdjData))
    {
        NitroVM vm;
        unsigned int fileID = 0;
        NitroVM_Initialize(&vm);
        while (PrepareReadFileInNARCByID(&vm, &narc, fileID))
        {
            char innerFilePath[80];
            NitroVM_WriteOutFilePath(&vm, innerFilePath, 80);
            
            const char* extension = strrchr(innerFilePath, '.');
            if (extension == NULL)
            {
                NitroVM_FinishRead(&vm);
                fileID++;
                continue;
            }
            unsigned int innerFilesize = vm.regbase_abc.c.u32 - vm.regbase_abc.b.u32;
            NitroVM_FinishRead(&vm);
            const void* innerFilePtr = narc.GetFileByIndex(fileID);
            if (strcmp(data_020ef199, extension) == 0)
            {
                int numIterations = unknownBData_.arraySize_;
                for (int i = 0; i < numIterations; i++)
                {
                    BData::BStruct* bstr = func_0201e2d8(&unknownBData_, i);
                    if (strstr(innerFilePath, bstr->buffer_10_))
                        ProcessBMDJFile(innerFilePtr, innerFilesize, bstr);
                }
            }
            fileID++;
        }
        for (Zone3D_BMDJStruct* item = firstBMDJStruct_41c_; item != NULL; item = item->pNext_)
        {
            func_02014a24(this, item);
            if (unknown_42c_ == 2) 
                break;
        }
        narc.Destroy();
    }
    loader->RemoveTask(mapAMDJLoadHandle_);
    mapAMDJLoadHandle_ = -1;
    if (unknown_42c_ == 1)
    {
        LoadMapAMDJ();
        return false;
    }

    if (unknown_16_[0] != '\0')
        func_0207a614(&unknown_struct_f4_[0], unknown_16_);
    return true;
}

bool Zone3D::ProcessBMDJFile(const void* filedata, unsigned int filesize, BData::BStruct* misc)
{
    SafeAllocator* allocator = pAllocator_68_;
    Zone3D_BMDJStruct* newStruct = (Zone3D_BMDJStruct*)allocator->Allocate(sizeof(Zone3D_BMDJStruct));
    if (newStruct == NULL)
        return false;

    func_02013454(newStruct);
    newStruct->unknown_0_ = misc->unknown_0_;
    newStruct->vec_48_ = misc->vec_4_;
    unsigned int decompressedLength;
    void* decompressed = DecompressLZ77FileIntoScratchSpace(*allocator, filedata, decompressedLength);
    if (decompressed == NULL)
        return false;

    func_0201f040(&newStruct->unk_4, allocator, decompressed, decompressedLength);
    newStruct->pNext_ = firstBMDJStruct_41c_;
    firstBMDJStruct_41c_ = newStruct;
    return true;
}

#endif