#include "Filesystem/BackgroundLoader.h"
#include "std_library_functions.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/FileIO.h"

#define BGFILELOADDATA_FLAG_0 (1 << 0)
#define BGFILELOADDATA_FLAG_1 (1 << 1)
#define BGFILELOADDATA_FLAG_2 (1 << 2)
#define BGFILELOADDATA_FLAG_3 (1 << 3)
#define BGFILELOADDATA_FLAG_4 (1 << 4)
#define BGFILELOADDATA_FLAG_5 (1 << 5)

#pragma dont_inline on

extern "C"
{
    // zero memory
    void func_0200f374(void*, unsigned);
    // get system language
    int func_0200fb08(BattleStruct*);

    // abort() or similar
    void func_020c9be0();

    // some sort of mutex lock/unlock, used in many other places
    void func_020d970c();
    void func_020d974c();

    void func_020d9788(int); // sleep context for n milliseconds

    // Populates context with priority = relPrio + 16
    void func_020d97a8(ProcessorContext* context, void* stackSpace, unsigned stackSize, int relPrio, const void* entry, int arg);
    void func_020d9828(ProcessorContext* context); // thunk for MarkContextReadyAndSwitch

    char* data_020ef8a4[]; // array of subdirectories within data/

    char data_020ef908[]; // "%s%s%s"
    char data_020ef90f[]; // "data/"
}

extern "C" void BGFileLoadEntry_Initialize(BackgroundLoader::Task *entry)
{
    entry->outerFilename[0] = '\0';
    entry->innerFilename[0] = '\0';
    entry->loaderID = 0;
    entry->status_32_low = BackgroundLoader::TaskStatus_Invalid;
    entry->externalAllocator = NULL;
    entry->pFileData = NULL;
    entry->fileLengthOrOverlayID = 0;
    entry->type = 0;
    entry->mainBlockAllocation.firstWord = 0;
    entry->mainBlockAllocation.numWords = 0;
}

void BGFileLoad_ThreadFunction(BackgroundLoader *loader)
{
    while (true)
    {
        loader->Process();
    }
}

extern "C" bool BGFileLoadEntry_GetFullName(BackgroundLoader::Task *entry, char *outName)
{
    sprintf(outName, data_020ef908, // "%s%s%s"
        data_020ef90f, // "data/"
        data_020ef8a4[entry->containerDirectoryIndex], // e.g. "ani/" or "bin/tbox/"
        entry->outerFilename);
    return true;
}

extern "C" BackgroundLoader *BGFileLoad_GetGlobalInstance()
{
    return data_02104304.pFileLoadData;
}

extern "C" void BGFileLoad_Global_Cleanup()
{
    if (data_02104304.pFileLoadData)
        BGFileLoad_Cleanup_02030110(data_02104304.pFileLoadData);
}

extern "C" void BGFileLoad_Global_AddBit()
{
    if (data_02104304.pFileLoadData)
        BGFileLoad_AddBit(data_02104304.pFileLoadData);
}

extern "C" void BGFileLoad_Global_RemoveBit()
{
    if (data_02104304.pFileLoadData)
        BGFileLoad_RemoveBit(data_02104304.pFileLoadData);
}

extern "C" void InitializeBGFileLoadData(BackgroundLoader* data)
{
    data->reader.ZeroInitialize();
    data_02104304.pFileLoadData = data;

    data->genericFileLoadPtr_110 = 0;
    data->genericFileLoadCapacity_114 = 0;
    data->lastBlockEnd_118 = 0;
    data->unknown_11c = 0;
    data->unknown_120 = 0x10000;
    data->numPendingTasks = 0;
    data->bits_788 = 0; 

    data->flags_78c_0 = true;
    data->flags_78c_3 = false;
    data->flags_78c_4 = false;

    BackgroundLoader::Task* pSubstruct = data->queuedTasks;
    for (int i = 0; i < 24; i++, pSubstruct++)
    {
        BGFileLoadEntry_Initialize(pSubstruct);
    }
}

