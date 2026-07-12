#pragma once

// All functions return the prior state encoded as an int
// (the old value of cpsr with the relevant bitmask applied).
// You'll often see patterns like
// int oldState = DisableIRQInterrupts();
// ...
// SetIRQInterruptState(oldState);
// to restore a prior state.
#include "DMA.h"

typedef void (*InterruptHandlerProc)();

void WaitForInterrupt(bool onlySubsequent, unsigned int mask);
void OnDMAOrTimerCompletion(int index);
void SetInterruptHandler(unsigned int mask, const void* proc);
InterruptHandlerProc GetInterruptHandler(unsigned int mask);
void SetDMACompletionCallback(int channel, DMACompletionCallback callback, int userdata);
void SetTimerOverflowCallback(int timer, DMACompletionCallback callback, int userdata);


int EnableIRQInterrupts();
int DisableIRQInterrupts();
int SetIRQInterruptState(int);

int DisableIRQAndFIQInterrupts();
int SetIRQAndFIQInterruptState(int);

int GetIRQInterruptState();
int GetProcessorMode();