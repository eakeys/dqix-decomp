#pragma once

#include "System/ProcessorContext.h"
#include "LowNitroHandle.h"
#include "FSStructs.h"

typedef void (*PFNNitroCleanup)(NitroHandle*);

// sizeof == 0x60
struct Arm7CardReadData
{
    int unknown_0;
    int unknown_4[23];
};

// sizeof <= 0x620
#define CARTRIDGE_READ_CONTEXT_FLAG_0 0
#define CARTRIDGE_READ_CONTEXT_FLAG_2 2
#define READ_MANAGER_FLAG_CONTEXT_HAS_TASK_PENDING 3
#define CARTRIDGE_READ_CONTEXT_FLAG_4 4
#define READ_MANAGER_FLAG_AWAITING_ARM7_ACTION 5
#define CARTRIDGE_READ_CONTEXT_FLAG_6 6

struct CardReadManager
{
    typedef void (*PFNCartridgeRead)(CardReadManager*);

    Arm7CardReadData* pSharedData;
    int unknown_4;
    struct CardReadManagerLock {
        // two of {owner, multiplicity, taskType} should probably be volatile,
        // but not all three. see the lock & initialize functions
        volatile int owner;
        int multiplicity;
        BlockedContextList waitingContexts;
        int taskType;
    } lock;
    unsigned int cartridgeReadOffset;
    unsigned char* writeDst;
    unsigned int writeLength;
    unsigned int dmaChannel;
    int unknown_2c[3];
    PFNNitroCleanup maybeCleanupProc_38;
    NitroHandle* handle_3c;
    PFNCartridgeRead cartridgeReadProc;
    // populated as a call within func_020cfe08
    ProcessorContext cartridgeReadContext;
    ProcessorContext* pContext_104; // often points to the prior
    // Set to 4 and 8 in different places
    unsigned int contextPriority_108;
    BlockedContextList list_10C;
    // bit 3: set when there is a task to be done by the read context
    volatile unsigned int flags_114;
    // see func_020d0a5c
    unsigned int maybeInstructionCacheLimit_118;
    unsigned int maybeDataCacheLimit_11c;

    // Start of this seems to have game title, but can't find where it gets
    // copied in
    char unknown[0x100];
    char readContextStack[0x400];
};

void SendTaskToReadContext(CardReadManager::PFNCartridgeRead task);

void LockCardReadManager(unsigned short ownerID, int taskType);
void UnlockCardReadManager(unsigned short ownerID, int taskType);

void InitializeCardReadManager();
int IsCardReadManagerInitialized();
void VerifyCardReadManagerInitialized();


unsigned int SetReadContextPriority(unsigned int priority);
void NitroVM_Command_AcquireCardReadResources(unsigned short ownerID);
void NitroVM_Command_ReleaseCardReadResources(unsigned short ownerID);

#if defined(jpn)
#define func_020d0f28 func_020d29f4
#endif

void SendGamecardBusCommand(unsigned int firstWord, unsigned int secondWord);
unsigned int SetupNormalGamecardBusCommandMode();
void LoadDataFromCartridgeToMemory(unsigned int dmaChannel,
    unsigned int cartridgeOffset, void* dest, unsigned int length,
    PFNNitroCleanup cleanupProc, NitroHandle* handle, CBool unknownBool);

extern "C"
{
    void InitRawReadStructs_020d0ec4();
    void func_020d0f28();
}

void IPCCommand11Proc(unsigned int command, unsigned int argument, unsigned int flag);
void CartridgeReadContextLoop();