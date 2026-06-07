#include "Filesystem/ExtendedNitroVM.h"
#include "Filesystem/FSInnerDefs.h"
#include "System/Cache.h"
#include <globaldefs.h>

//#pragma optimize_for_size off

extern "C"
{
    // Get CRC hash for a null terminated string
    unsigned int func_01ff860c(const char*);

    // Zero memory and flush cache
    void func_020d84f8(void*, unsigned);

    void func_020d970c();
    void func_020d974c();

    void func_020d9788(int);
}

// Seems to hold whether cached file accessors have been saved or not
extern bool data_01ffd998;
// CRC hashes for cached files
extern unsigned int data_01ffd99c[61];
extern NitroFileAccessor data_01ffda90[61];

void ExtendedNitroVM::ZeroInitialize()
{
    func_020d84f8(this, sizeof(ExtendedNitroVM));
    unknown_0 = 0;
    unknown_2 = 0;
}

bool ExtendedNitroVM::CheckUnknownFlagBit4()
{
    switch (unknown_0)
    {
    case 1:
        return GET_FLAG_BIT(machine.flags, NITROVM_FLAG_4);
    case 2:
        return true;
    }
    return false;
}

unsigned int ExtendedNitroVM::GetFileSize()
{
    switch (unknown_0)
    {
    case 1:
        return machine.regbase_abc.c.u32 - machine.regbase_abc.b.u32;
    case 2:
        return 0;
    }
    return 0;
}

bool ExtendedNitroVM::Seek(unsigned int where)
{
    if (!CheckUnknownFlagBit4())
        return false;

    switch (unknown_0)
    {
    case 1:
        return NitroVM_Seek(&machine, where, 0);
    case 2:
        return false;
    }
    return false;
}

bool ExtendedNitroVM::MaybeReset()
{
    bool didSomething = false;

    if (unknown_0 != 1)
    {
    }
    else if (NitroVM_MaybeCompleteTasks_020cca80(&machine))
        didSomething = true;

    func_020d84f8(this, sizeof(ExtendedNitroVM));
    unknown_0 = 0;
    unknown_2 = 0;
    return didSomething;
}

bool ExtendedNitroVM::PrepareRead(const char *filePath, bool skip)
{
    MaybeReset();

    if (!skip)
    {
        unsigned int cacheIndex;
        const char* abridgedPath = filePath;
        if (data_01ffd998)
        {   
            if (abridgedPath[0] == '/')
                abridgedPath++;
            unsigned int targetCRC = func_01ff860c(abridgedPath);

            int searchMax, searchMin;
            searchMin = 0;
            searchMax = 60;
            while (searchMin <= searchMax)
            {
                cacheIndex = searchMin + ((searchMax - searchMin + 1) >> 1);
                unsigned int candidateCRC = data_01ffd99c[cacheIndex];
                if (targetCRC == candidateCRC)
                    goto EscapeBinarySearch;
                else if (targetCRC < candidateCRC)
                    searchMax = cacheIndex - 1;
                else
                    searchMin = cacheIndex + 1;
            }
        }

        cacheIndex = 0xffffffff;
        EscapeBinarySearch:
        if (cacheIndex < 61)
        {
            if (NitroVM_PrepareReadFileByID(&machine, data_01ffda90[cacheIndex]))
                unknown_0 = 1;
        }
        else
        {
            if (NitroVM_PrepareReadFileByPath(&machine, filePath))
                unknown_0 = 1;
        }
    }

    unknown_2 = 0;
    return (unknown_0 != 0);
}

unsigned int ExtendedNitroVM::LoadToBuffer(void *into, unsigned int capacity)
{
    unknown_2 = 0;

    if (!CheckUnknownFlagBit4())
        return false;

    unsigned int length = 0;
    switch (unknown_0)
    {
    case 1:
        length = NitroVM_MaybeExecuteLoad_v1(&machine, into, capacity);
        while (GET_FLAG_BIT(machine.flags, NITROVM_FLAG_0))
            func_020d9788(1);
        break;
    case 2:
        break;
    }

    if (unknown_2 == 0)
        CleanInvalidateCacheRange(into, length);
    else
        length = 0;

    return length;
}

bool ExtendedNitroVM::DoFlagStuff()
{
    bool success = false;
    func_020d970c();

    if (unknown_0 == 1)
    {
        NitroVM_FlagStuff_020ccba8(&machine);
        success = true;
    }
    unknown_2 = 1;
    
    func_020d974c();
    return success;
}