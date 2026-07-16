#include "Filesystem/Overlay_33/Ov33BackgroundLoader.h"
#include "Filesystem/GPC.h"

extern "C"
{
    // get crc for string
    unsigned int func_01ff860c(const char*);

    // alternate version of load overlay
    void func_020a1a40(int);
    // alternate unload overlay
    void func_020a1ccc(int);

    // mutex lock/unlock
    void func_020d970c();
    void func_020d974c();

    // sleep thread
    void func_020d9788(int);
}

extern Ov33BackgroundLoader data_ov033_022a2a2c;

void PopulateOv33BackgroundLoader(void* fileLoadSpace, unsigned int capacity, int relativePrio)
{
    BGFileLoad_Populate(&data_ov033_022a2a2c, fileLoadSpace, capacity, relativePrio);
}

int Ov33BackgroundLoader::Process()
{
    GPCFile* pGPC;
    func_020d970c();    
    ZeroInitGPCPointer(&pGPC);
    unsigned int crc = 0;
    while (true)
    {
        this->flags_78c_0 = true;
        Task* pTask = this->queuedTasks;
        for (int i = 0; i < this->numPendingTasks; i++, pTask++)
        {
            switch (pTask->type)
            {
            case TaskType_LoadOverlay:
            {
                pTask->status_32_low = TaskStatus_InFlight;
                func_020d974c();
                func_020a1a40(pTask->fileLengthOrOverlayID);
                func_020d970c();
                pTask->status_32_low = TaskStatus_Complete;
                break;
            }
            case TaskType_UnloadOverlay:
            {
                pTask->status_32_low = TaskStatus_InFlight;
                func_020d974c();
                func_020a1ccc(pTask->fileLengthOrOverlayID);
                func_020d970c();
                pTask->status_32_low = TaskStatus_Complete;
                break;
            }
            default:
                continue;
            }
            // If we get here, we actually did something, so remove the task
            // and shift things up
            Task* qTask = pTask;
            for (int j = i + 1; j < this->numPendingTasks; j++, qTask++)
            {
                qTask[0] = qTask[1];
            }
            pTask--;
            this->numPendingTasks--;
        }

        bool allCompleteTasksAreExternallyAllocated = true;
        if (this->lastBlockEnd_118 != 0)
        {
            Task* pTask = this->queuedTasks;
            for (int i = 0; i < this->numPendingTasks; i++, pTask++)
            {
                if (pTask->externalAllocator == NULL && pTask->status_32_low == TaskStatus_Complete)
                {
                    allCompleteTasksAreExternallyAllocated = false;
                    break;
                }
            }
        }
        if (allCompleteTasksAreExternallyAllocated)
            this->lastBlockEnd_118 = 0;

        if (this->numPendingTasks <= 0)
            break;
        if (this->bits_788 != 0)
        {
            this->flags_78c_0 = false;
            break;
        }

        Task* gpcTask = NULL;
        if (CheckGPCPairValid_020d917c(pGPC, this->reader))
        {
            for (int j = 0; j < this->numPendingTasks; j++)
            {
                if (this->queuedTasks[j].status_32_low == TaskStatus_NewlyCreated)
                {
                    this->flags_78c_0 = false;
                    goto end;
                }
                else if (this->queuedTasks[j].status_32_low == TaskStatus_Unallocated &&
                    this->queuedTasks[j].type == TaskType_LoadFromGP2 &&
                    crc == func_01ff860c(this->queuedTasks[j].outerFilename))
                {
                    this->flags_78c_0 = false;
                    gpcTask = &this->queuedTasks[j];
                    break;
                }
            }
        }
        
        if (gpcTask == NULL)
        {
            if (CheckGPCPairValid_020d917c(pGPC, this->reader))
            {
                ResetGPCPair(&pGPC, this->reader);
                crc = 0;
                this->unknown_11c = 0;
                this->flags_78c_4 = false;
            }
            Task* qTask = this->queuedTasks;
            for (int j = 0; j < this->numPendingTasks; j++, qTask++)
            {
                if (qTask->status_32_low == TaskStatus_NewlyCreated)
                {
                    this->flags_78c_0 = false;
                    goto end;
                }
                else if (qTask->status_32_low == TaskStatus_Unallocated)
                {
                    this->flags_78c_0 = false;
                    gpcTask = qTask;
                    break;
                }
            }
        }

        if (gpcTask == NULL)
            goto end;

        gpcTask->status_32_low = TaskStatus_InFlight;
        Task taskCopy;
        taskCopy = *gpcTask; // call copy-assignment operator instead of copy constructor
        this->flags_78c_3 = true;
        func_020d974c();
        char fullName[80];
        BGFileLoadEntry_GetFullName(&taskCopy, fullName);
        taskCopy.pFileData = NULL;
        taskCopy.fileLengthOrOverlayID = 0;
        switch (taskCopy.type)
        {
        case TaskType_LoadFileDefault:
        case TaskType_LoadGP1:
        {
            this->reader.ZeroInitialize();
            if (!this->reader.Open(fullName, false))
                taskCopy.status_32_low = TaskStatus_LoadFileFailed;
            else
            {
                if (taskCopy.type == TaskType_LoadFileDefault)
                    taskCopy.fileLengthOrOverlayID = this->reader.GetFileSize();
                else
                {
                    CompressionPrefix prefix;
                    *(unsigned int*)&prefix = 0;
                    this->reader.Read(&prefix, 4);
                    taskCopy.fileLengthOrOverlayID = prefix.GetDecompressedLength();
                    this->reader.Seek(0);
                }
                unsigned int allocSize;
                if (taskCopy.type == TaskType_LoadFileDefault)
                    allocSize = taskCopy.fileLengthOrOverlayID;
                else
                    allocSize = (taskCopy.fileLengthOrOverlayID + 7) & ~3;

                if (taskCopy.externalAllocator != NULL)
                {
                    taskCopy.pFileData = taskCopy.externalAllocator->Allocate(allocSize);
                    if (taskCopy.pFileData == NULL)
                        taskCopy.status_32_low = TaskStatus_Unallocated;
                }
                else
                {
                    taskCopy.pFileData = BGFileLoad_AllocateBlock(this, &taskCopy.mainBlockAllocation, allocSize);
                    if (taskCopy.pFileData == NULL)
                        taskCopy.status_32_low = TaskStatus_Unallocated;
                }

                if (taskCopy.pFileData != NULL && taskCopy.status_32_low == TaskStatus_InFlight)
                {
                    if (taskCopy.type == TaskType_LoadFileDefault)
                    {
                        this->reader.Read(taskCopy.pFileData, taskCopy.fileLengthOrOverlayID);
                    }
                    else
                    {
                        unsigned int compressedSize = this->reader.GetFileSize();
                        this->reader.DecompressBytes(taskCopy.pFileData, taskCopy.fileLengthOrOverlayID,
                            compressedSize, allocSize);
                    }
                }
                else
                {
                    BGFileLoad_FreeBlock(this, &taskCopy.mainBlockAllocation);
                    taskCopy.fileLengthOrOverlayID = 0;
                }
                this->reader.Close();
            }
            break;
        }
        case TaskType_LoadFromGP2:
        {
            this->flags_78c_4 = true;
            GPCFile::Header header;
            unsigned int outputCapacity = this->genericFileLoadCapacity_114 - this->lastBlockEnd_118 - this->unknown_11c;
            unsigned char* outputBuffer = (unsigned char*)this->genericFileLoadPtr_110 + this->lastBlockEnd_118;
            unsigned int outLength = 0;
            bool outSuccess = false;
            if (!CheckGPCPairValid_020d917c(pGPC, this->reader) &&
                !LoadAndDecompressGPCHeaderAndInnerFileInfo(&pGPC, this->reader, fullName, outputBuffer,
                    outLength, outputCapacity, true, &outSuccess))
            {
                this->flags_78c_4 = false;
                if (outSuccess)
                    taskCopy.status_32_low = TaskStatus_Unallocated;
                else
                    taskCopy.status_32_low = TaskStatus_LoadFileFailed;
            }
            else
            {
                crc = func_01ff860c(taskCopy.outerFilename);
                this->unknown_11c += outLength;
                unsigned int innerFileLength = GetGPCInnerFileLengthByName(pGPC, this->reader, taskCopy.innerFilename);
                unsigned int allocSize = (innerFileLength + 7) & ~3;
                if (taskCopy.externalAllocator != NULL)
                {
                    taskCopy.pFileData = taskCopy.externalAllocator->Allocate(allocSize);
                    if (taskCopy.pFileData == NULL)
                        taskCopy.status_32_low = TaskStatus_Unallocated;
                    else
                    {
                        if (!DecompressFileFromGPCByName(pGPC, this->reader, taskCopy.pFileData, taskCopy.fileLengthOrOverlayID, allocSize, taskCopy.innerFilename))
                        {
                            taskCopy.externalAllocator->Free(taskCopy.pFileData);
                            taskCopy.pFileData = NULL;
                            taskCopy.fileLengthOrOverlayID = 0;
                            taskCopy.status_32_low = TaskStatus_DecompressionFailed;
                        }
                    }
                }
                else
                {
                    taskCopy.pFileData = BGFileLoad_AllocateBlock(this, &taskCopy.mainBlockAllocation, allocSize);
                    if (taskCopy.pFileData == NULL)
                        taskCopy.status_32_low = TaskStatus_Unallocated;
                    else
                    {
                        if (!DecompressFileFromGPCByName(pGPC, this->reader, taskCopy.pFileData, taskCopy.fileLengthOrOverlayID, allocSize, taskCopy.innerFilename))
                        {
                            BGFileLoad_FreeBlock(this, &taskCopy.mainBlockAllocation);
                            taskCopy.pFileData = NULL;
                            taskCopy.fileLengthOrOverlayID = 0;
                            taskCopy.status_32_low = TaskStatus_DecompressionFailed;
                        }
                    }
                }
            }

            break;
        }
        }
        if (taskCopy.pFileData != NULL)
            taskCopy.status_32_low = TaskStatus_Complete;
        
        func_020d970c();
        this->flags_78c_3 = false;
        BGFileLoad_RecalculateCounters(this);
        Task* pTaskToUpdate = NULL;
        Task* qTask = this->queuedTasks;
        for (int i = 0; i < this->numPendingTasks; i++, qTask++)
        {
            if (qTask->loaderID != taskCopy.loaderID)
                continue;

            if (qTask->status_32_low == TaskStatus_InFlight)
                pTaskToUpdate = qTask;
            break;
        }

        if (pTaskToUpdate != NULL)
        {
            *pTaskToUpdate = taskCopy;
        }
        else
        {
            BGFileLoad_FreeBlock(this, &taskCopy.mainBlockAllocation);
            if (taskCopy.status_32_low == TaskStatus_Complete && taskCopy.externalAllocator != NULL)
                taskCopy.externalAllocator->Free(taskCopy.pFileData);
            if (CheckGPCPairValid_020d917c(pGPC, this->reader))
            {
                ResetGPCPair(&pGPC, this->reader);
                crc = 0;
                this->unknown_11c = 0;
                this->flags_78c_4 = false;
            }
        }

        BGFileLoadEntry_Initialize(&taskCopy);
    }

end:
    func_020d974c();
    if (CheckGPCPairValid_020d917c(pGPC, this->reader))
    {
        ResetGPCPair(&pGPC, this->reader);
        this->unknown_11c = 0;
        this->flags_78c_4 = false;
    }
    if (this->numPendingTasks <= 0)
        func_020d9788(5);
    ZeroDestroyGPCPointer(&pGPC);
    return 0;
}