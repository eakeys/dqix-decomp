#include "Filesystem/NarcHandle.h"
#include "std_library_functions.h"
#include "Filesystem/FSStructs.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileAccessor.h"
#include <globaldefs.h>

#define NARC_HEADER 0x4352414e
#define BYTE_ORDER_MARK 0xfffe

#define SIGNATURE_FATB 0x46415442
#define SIGNATURE_FIMG 0x46494d47
#define SIGNATURE_FNTB 0x464e5442

#pragma optimize_for_size off

struct NarcHeader
{
    unsigned int magic;
    unsigned short byteOrderMark;
    unsigned short version;
    unsigned int fileSize;
    unsigned short headerSize;
    unsigned short numSections;
};

struct NarcSectionHeader
{
    unsigned int magic;
    unsigned int sectionSize;
};

struct FATBlock
{
    NarcSectionHeader generic;
    unsigned short numFiles;
    unsigned short reserved;
    struct FATEntry
    {
        unsigned int startOffset;
        unsigned int endOffset;
    } entries[0];
};

bool IsFileValidNarc(const unsigned char* buffer)
{
    const NarcHeader& narc = *(const NarcHeader*)buffer;
    
    if (narc.magic != NARC_HEADER)
        return false;

    if (narc.byteOrderMark != BYTE_ORDER_MARK)
        return false;

    if (narc.version != 0x0100)
        return false;
    return true;
}

bool NarcHandle::Initialize(const char* signatureString, const unsigned char* buffer)
{
    const NarcSectionHeader* fatPtr = NULL;
    const NarcSectionHeader* fntPtr = NULL;
    const NarcSectionHeader* imgPtr = NULL;

    if (!IsFileValidNarc(buffer))
        return false;

    const NarcHeader& header = *(const NarcHeader*)buffer;
    int i = 0;
    const NarcSectionHeader* sectionPtr = (const NarcSectionHeader*)(buffer + header.headerSize);

    if ((int)header.numSections > 0)
    {
        do
        {
            switch (sectionPtr->magic)
            {
            case SIGNATURE_FATB:
                fatPtr = sectionPtr;
                break;
            case SIGNATURE_FIMG:
                imgPtr = sectionPtr;
                break;
            case SIGNATURE_FNTB:
                fntPtr = sectionPtr;
                break;
            }                

            i++;
            sectionPtr = (const NarcSectionHeader*)((const char*)sectionPtr + sectionPtr->sectionSize);
        } while (i < header.numSections);
        
    }

    NitroHandle_ZeroInit(&initial);
    pNarcFile = buffer;
    pFATBSection = fatPtr;
    unsigned char* imgPostHeader = (unsigned char*)(imgPtr) + 8;
    pFileDataStart = imgPostHeader;

    if (!NitroHandle_AddToList(&initial, signatureString, strlen(signatureString)))
        return false;

    if (NitroHandle_Populate(&initial, imgPostHeader,
            (unsigned int)fatPtr + 12 - (unsigned int)imgPostHeader, fatPtr->sectionSize - 12,
            (unsigned int)fntPtr + 8 - (unsigned int)imgPostHeader, fntPtr->sectionSize - 8,
            NULL, NULL))
        return true;

    NitroHandle_RemoveFromList(&initial);

    return false;
}

bool NarcHandle::Destroy()
{
    if (!NitroHandle_Destroy(&initial))
        return false;

    NitroHandle_RemoveFromList(&initial);
    return true;
}

const void* GetFileFromNARCInMemory(const char* filename)
{
    const unsigned char* addr = NULL;
    NitroVM machine;
    NitroVM_Initialize(&machine);

    if (NitroVM_PrepareReadFileByPath(&machine, filename))
    {
        // after running the previous function, base_B holds offset of file from file data
        addr = (const unsigned char*)((NarcHandle*)machine.linkedHandle)->pFileDataStart + machine.regbase_abc.b.s32;
        NitroVM_MaybeCompleteTasks_020cca80(&machine);
    }
    return addr;
}

const void* NarcHandle::GetFileByIndex(unsigned int idx) const
{
    const FATBlock& fatb = *(const FATBlock*)pFATBSection;
    const void* ret = NULL;

    if (idx < fatb.numFiles)
        ret = (const unsigned char*)pFileDataStart + fatb.entries[idx].startOffset;

    return ret;
}

bool PrepareReadFileInNARCByID(NitroVM* vm, NarcHandle* handle, unsigned int id)
{
    bool success = false;

    if (id < ((FATBlock*)handle->pFATBSection)->numFiles)
    {
        NitroFileAccessor accessor;
        accessor.handle = &handle->initial;
        accessor.fileID = id;
        success = NitroVM_PrepareReadFileByID(vm, accessor);
    }

    return success;
}