extern "C" void BGFileLoad_Populate(BackgroundLoader *loader, void* fileLoadSpace,
    unsigned int spaceCapacity, int relativePrio)
{
    InitializeBGFileLoadData(loader);
    loader->genericFileLoadPtr_110 = fileLoadSpace;
    loader->genericFileLoadCapacity_114 = spaceCapacity;
    loader->bits_788 = 0;
#ifdef USE_BITFIELD
    ((volatile BackgroundLoader*)loader)->flags_78c_1 = false;
    ((volatile BackgroundLoader*)loader)->flags_78c_0 = true;
    ((volatile BackgroundLoader*)loader)->flags_78c_2 = true;
#else
    ((volatile BGFileLoadData*)loader)->flags_78c = loader->flags_78c & ~BGFILELOADDATA_FLAG_1;
    ((volatile BGFileLoadData*)loader)->flags_78c = loader->flags_78c & ~BGFILELOADDATA_FLAG_0 | BGFILELOADDATA_FLAG_0;
    ((volatile BGFileLoadData*)loader)->flags_78c = loader->flags_78c | BGFILELOADDATA_FLAG_2;
#endif
    func_020d97a8(&loader->context, &data_02104304.stackSpace,
        sizeof(data_02104304.stackSpace), relativePrio,
        &BGFileLoad_ThreadFunction, (int)loader);
    func_020d9828(&loader->context);
}

extern "C" void BGFileLoad_AwaitFlagBits3And4Clear_0202f920(BackgroundLoader* loader)
{
    BGFileLoad_Cleanup_02030110(loader);
    BGFileLoad_AddBit(loader);
    while (true)
    {
        func_020d970c();
        bool canStop = true;
        if (!loader->flags_78c_3 && loader->flags_78c_4 == 0)
            canStop = false;

        func_020d974c();
        if (!canStop)
            return;
        func_020d9788(1);
    }
}

extern "C" void BGFileLoad_AddBit(BackgroundLoader* loader)
{
    if (loader->numPendingTasks == 0 && loader->bits_788 == 0)
        return;

    loader->bits_788 <<= 1;
    loader->bits_788 |= 1;
}

extern "C" void BGFileLoad_RemoveBit(BackgroundLoader* loader)
{
    if (loader->bits_788 == 0)
        return;

    loader->bits_788 >>= 1;
    if (loader->bits_788 == 0)
    {
        loader->flags_78c_2 = false;
        if (loader->numPendingTasks != 0)
            func_020d9788(1);
    }
}

extern "C" void BGFileLoad_ClearAllBits(BackgroundLoader *loader)
{
    if (loader->bits_788 != 0)
        loader->bits_788 = 0;

    loader->flags_78c_2 = false;

    if (loader->numPendingTasks != 0)
        func_020d9788(1);
}

extern "C" int BGFileLoad_CreateFileEntry(BackgroundLoader *loader, const char *filename, int type, const char *innerFile, SafeAllocator &alloc)
{
    int newID = -1;
    func_020d970c();
    int language = func_0200fb08(GetBattleStruct());

    char replacedFilename[80];
    func_0200f374(replacedFilename, sizeof(replacedFilename));
    StringReplaceLanguageTag(filename, replacedFilename, language);

    if (filename != NULL)
    {
        if (loader->numPendingTasks >= 24)
            func_020c9be0();
        else
        {   
            BackgroundLoader::Task* entry = &loader->queuedTasks[loader->numPendingTasks];
            entry->loaderID = data_02104304.counter;
            entry->status_32_low = BackgroundLoader::TaskStatus_Unallocated;
            entry->externalAllocator = &alloc;
            entry->pFileData = NULL;
            entry->type = type;
            if (type == BackgroundLoader::TaskType_LoadFromGP2 && innerFile != NULL)
            {
                char replacedInnerFile[128];
                func_0200f374(replacedInnerFile, sizeof(replacedInnerFile));
                StringReplaceLanguageTag(innerFile, replacedInnerFile, language);
                strcpy(entry->innerFilename, replacedInnerFile);
            }
            for (char* ptr = replacedFilename; *ptr != '\0'; ptr++)
            {
                if (*ptr == '\\')
                    *ptr = '/';
            }

            bool success;
            // presumably this is used because the filename might be like
            // "ROM:/data/bin/blah.gp2" and you want to strip the "ROM:/" prefix
            const char* pathTrueStart = strstr(replacedFilename, data_020ef90f);
            if (pathTrueStart == NULL)
                success = false;
            else
            {
                int subdirIdx;
                const char* candidateSubdir;
                const char* innerDirPtr = pathTrueStart + strlen(data_020ef90f);
                for (subdirIdx = 0; (candidateSubdir = data_020ef8a4[subdirIdx]) != NULL; subdirIdx++)
                {
                    if (strstr(innerDirPtr, candidateSubdir) == innerDirPtr)
                    {
                        unsigned int subdirLength = strlen(candidateSubdir);
                        unsigned int filenameLength = strlen(innerDirPtr + subdirLength);
                        if (filenameLength + 1 > 0x18)
                            func_020c9be0();
                        entry->containerDirectoryIndex = subdirIdx;
                        strcpy(entry->outerFilename, innerDirPtr + subdirLength);
                        success = true;
                        goto BreakOutOfLoopSuccess;
                    }
                }
                success = false;
            }
        BreakOutOfLoopSuccess:
            if (success)
            {
                newID = data_02104304.counter;
                loader->numPendingTasks++;
                data_02104304.counter = (newID + 1) & 0x3ff;
                loader->flags_78c_0 = false;
            }
        }
    }
    func_020d974c();
    return newID;
}

