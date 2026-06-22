#pragma once

struct ProcessorContext;

struct BlockedContextList
{
    ProcessorContext* first;
    ProcessorContext* last;

    // Insertions are sorted in priority order
    void Insert(ProcessorContext* context);
    ProcessorContext* PopFront();
    // Returns the context if it was found and NULL otherwise
    ProcessorContext* Remove(ProcessorContext* context);
};

int GenerateUniqueContextID();

struct ProcessorContext
{
    unsigned int programStatusRegister;
    unsigned int userModeRegisters[15];
    unsigned int resumeAddress;
    unsigned int supervisorStackPointer;

    struct MathRegisters
    {
        unsigned long long div_numer;
        unsigned long long div_denom;
        unsigned long long sqrt_param;
        unsigned short div_control;
        unsigned short sqrt_control;
    } mathRegisters;

    int blockState; // 0 = blocked, 1 = ready, 2 = newly created
    // All the active contexts are stored in a singly linked list
    ProcessorContext* pNext;
    unsigned int uniqueID;
    unsigned int maybeContextPriority; // lower number = high priority
    unsigned int unknown_74;
    BlockedContextList* containerBlockedQueue;
    ProcessorContext* pPrevBlocked;
    ProcessorContext* pNextBlocked;
    unsigned int unknown_84;
    unsigned int unknown_88;
    unsigned int unknown_8C;
    unsigned int stackTop;
    unsigned int stackBottom;
    unsigned int unknown_98;
    BlockedContextList unknown_9C;
    unsigned int unknown_A4;
    unsigned int unknown_A8;
    unsigned int unknown_AC;
    unsigned int unknown_B0;
    unsigned int unknown_B4;
    unsigned int unknown_B8;
    unsigned int unknown_BC;
};

#define STACK_TOP_MAGIC 0x7bf9dd5b
#define STACK_BOTTOM_MAGIC 0xfddb597d

#define CONTEXT_STATE_BLOCKED 0
#define CONTEXT_STATE_READY 1
#define CONTEXT_STATE_NEWLY_CREATED 2

typedef void (*PFNSwitchContextProc)(ProcessorContext*, ProcessorContext*);

struct Struct_02111304
{
    unsigned short unknown_0;
    unsigned short unknown_2;
    ProcessorContext* activeContext;
    // first element of the global context list, sorted by priority
    ProcessorContext* firstContext;
    PFNSwitchContextProc switchContextProcB;
};

struct Struct_021112e0
{
    PFNSwitchContextProc switchContextProcA;
    unsigned int unknown_4;
    ProcessorContext** ppActiveContext; // points to 0x02111308, which holds a pointer to the active context
    int hasSetupPrimaryContext;
    unsigned int unknown_10;
    unsigned int unknown_14;
    unsigned int unknown_18;
    unsigned int unknown_1C;
    int contextUniqueIDCounter; // repeatedly incremented to generate unique ids
    Struct_02111304 substruct_24;
    ProcessorContext contextA; // alias for data_02111314
    ProcessorContext contextB; // alias for data_021113d4
};

#if defined(jpn)
#define data_021112e0 data_02110f80
#define data_02111304 data_02110fa4
#endif

extern Struct_021112e0 data_021112e0;
extern Struct_02111304 data_02111304;
extern ProcessorContext data_02111314;
extern ProcessorContext data_021113d4;

// The two pointers at offset 0x88 and 0x8C in ProcessorContext
// are to the ends of yet another doubly linked list. The entries in this contain
// a BlockedContextList. So it's some sort of 'list of lists' but I have no idea
// what for. This function can pop the first list here. Only implemented to avoid
// having to use another translation unit.
extern "C" void* UnknownImplementedFunction_020c72bc(void*);

ProcessorContext* InsertContextIntoGlobalList(ProcessorContext* context);
void RemoveContextFromGlobalList(ProcessorContext* context);

// switches to the first ready context (with the lowest priority value)
void SwitchContext();

void BlockCurrentContext(BlockedContextList* blockQueue);
void UnblockContexts(BlockedContextList* blockQueue);
void MarkContextReadyAndSwitch(ProcessorContext* context);
ProcessorContext* GetFirstReadyContext();

// low level asm stuff, probably not going in decomp
extern "C" int SaveContext(ProcessorContext* context);
extern "C" void RestoreContext(ProcessorContext* context);