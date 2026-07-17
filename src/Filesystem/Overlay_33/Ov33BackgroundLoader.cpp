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
    data_ov033_022a2a2c.Populate(fileLoadSpace, capacity, relativePrio);
}

int Ov33BackgroundLoader::Process()
{
    GPCFile* pGPC;
    func_020d970c();    
    ZeroInitGPCPointer(&pGPC);
    unsigned int crc = 0;
    while (true)
    {
        flags_78c_0_ = true;
        // Begin by doing overlay load/unload operations
        Task* pTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, pTask++)
        {
            switch (pTask->type_)
            {
            case TaskType_LoadOverlay:
            {
                pTask->status_ = TaskStatus_InFlight;
                func_020d974c();
                func_020a1a40(pTask->fileLengthOrOverlayID_);
                func_020d970c();
                pTask->status_ = TaskStatus_Complete;
                break;
            }
            case TaskType_UnloadOverlay:
            {
                pTask->status_ = TaskStatus_InFlight;
                func_020d974c();
                func_020a1ccc(pTask->fileLengthOrOverlayID_);
                func_020d970c();
                pTask->status_ = TaskStatus_Complete;
                break;
            }
            default:
                continue;
            }
            // If we get here, we actually did something, so remove the task
            // and shift things up
            Task* qTask = pTask;
            for (int j = i + 1; j < numPendingTasks_; j++, qTask++)
            {
                qTask[0] = qTask[1];
            }
            pTask--;
            numPendingTasks_--;
        }

        bool allCompleteTasksAreExternallyAllocated = true;
        if (lastBlockEnd_118_ != 0)
        {
            Task* pTask = &queuedTasks_[0];
            for (int i = 0; i < numPendingTasks_; i++, pTask++)
            {
                if (pTask->externalAllocator_ == NULL && pTask->status_ == TaskStatus_Complete)
                {
                    allCompleteTasksAreExternallyAllocated = false;
                    break;
                }
            }
        }
        if (allCompleteTasksAreExternallyAllocated)
            lastBlockEnd_118_ = 0;

        if (numPendingTasks_ <= 0)
            goto end;
        if (processLockBits_ != 0)
        {
            flags_78c_0_ = false;
            goto end;
        }

        Task* gpcTask = NULL;
        if (CheckGPCPairValid_020d917c(pGPC, reader_))
        {
            for (int j = 0; j < numPendingTasks_; j++)
            {
                if (queuedTasks_[j].status_ == TaskStatus_Unknown5)
                {
                    flags_78c_0_ = false;
                    goto end;
                }
                else if (queuedTasks_[j].status_ == TaskStatus_Unallocated &&
                    queuedTasks_[j].type_ == TaskType_LoadFromGP2 &&
                    crc == func_01ff860c(queuedTasks_[j].outerFilename_))
                {
                    flags_78c_0_ = false;
                    gpcTask = &queuedTasks_[j];
                    break;
                }
            }
        }
        
        if (gpcTask == NULL)
        {
            if (CheckGPCPairValid_020d917c(pGPC, reader_))
            {
                ResetGPCPair(&pGPC, reader_);
                crc = 0;
                unknown_11c_ = 0;
                flagMaybeGP2OperationInFlight_ = false;
            }
            Task* qTask = &queuedTasks_[0];
            for (int j = 0; j < numPendingTasks_; j++, qTask++)
            {
                if (qTask->status_ == TaskStatus_Unknown5)
                {
                    flags_78c_0_ = false;
                    goto end;
                }
                else if (qTask->status_ == TaskStatus_Unallocated)
                {
                    flags_78c_0_ = false;
                    gpcTask = qTask;
                    break;
                }
            }
        }

        if (gpcTask == NULL)
            goto end;

        gpcTask->status_ = TaskStatus_InFlight;
        Task inFlightTask;
        inFlightTask = *gpcTask; // work with a copy, we'll copy changes back at the end
        flags_78c_3_ = true;
        func_020d974c();
        char fullName[80];
        inFlightTask.GetFullFilename(fullName);
        inFlightTask.pFileData = NULL;
        inFlightTask.fileLengthOrOverlayID_ = 0;
        switch (inFlightTask.type_)
        {
        case TaskType_LoadFileDefault:
        case TaskType_LoadGP1:
        {
            reader_.ZeroInitialize();
            if (!reader_.Open(fullName, false))
                inFlightTask.status_ = TaskStatus_LoadFileFailed;
            else
            {
                if (inFlightTask.type_ == TaskType_LoadFileDefault)
                    inFlightTask.fileLengthOrOverlayID_ = reader_.GetFileSize();
                else
                {
                    CompressionPrefix prefix;
                    *(unsigned int*)&prefix = 0;
                    reader_.Read(&prefix, 4);
                    inFlightTask.fileLengthOrOverlayID_ = prefix.GetDecompressedLength();
                    reader_.Seek(0);
                }
                unsigned int allocSize;
                if (inFlightTask.type_ == TaskType_LoadFileDefault)
                    allocSize = inFlightTask.fileLengthOrOverlayID_;
                else
                    allocSize = (inFlightTask.fileLengthOrOverlayID_ + 7) & ~3;

                if (inFlightTask.externalAllocator_ != NULL)
                {
                    inFlightTask.pFileData = inFlightTask.externalAllocator_->Allocate(allocSize);
                    if (inFlightTask.pFileData == NULL)
                        inFlightTask.status_ = TaskStatus_Unallocated;
                }
                else
                {
                    inFlightTask.pFileData = AllocateInScratchSpace(&inFlightTask.scratchAlloc_, allocSize);
                    if (inFlightTask.pFileData == NULL)
                        inFlightTask.status_ = TaskStatus_Unallocated;
                }

                if (inFlightTask.pFileData != NULL && inFlightTask.status_ == TaskStatus_InFlight)
                {
                    if (inFlightTask.type_ == TaskType_LoadFileDefault)
                    {
                        reader_.Read(inFlightTask.pFileData, inFlightTask.fileLengthOrOverlayID_);
                    }
                    else // TaskType_LoadGP1
                    {
                        unsigned int compressedSize = reader_.GetFileSize();
                        reader_.DecompressBytes(inFlightTask.pFileData, inFlightTask.fileLengthOrOverlayID_,
                            compressedSize, allocSize);
                    }
                }
                else
                {
                    FreeScratchSpace(&inFlightTask.scratchAlloc_);
                    inFlightTask.fileLengthOrOverlayID_ = 0;
                }
                reader_.Close();
            }
            break;
        }
        case TaskType_LoadFromGP2:
        {
            flagMaybeGP2OperationInFlight_ = true;
            GPCFile::Header header;
            unsigned int outputCapacity = scratchSpaceSize_ - lastBlockEnd_118_ - unknown_11c_;
            unsigned char* outputBuffer = (unsigned char*)scratchSpace_ + lastBlockEnd_118_;
            unsigned int outLength = 0;
            bool outSuccess = false;
            if (!CheckGPCPairValid_020d917c(pGPC, reader_) &&
                !LoadAndDecompressGPCHeaderAndInnerFileInfo(&pGPC, reader_, fullName, outputBuffer,
                    outLength, outputCapacity, true, &outSuccess))
            {
                flagMaybeGP2OperationInFlight_ = false;
                if (outSuccess)
                    inFlightTask.status_ = TaskStatus_Unallocated;
                else
                    inFlightTask.status_ = TaskStatus_LoadFileFailed;
            }
            else
            {
                crc = func_01ff860c(inFlightTask.outerFilename_);
                unknown_11c_ += outLength;
                unsigned int innerFileLength = GetGPCInnerFileLengthByName(pGPC, reader_, inFlightTask.innerFilename_);
                unsigned int allocSize = (innerFileLength + 7) & ~3;
                if (inFlightTask.externalAllocator_ != NULL)
                {
                    inFlightTask.pFileData = inFlightTask.externalAllocator_->Allocate(allocSize);
                    if (inFlightTask.pFileData == NULL)
                        inFlightTask.status_ = TaskStatus_Unallocated;
                    else
                    {
                        if (!DecompressFileFromGPCByName(pGPC, reader_, inFlightTask.pFileData, inFlightTask.fileLengthOrOverlayID_, allocSize, inFlightTask.innerFilename_))
                        {
                            inFlightTask.externalAllocator_->Free(inFlightTask.pFileData);
                            inFlightTask.pFileData = NULL;
                            inFlightTask.fileLengthOrOverlayID_ = 0;
                            inFlightTask.status_ = TaskStatus_DecompressionFailed;
                        }
                    }
                }
                else
                {
                    inFlightTask.pFileData = AllocateInScratchSpace(&inFlightTask.scratchAlloc_, allocSize);
                    if (inFlightTask.pFileData == NULL)
                        inFlightTask.status_ = TaskStatus_Unallocated;
                    else
                    {
                        if (!DecompressFileFromGPCByName(pGPC, reader_, inFlightTask.pFileData, inFlightTask.fileLengthOrOverlayID_, allocSize, inFlightTask.innerFilename_))
                        {
                            FreeScratchSpace(&inFlightTask.scratchAlloc_);
                            inFlightTask.pFileData = NULL;
                            inFlightTask.fileLengthOrOverlayID_ = 0;
                            inFlightTask.status_ = TaskStatus_DecompressionFailed;
                        }
                    }
                }
            }
            break; // switch statement
        }
        }
        if (inFlightTask.pFileData != NULL)
            inFlightTask.status_ = TaskStatus_Complete;
        
        func_020d970c();
        flags_78c_3_ = false;
        RefreshCounters();
        Task* pTaskToUpdate = NULL;
        Task* qTask = &queuedTasks_[0];
        for (int i = 0; i < numPendingTasks_; i++, qTask++)
        {
            if (qTask->taskID_ != inFlightTask.taskID_)
                continue;

            if (qTask->status_ == TaskStatus_InFlight)
                pTaskToUpdate = qTask;
            break;
        }

        if (pTaskToUpdate != NULL)
        {
            *pTaskToUpdate = inFlightTask;
        }
        else
        {
            FreeScratchSpace(&inFlightTask.scratchAlloc_);
            if (inFlightTask.status_ == TaskStatus_Complete && inFlightTask.externalAllocator_ != NULL)
                inFlightTask.externalAllocator_->Free(inFlightTask.pFileData);
            if (CheckGPCPairValid_020d917c(pGPC, reader_))
            {
                ResetGPCPair(&pGPC, reader_);
                crc = 0;
                unknown_11c_ = 0;
                flagMaybeGP2OperationInFlight_ = false;
            }
        }

        inFlightTask.ZeroInitialize();
    }

end:
    func_020d974c();
    if (CheckGPCPairValid_020d917c(pGPC, reader_))
    {
        ResetGPCPair(&pGPC, reader_);
        unknown_11c_ = 0;
        flagMaybeGP2OperationInFlight_ = false;
    }
    if (numPendingTasks_ <= 0)
        func_020d9788(5);
    ZeroDestroyGPCPointer(&pGPC);
    return 0;
}