extern "C" int BGFileLoad_QueueOverlayTask(BackgroundLoader *loader, unsigned int id, bool toLoad)
{
    int newID = -1;
    func_020d970c();
    if (loader->numPendingTasks >= 24)
        func_020c9be0();
    else
    {
        BackgroundLoader::Task* entry = &loader->queuedTasks[loader->numPendingTasks];
        entry->loaderID = data_02104304.counter;
        entry->status_32_low = BackgroundLoader::TaskStatus_Unallocated;
        entry->fileLengthOrOverlayID = id;
        entry->type = (toLoad ? BackgroundLoader::TaskType_LoadOverlay : BackgroundLoader::TaskType_UnloadOverlay);

        newID = data_02104304.counter;
        loader->numPendingTasks++;
        data_02104304.counter = (newID + 1) & 0x3ff;
        loader->flags_78c_0 = false;
    }
    func_020d974c();
    return newID;
}

extern "C" int BGFileLoad_QueueLoadFileByName(BackgroundLoader* loader, const char* filename, SafeAllocator& alloc)
{
    return BGFileLoad_CreateFileEntry(loader, filename, BackgroundLoader::TaskType_LoadFileDefault, NULL, alloc);
}

extern "C" int BGFileLoad_QueueLoadGP1Archive(BackgroundLoader* loader, const char* filename, SafeAllocator& alloc)
{
    return BGFileLoad_CreateFileEntry(loader, filename, BackgroundLoader::TaskType_LoadGP1, NULL, alloc);
}

extern "C" int BGFileLoad_QueueLoadFileFromGP2(BackgroundLoader* loader, const char* archive, const char* innerfile, SafeAllocator& alloc)
{
    return BGFileLoad_CreateFileEntry(loader, archive, BackgroundLoader::TaskType_LoadFromGP2, innerfile, alloc);
}

extern "C" int BGFileLoad_QueueLoadOverlay(BackgroundLoader* loader, unsigned int overlayID)
{
    BGFileLoad_QueueOverlayTask(loader, overlayID, true);
}

extern "C" void BGFileLoad_CreateEntryWith32Low5(BackgroundLoader* loader)
{
    func_020d970c();
    if (loader->numPendingTasks > 0)
    {
        if (loader->numPendingTasks >= 24)
            func_020c9be0();
        else
        {
            BackgroundLoader::Task* task = &loader->queuedTasks[loader->numPendingTasks];
            BGFileLoadEntry_Initialize(task);
            task->loaderID = -1;
            task->status_32_low = BackgroundLoader::TaskStatus_NewlyCreated;
            loader->numPendingTasks++;
            loader->flags_78c_0 = false;
        }
    }
    func_020d974c();
}

