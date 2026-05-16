#pragma once

// All functions return the prior state encoded as an int
// (the old value of cpsr with the relevant bitmask applied).
// You'll often see patterns like
// int oldState = DisableIRQInterrupts();
// ...
// SetIRQInterruptState(oldState);
// to restore a prior state.

int EnableIRQInterrupts();
int DisableIRQInterrupts();
int SetIRQInterruptState(int);

int DisableIRQAndFIQInterrupts();
int SetIRQAndFIQInterruptState(int);

int GetIRQInterruptState();
int GetProcessorMode();