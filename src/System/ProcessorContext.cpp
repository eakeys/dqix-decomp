#include "System/ProcessorContext.h"
#include "System/Interrupts.h"
#include <globaldefs.h>

void BlockCurrentContext(BlockedContextList* blockQueue)
{
    int priorState = DisableIRQInterrupts();

    ProcessorContext* current = *data_021112e0.ppActiveContext;
    if (blockQueue != NULL)
    {
        current->containerBlockedQueue = blockQueue;
        blockQueue->Insert(current);
    }
    current->blockState = CONTEXT_STATE_BLOCKED;
    SwitchContext();
    SetIRQInterruptState(priorState);
}

void UnblockContexts(BlockedContextList* blockQueue)
{
    int priorState = DisableIRQInterrupts();
    if (blockQueue->first != NULL)
    {
        // wtf?
        if (blockQueue->first != NULL)
        {
            do
            {
                ProcessorContext* context = blockQueue->PopFront();
                context->blockState = CONTEXT_STATE_READY;
                context->containerBlockedQueue = NULL;
                context->pNextBlocked = NULL;
                context->pPrevBlocked = NULL;
            } while (blockQueue->first != NULL);
        }
        blockQueue->last = NULL;
        blockQueue->first = NULL;
        SwitchContext();
    }
    SetIRQInterruptState(priorState);
}

void MarkContextReadyAndSwitch(ProcessorContext *context)
{
    int priorState = DisableIRQInterrupts();
    context->blockState = CONTEXT_STATE_READY;
    SwitchContext();
    SetIRQInterruptState(priorState);
}

ProcessorContext* GetFirstReadyContext()
{
    ProcessorContext* context = data_021112e0.substruct_24.firstContext;
    while (context != NULL && context->blockState != CONTEXT_STATE_READY)
    {
        context = context->pNext;
    }
    return context;
}