extern "C" int BGFileLoad_GetTaskStatus_0202fdd0(BackgroundLoader* loader, int taskID)
{
    int status = -1;
    func_020d970c();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = loader->queuedTasks;
        for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
        {
            if (taskID != pTask->loaderID)
                continue;
            if (pTask->status_32_low == BackgroundLoader::TaskStatus_Complete)
                status = 1;
            else if (pTask->status_32_low == BackgroundLoader::TaskStatus_DecompressionFailed)
                status = -1;
            else if (pTask->status_32_low == BackgroundLoader::TaskStatus_LoadFileFailed)
                status = -1;
            else
                status = 0;
            break;
        }
    }
    func_020d974c();
    return status;
}

extern "C" int BGFileLoad_GetFlagBit0(BackgroundLoader* loader)
{
    return loader->flags_78c_0;
}

extern "C" int BGFileLoad_GetDetailedTaskStatus_0202fe68(BackgroundLoader* loader, int taskID)
{
    int status = -1;
    func_020d970c();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = loader->queuedTasks;
        for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
        {
            if (taskID != pTask->loaderID)
                continue;
            status = pTask->status_32_low;
            break;
        }
    }
    func_020d974c();
    return status;
}

extern "C" void BGFileLoad_GetFilePointerById(BackgroundLoader *loader, int id, void **outPtr, unsigned int *outLength)
{
    func_020d970c();
    *outPtr = NULL;
    *outLength = 0;
    if (id >= 0)
    {
        BackgroundLoader::Task* pSubstruct = loader->queuedTasks;
        for (int i = 0; i < loader->numPendingTasks; i++, pSubstruct++)
        {
            if (id != pSubstruct->loaderID)
                continue;
            
            *outPtr = pSubstruct->pFileData;
            *outLength = pSubstruct->fileLengthOrOverlayID;
            break;
        }
    }
    func_020d974c();
}

extern "C" int BGFileLoad_GetFilePointerByName_0202ff34(BackgroundLoader *loader,
    const char *filename, void **outPtr, unsigned int *outLength)
{
    int ret = -1;
    func_020d970c();
    *outPtr = NULL;
    *outLength = 0;
    BackgroundLoader::Task* pSubstruct = loader->queuedTasks;
    for (int i = 0; i < loader->numPendingTasks; i++, pSubstruct++)
    {
        char fullname[80];
        BGFileLoadEntry_GetFullName(pSubstruct, fullname);

        if (strcmp(fullname, filename) == 0)
        {
            if (pSubstruct->status_32_low == BackgroundLoader::TaskStatus_Complete)
            {
                *outPtr = pSubstruct->pFileData;
                *outLength = pSubstruct->fileLengthOrOverlayID;
            }
            ret = pSubstruct->loaderID;
            break;
        }
    }
    func_020d974c();
    return ret;
}

extern "C" int BGFileLoad_GetInnerFilePointerByName_0202ffd8(BackgroundLoader *loader,
    const char *outerFile, const char* innerFile, void **outPtr, unsigned int *outLength)
{
    int ret = -1;
    func_020d970c();
    *outPtr = NULL;
    *outLength = 0;
    BackgroundLoader::Task* pTask = loader->queuedTasks;
    for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
    {
        char fullname[80];
        BGFileLoadEntry_GetFullName(pTask, fullname);

        if (strcmp(fullname, outerFile) == 0 && strcmp(pTask->innerFilename, innerFile) == 0)
        {
            if (pTask->status_32_low == BackgroundLoader::TaskStatus_Complete)
            {
                *outPtr = pTask->pFileData;
                *outLength = pTask->fileLengthOrOverlayID;
            }
            ret = pTask->loaderID;
            break;
        }
    }
    func_020d974c();
    return ret;
}

extern "C" void BGFileLoad_InitOrReset_02030090(BackgroundLoader* loader)
{
    func_020d970c();
    BackgroundLoader::Task* pTask = loader->queuedTasks;
    for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
    {
        BGFileLoad_FreeBlock(loader, &pTask->mainBlockAllocation);
        BGFileLoadEntry_Initialize(pTask);
    }

    loader->numPendingTasks = 0;
    loader->lastBlockEnd_118 = 0;
    loader->bits_788 = 0;
    loader->flags_78c_1 = false;
    loader->flags_78c_0 = true;
    loader->flags_78c_2 = true;

    func_020d974c();
}

