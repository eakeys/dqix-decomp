#pragma once

#include "ExtendedNitroVM.h"
#include "System/ProcessorContext.h"
#include "Memory/SafeAllocator.h"

#define USE_BITFIELD

extern "C" void InitializeBGFileLoadData(struct BackgroundLoader* data);

// sizeof == 0x794 == 1940 bytes. This struct is passed as the first
// parameter to various functions in the 0202fxxx range, and an instance
// of it is constructed in overlay 33's bss section (at 0x022a2a2c, usa).
// It does stuff related to file loading, a layer on top of ExtendedNitroVM.
// A pointer to this instance is then held at 0x02104308 (usa)
struct BackgroundLoader
{
    // We might want an inline constructor here

    // sizeof == 0x44 == 68 bytes
    struct Task
    {
        char outerFilename[0x18];
        char innerFilename[0x18];
        short loaderID; // unrelated to nitro file ID or cache index
        char status_32_low : 4; // probably task status
        char type : 4; // e.g. 2 = archive or similar
        unsigned char containerDirectoryIndex; // this is indexing directories within data/
        SafeAllocator* externalAllocator;
        void* pFileData;
        unsigned int fileLengthOrOverlayID;
        struct Allocation {
            unsigned short firstWord;
            unsigned short numWords;
        }; 
        Allocation mainBlockAllocation;
    };

    enum TaskStatus
    {
        TaskStatus_Invalid = -1,
        TaskStatus_Unallocated = 0,
        TaskStatus_InFlight = 1,
        TaskStatus_Complete = 2, // probably means done
        TaskStatus_DecompressionFailed = 3,
        TaskStatus_LoadFileFailed = 4,
        TaskStatus_NewlyCreated = 5,
    };

    enum TaskType
    {
        TaskType_LoadFileDefault = 0, // seems to be regular file
        TaskType_LoadGP1 = 1, // only used in one place
        TaskType_LoadFromGP2 = 2,
        TaskType_LoadOverlay = 3, // never called in practice?
        TaskType_UnloadOverlay = 4, // never called in practice?
    };

    // I wanted to implement this as a pointer to a function pointer, but it
    // didn't match, whereas virtual stuff did. This is offset 0.
    virtual int Process() = 0;

    // points to 0x022a2a08, which in turn points to the function
    // func_ov_033_022a21a8. The context's execution repeatedly calls this function
    ProcessorContext context;
    ExtendedNitroVM reader;
    void* genericFileLoadPtr_110;
    volatile unsigned int genericFileLoadCapacity_114;
    volatile int lastBlockEnd_118;
    volatile unsigned int unknown_11c;
    int unknown_120;
    volatile int numPendingTasks;
    Task queuedTasks[24];
    volatile unsigned int bits_788; // this gets left shifted and 1s inserted
    volatile int flags_78c_0 : 1;
    volatile int flags_78c_1 : 1;
    volatile int flags_78c_2 : 1;
    volatile int flags_78c_3 : 1;
    volatile int flags_78c_4 : 1;
    int unknown_790;
};

struct Struct_02104304
{
    unsigned short counter;
    BackgroundLoader* pFileLoadData;
    char stackSpace[0x800];
} extern data_02104304;

extern "C"
{
    // usa: func_0202f700
    void BGFileLoadEntry_Initialize(BackgroundLoader::Task* entry);
    // usa: func_0202f744
    void BGFileLoad_ThreadFunction(BackgroundLoader* loader);
    // usa: func_0202f760
    bool BGFileLoadEntry_GetFullName(BackgroundLoader::Task* entry, char* outName);
    // usa: func_0202f798
    BackgroundLoader* BGFileLoad_GetGlobalInstance();

    // usa: func_0202f7a8
    void BGFileLoad_Global_Cleanup();
    // usa: func_0202f7c8
    void BGFileLoad_Global_AddBit();
    // usa: func_0202f7e8
    void BGFileLoad_Global_RemoveBit();
    // usa: func_0202f808
    void InitializeBGFileLoadData(BackgroundLoader* data);
    // usa: func_0202f894
    void BGFileLoad_Populate(BackgroundLoader* loader, void* fileLoadSpace, unsigned int spaceCapacity, int relativePrio);
    // usa: func_0202f920
    void BGFileLoad_AwaitFlagBits3And4Clear_0202f920(BackgroundLoader* loader);
    // usa: func_0202f984
    void BGFileLoad_AddBit(BackgroundLoader* loader);
    // usa: func_0202f9b4
    void BGFileLoad_RemoveBit(BackgroundLoader* loader);
    // usa: func_0202fa00
    void BGFileLoad_ClearAllBits(BackgroundLoader* loader);
    // usa: func_0202fa38
    int BGFileLoad_CreateFileEntry(BackgroundLoader* loader, const char* filename, int type, const char* innerFile, SafeAllocator& alloc);
    // usa: func_0202fc38
    int BGFileLoad_QueueOverlayTask(BackgroundLoader* loader, unsigned int id, bool toLoad);

    int BGFileLoad_QueueLoadFileByName(BackgroundLoader* loader, const char* filename, SafeAllocator& alloc);
    int BGFileLoad_QueueLoadGP1Archive(BackgroundLoader* loader, const char* filename, SafeAllocator& alloc);
    int BGFileLoad_QueueLoadFileFromGP2(BackgroundLoader* loader, const char* archive, const char* innerfile, SafeAllocator& alloc);
    int BGFileLoad_QueueLoadOverlay(BackgroundLoader* loader, unsigned int overlayID);

    void BGFileLoad_CreateEntryWith32Low5(BackgroundLoader* loader);
    // returns -1, 0 or 1. I believe: -1 = invalid/fail, 0 = pending, 1 = done
    int BGFileLoad_GetTaskStatus_0202fdd0(BackgroundLoader* loader, int taskID);
    int BGFileLoad_GetFlagBit0(BackgroundLoader* loader);
    int BGFileLoad_GetDetailedTaskStatus_0202fe68(BackgroundLoader* loader, int taskID);

    void BGFileLoad_GetFilePointerById(BackgroundLoader* loader, int id, void** outPtr, unsigned int* outLength);
    int BGFileLoad_GetFilePointerByName_0202ff34(BackgroundLoader* loader,
        const char* filename, void** outPtr, unsigned int* outLength);
    int BGFileLoad_GetInnerFilePointerByName_0202ffd8(BackgroundLoader* loader,
        const char* outerFile, const char* innerFile, void** outPtr, unsigned int* outLength);

    // usa: func_02030090
    void BGFileLoad_InitOrReset_02030090(BackgroundLoader* loader);
    // usa: func_02030110
    void BGFileLoad_Cleanup_02030110(BackgroundLoader* loader);
    // usa: func_020301c8
    void BGFileLoad_020301c8(BackgroundLoader* loader, int taskID);
    // func_02030310 is BGFileLoadData::Task::operator=(const Task&)
    // usa: func_02030390
    int BGFileLoad_GetNumQueuedTasks(BackgroundLoader* loader);
    // usa: func_02030398
    bool BGFileLoad_GetTaskFilename(BackgroundLoader* loader, int taskID, char* outBuffer);
    // usa: func_02030400
    void* BGFileLoad_AllocateBlock(BackgroundLoader* loader, BackgroundLoader::Task::Allocation* output, unsigned int filesize);
    // usa: func_02030584
    bool BGFileLoad_FreeBlock(BackgroundLoader* loader, BackgroundLoader::Task::Allocation* block);
    // usa: func_020305c8
    void BGFileLoad_RecalculateCounters(BackgroundLoader* loader);
}