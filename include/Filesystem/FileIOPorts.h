#pragma once

#include "FSStructs.h"

typedef void (*PFNNitroCleanup)(NitroHandle*);

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