extern "C" void BGFileLoad_Cleanup_02030110(BackgroundLoader* loader)
{
    if (loader->numPendingTasks == 0)
        return;

    func_020d970c();
    BackgroundLoader::Task* pTask = loader->queuedTasks;
    for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
    {
        switch (pTask->status_32_low)
        {
        case BackgroundLoader::TaskStatus_Complete:
        {
            BGFileLoad_FreeBlock(loader, &pTask->mainBlockAllocation);
            if (pTask->externalAllocator != NULL)
            {
                pTask->externalAllocator->Free(pTask->pFileData);
            }
            pTask->pFileData = NULL;
            pTask->fileLengthOrOverlayID = 0;
            break;
        }
        case BackgroundLoader::TaskStatus_NewlyCreated:
            continue; 
        }
        pTask->status_32_low = BackgroundLoader::TaskStatus_Unallocated;
    }
    loader->reader.Abort();
    loader->flags_78c_0 = false;
    loader->flags_78c_2 = true;
    func_020d974c();
}

extern "C" void BGFileLoad_MaybeRemoveTask_020301c8(BackgroundLoader* loader, int taskID)
{
    if (taskID < 0)
        return;

    func_020d970c();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = loader->queuedTasks;
        for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
        {
            if (taskID != pTask->loaderID)
                continue;
            
            BGFileLoad_FreeBlock(loader, &pTask->mainBlockAllocation);
            BGFileLoadEntry_Initialize(pTask);

            BackgroundLoader::Task* pLaterTask = &loader->queuedTasks[i];
            for (int j = i + 1; j < loader->numPendingTasks; j++, pLaterTask++)
            {
                *pLaterTask = *(pLaterTask + 1);
            }
            // Now pLaterTask points to index numPendingTasks-1, i.e. 1 past the new end
            BGFileLoadEntry_Initialize(pLaterTask);
            loader->numPendingTasks--;

            int iVar2 = 0;
            BackgroundLoader::Task* qTask = loader->queuedTasks;
            for (int j = 0; j < loader->numPendingTasks; j++, qTask++)
            {
                if (qTask->status_32_low != 5)
                    break;
                iVar2++;
            }
            if (iVar2 > 0)
            {
                BackgroundLoader::Task* qTask = loader->queuedTasks;
                loader->numPendingTasks -= iVar2;
                for (int k = 0; k < loader->numPendingTasks; k++, qTask++)
                {
                    *qTask = *(qTask + iVar2);
                }
                loader->lastBlockEnd_118 = 0;
                loader->flags_78c_1 = false;
            }
            break;
        }
    }
    func_020d974c();
}

extern "C" int BGFileLoad_GetNumQueuedTasks(BackgroundLoader* loader)
{
    return loader->numPendingTasks;
}

extern "C" bool BGFileLoad_GetTaskFilename(BackgroundLoader* loader, int taskID, char* outBuffer)
{
    bool success = false;
    func_020d970c();
    if (taskID >= 0)
    {
        BackgroundLoader::Task* pTask = loader->queuedTasks;
        for (int i = 0; i < loader->numPendingTasks; i++, pTask++)
        {
            if (taskID != pTask->loaderID)
                continue;

            BGFileLoadEntry_GetFullName(pTask, outBuffer);
            success = true;
            break;
        }
    }
    func_020d974c();
    return success;
}

#if 1
extern "C" void* BGFileLoad_AllocateBlock(BackgroundLoader* loader, BackgroundLoader::Task::Allocation* output, unsigned int filesize)
{
    BackgroundLoader::Task::Allocation* orderedBlocks[24];
    unsigned int* returnAddress = NULL;

    func_020d970c();
    if (filesize != 0)
    {
        unsigned int requiredBytesRounded = (filesize + 3) & ~3;
        // interpretation of this is still pretty shaky
        unsigned int leftoverCapacity = loader->genericFileLoadCapacity_114 - loader->unknown_11c;
        
        unsigned int initialNumBlocks = 0;
        unsigned int initialNumUsedWords = 0;
        unsigned int requiredWords = requiredBytesRounded >> 2;        

        // Populate the ordered pointer list
        BackgroundLoader::Task* pTask = loader->queuedTasks;
        for (unsigned int i = 0; i < loader->numPendingTasks; i++, pTask++)
        {
            if (pTask->mainBlockAllocation.numWords == 0)
                continue;

            // insertion sort (later entries get shifted right)
            BackgroundLoader::Task::Allocation** shiftPtr = &orderedBlocks[initialNumBlocks];
            for (int j = initialNumBlocks; j != 0; j--, shiftPtr--)
            {
                if (pTask->mainBlockAllocation.firstWord > shiftPtr[-1]->firstWord)
                    break;
                
                shiftPtr[0] = shiftPtr[-1];
            }
            // perform the insertion
            shiftPtr[0] = &pTask->mainBlockAllocation;
            initialNumBlocks++;
            initialNumUsedWords += pTask->mainBlockAllocation.numWords;
        }

        if (initialNumUsedWords + requiredWords <= (leftoverCapacity >> 2))
        {
            int allocationPosition = -1;

            if (initialNumBlocks == 0)
                allocationPosition = 0;
            else
            {
                BackgroundLoader::Task::Allocation** searchPtr = &orderedBlocks[0];
                if (requiredWords <= orderedBlocks[0]->firstWord)
                    allocationPosition = 0;
                
                if (allocationPosition < 0)
                {
                    for (int i = 1; i < initialNumBlocks; i++, searchPtr++)
                    {
                        // Test to see if we can put it between searchPtr[0]
                        // and searchPtr[1] (that's why we start i from 1).
                        // If so, place it right after the end of searchPtr[0]
                        if (requiredWords + (searchPtr[0]->firstWord + searchPtr[0]->numWords) <= searchPtr[1]->firstWord)
                        {
                            allocationPosition = searchPtr[0]->firstWord;
                            allocationPosition += searchPtr[0]->numWords;
                            break;
                        }
                    }
                }

                // If we still didn't find a spot, try at the end
                // (searchPtr[0] is now the last entry in the sorted list)
                if (allocationPosition < 0)
                {
                    // this makes me think leftover capacity is the amount on the
                    // right of the scratch space
                    if (requiredWords + (searchPtr[0]->firstWord + searchPtr[0]->numWords) <= (leftoverCapacity >> 2))
                    {
                        allocationPosition = searchPtr[0]->firstWord;
                        allocationPosition += searchPtr[0]->numWords;
                    }
                }
            }

            if (allocationPosition >= 0)
            {
                output->firstWord = allocationPosition;
                output->numWords = requiredWords;
                returnAddress = ((unsigned int*)loader->genericFileLoadPtr_110) + allocationPosition;
                BGFileLoad_RecalculateCounters(loader);
            }
        }
    }
    func_020d974c();
    return returnAddress;
}

extern "C" bool BGFileLoad_FreeBlock(BackgroundLoader* loader, BackgroundLoader::Task::Allocation* block)
{
    bool success = false;
    func_020d970c();
    if (block->numWords != 0)
    {
        block->firstWord = 0;
        block->numWords = 0;
        success = true;
        BGFileLoad_RecalculateCounters(loader);
    }
    func_020d974c();
    return success;
}

extern "C" void BGFileLoad_RecalculateCounters(BackgroundLoader* loader)
{
    loader->lastBlockEnd_118 = 0;
    BackgroundLoader::Task* pTask = loader->queuedTasks;
    for (unsigned int i = 0; i < loader->numPendingTasks; i++, pTask++)
    {
        if (pTask->mainBlockAllocation.numWords == 0)
            continue;
        unsigned int sectionEndWord = pTask->mainBlockAllocation.firstWord + pTask->mainBlockAllocation.numWords;
        int newVal = loader->lastBlockEnd_118;
        if (newVal <= sectionEndWord * 4)
            newVal = sectionEndWord * 4;
        loader->lastBlockEnd_118 = newVal;
    }
    unsigned int unk = loader->lastBlockEnd_118 + loader->unknown_11c;
    if (unk > loader->unknown_120)
        loader->unknown_120 = unk;
}
#endif