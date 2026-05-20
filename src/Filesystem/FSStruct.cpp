#include "Filesystem/FSStructs.h"
#include "System/Interrupts.h"
#include <globaldefs.h>

#pragma optimize_for_size off

typedef int(*FixedCommand)(volatile FSStruct72*);

extern FixedCommand data_020ed6c8[];

extern "C" void func_020c7898(void*);
extern "C" void func_020c78e8(volatile void*);

#define C_BITTEST(what, mask) (((what) & (1 << (mask))) ? 1 : 0)

// The result of a test seems to be some sort of enum. I don't know all the
// values, but it looks like:
// 0, 1 and 4 are genuine results of some kind, to be stored
// 7 indicates that the default code should run
// 8 indicates that an instruction was invalid and should be skipped in future

extern "C" void FS72_PopAndUpdateResult(FSStruct72* fs, int result)
{
    int priorIRQState = DisableIRQInterrupts();

    FSStruct72* prev = fs->pPrev;
    FSStruct72* next = fs->pNext;

    if (prev != NULL)
        prev->pNext = next;
    
    if (next != NULL)
        next->pPrev = prev;

    fs->pPrev = NULL;
    fs->pNext = NULL;
    fs->flags &= ~0x4f;
    fs->storedResult = result;

    func_020c78e8(&fs->unknown_sublist_18);

    SetIRQInterruptState(priorIRQState);
}

extern "C" int FS72_ExecuteCommand(FSStruct72* fs, int opcode)
{
    int result;
    int startingFlags = fs->flags;
    NarcHandleInitialPart* nitroHandle = fs->unknown_8;
    int bitmask = 1 << opcode;
    
    if (C_BITTEST(startingFlags, 2))
        nitroHandle->flags_1C |= 0x200;
    else
        nitroHandle->flags_1C |= 0x100;

    if ((nitroHandle->overrideOpcodeFlags & bitmask))
    {
        result = nitroHandle->instructionOverride(fs, opcode);
        switch (result)
        {
        case 0:
        case 1:
        case 4:
            fs->storedResult = result;
            break;
        case 8:
            nitroHandle->overrideOpcodeFlags &= ~bitmask;
            result = 7;
            break;
        }
    }
    else
        result = 7;

    if (result == 7)
        result = data_020ed6c8[opcode](fs);

    if (result == 6)
    {
        if (C_BITTEST(fs->flags, 2))
        {
            int priorIRQState = DisableIRQInterrupts();
            int checkTest = (nitroHandle->flags_1C & 0x200) != 0;
            if (checkTest)
            {
                do
                {
                    func_020c7898(&nitroHandle->unknown_0C);
                    checkTest = (nitroHandle->flags_1C & 0x200) != 0;
                } while (checkTest);
            }
            result = fs->storedResult;
            SetIRQInterruptState(priorIRQState);
        }
    }
    else
    {
        if (!C_BITTEST(fs->flags, 2))
        {
            nitroHandle->flags_1C &= ~0x100;
            FS72_PopAndUpdateResult(fs, result);
        }
        else
        {
            nitroHandle->flags_1C &= ~0x200;
            fs->storedResult = result;
        }
    }
    
    return result;
}

extern "C" int CaseInsensitiveStrncmp(const unsigned char* first, const unsigned char* second, unsigned int len)
{
    unsigned int index = 0;
    // needed to prevent optimizing to len != 0
    unsigned int ZERO = 0;
    if (len > ZERO)
    {
        do
        {
            unsigned int charA = (unsigned char)first[index] - 'A';
            unsigned int charB = (unsigned char)second[index] - 'A';

            if (charA <= 25)
                charA += 'a' - 'A';

            if (charB <= 25)
                charB += 'a' - 'A';

            if (charA != charB)
                return charA - charB;
            index++;
        } while (index < len);
    }
    return 0;
}

extern "C" int FS_ReadBytes(FSReadHandle* handle, void* dst, unsigned int len)
{
    NarcHandleInitialPart* nitroHandle = handle->nitroHandle;
    nitroHandle->flags_1C |= 0x200;

    int result = nitroHandle->loadFileProc_50(nitroHandle, dst, handle->offset, len);
    switch (result)
    {
    case 0:
    case 1:
        nitroHandle->flags_1C &= ~0x200;
        break;
    case 6:
    {
        int priorState = DisableIRQInterrupts();

        int checkTest = (nitroHandle->flags_1C & 0x200) != 0;
        if (checkTest)
        {
            do
            {
                func_020c7898(&nitroHandle->unknown_0C);
                checkTest = (nitroHandle->flags_1C & 0x200) != 0;
            } while (checkTest);
        }

        SetIRQInterruptState(priorState);
        result = nitroHandle->fs_24->storedResult;
    }
    }
    handle->offset += len;
    return result;
}

extern "C" int FS72_LoadDirectoryDataByIndex(FSStruct72* fs, unsigned int fileIndex)
{
    fs->flags |= 4;
    fs->regext.a.ptr = fs->unknown_8;
    fs->regext.c.s32 = 0;
    fs->regext.b.u16.high = 0;
    fs->regext.b.u16.low = fileIndex;
    return FS72_ExecuteCommand(fs, 2);
}

extern "C" int FS72_Command_Load(FSStruct72* fs)
{
    unsigned int oldOffset = fs->regbase.d.u32;
    unsigned int length = fs->regext.c.u32;
    NarcHandleInitialPart* nitroHandle = fs->unknown_8;
    void* dst = fs->regext.a.ptr;

    fs->regbase.d.u32 = oldOffset + length;
    return nitroHandle->loadFileProc_48(nitroHandle, dst, oldOffset, length);
}

extern "C" int FS72_Command_Save(FSStruct72* fs)
{
    unsigned int oldOffset = fs->regbase.d.u32;
    unsigned int length = fs->regext.c.u32;
    NarcHandleInitialPart* nitroHandle = fs->unknown_8;
    void* src = fs->regext.a.ptr;

    fs->regbase.d.u32 = oldOffset + length;
    return nitroHandle->saveFileProc(nitroHandle, src, oldOffset, length);
}

struct FNTMainTableEntry
{
    unsigned int subtableOffset; // relative to FNT base
    unsigned short firstContainedFileID;
    unsigned short numDirectoriesOrParentID; // first (root) entry holds num directories, rest hold parent ID
};

extern "C" int FS72_Command_GetDirectoryData(FSStruct72* fs)
{
    NarcHandleInitialPart* nitroHandle = fs->unknown_8;
    FSRegisterSet* extendedRegs = &fs->regext;
    FNTMainTableEntry tableEntry;
    FSReadHandle readHandle;
    
    readHandle.nitroHandle = nitroHandle;
    readHandle.offset = extendedRegs->b.u16.low * 8 + nitroHandle->nameTableOffset_34;
    
    int result = FS_ReadBytes(&readHandle, &tableEntry, 8);

    if (result == 0)
    {
        fs->regbase.abc = extendedRegs->abc;
    
        if (extendedRegs->b.u16.high == 0 && extendedRegs->c.u32 == 0)
        {
            fs->regbase.b.u16.high = tableEntry.firstContainedFileID;
            fs->regbase.c.u32 = nitroHandle->nameTableOffset_34 + tableEntry.subtableOffset;
        }
        fs->regbase.d.u32 = tableEntry.numDirectoriesOrParentID & 0xfff;
    }

    return result;
}

struct FileDataStore
{
    NarcHandleInitialPart* nitroHandle;
    // If holding data for a file, holds the file ID as a 32-bit value
    // (i.e. top 16 bits are zero).
    // If holding data for a directory, the bottom 16 bits hold its ID
    // without the 0xF000 term, and the top 16 bits hold the ID of the first
    // file in the directory.
    FSRegister fileOrDirID;
    // Only used for directory entries
    unsigned int maybeSubtableOffset;
    unsigned int isDirectory; // 1 = directory, 0 = file
    unsigned int stringLength;
    unsigned char name[128];
};

extern "C" int FS72_Command_GetFileOrDirectoryNameData(FSStruct72* fs)
{
    FileDataStore* pStorage = (FileDataStore*)fs->regext.a.ptr;
    FSReadHandle readHandle;

    readHandle.nitroHandle = fs->unknown_8;
    readHandle.offset = fs->regbase.c.u32;

    unsigned char stringLengthAndType;
    int result = FS_ReadBytes(&readHandle, &stringLengthAndType, 1);

    if (result != 0)
        return result;

    pStorage->stringLength = stringLengthAndType & 0x7f;
    pStorage->isDirectory = ((int)stringLengthAndType >> 7) & 1;
    
    if (pStorage->stringLength == 0)
        return 1;

    if (!fs->regext.b.u32) // don't skip copying the string
    {
        result = FS_ReadBytes(&readHandle, pStorage->name, pStorage->stringLength);
        if (result != 0)
            return result;

        pStorage->name[pStorage->stringLength] = '\0';
    }
    else // do skip over the string
    {
        readHandle.offset += pStorage->stringLength;
    }

    if (pStorage->isDirectory)
    {
        unsigned short directoryID;
        result = FS_ReadBytes(&readHandle, &directoryID, 2);
        if (result != 0)
            return result;
        pStorage->nitroHandle = fs->unknown_8;
        pStorage->fileOrDirID.u16.low = directoryID & 0xfff;
        pStorage->fileOrDirID.u16.high = 0;
        pStorage->maybeSubtableOffset = 0;
    }
    else
    {
        pStorage->nitroHandle = fs->unknown_8;
        pStorage->fileOrDirID.u32 = fs->regbase.b.u16.high;
        fs->regbase.b.u16.high++;
    }
    
    fs->regbase.c.u32 = readHandle.offset;
    return result;
}

extern "C" int FS72_Command_GetFileOrDirectoryByName(FSStruct72* fs)
{
    FileDataStore storage;
    unsigned char* filePath = (unsigned char*)fs->regext.d.ptr;
    int targetIsDirectory = fs->reg8.s32;

    FS72_ExecuteCommand(fs, 2);
    
    // Parse the filename string in terms of tokens (i.e. directory names
    // followed by the final filename).
    if (filePath[0] != '\0')
    {
        do
        {
            int tokenLength = 0;
            // This variable is used for 2 different purposes: mainly to hold
            // a bool (whether the current token refers to a directory or the file)
            // but in the beginning, we use it to hold characters of the string
            int isParsingDirectory;
            goto SkipInitialIncrement;
            while (true)
            {
                tokenLength++;
            SkipInitialIncrement:
                int regularChar = false;
                isParsingDirectory = filePath[tokenLength];
                if (!(isParsingDirectory == '\0' || isParsingDirectory == '/' || isParsingDirectory == '\\'))
                    regularChar = true;
                if (!regularChar)
                    break;
            }

            // If this doesn't run, then isParsingDirectory = '\0' already, i.e. is false
            if (isParsingDirectory != '\0' || targetIsDirectory)
                isParsingDirectory = true;

            if (tokenLength == 0)
                return 1;
            
            // Special treatment for directory names "." (this dir) and ".." (go up a dir)
            if (*filePath == '.')
            {
                if (tokenLength == 1)
                {
                    filePath++;
                    goto loopEnd;
                }
                // single & instead of &&. Probably originally a typo but means
                // the check doesn't short-circuit
                if (tokenLength == 2 & filePath[1] == '.')
                {
                    // base_D holds the parent directory index following initial
                    // execution of opcode 2. (If this runs, it'll be updated to the parent of that)
                    if (fs->regbase.b.u16.low != 0)
                        FS72_LoadDirectoryDataByIndex(fs, fs->regbase.d.u32);
                    filePath += 2;
                    goto loopEnd;
                }
            }

            if (tokenLength > 127)
                return 1;

            fs->regext.a.ptr = &storage;
            fs->regext.b.u32 = 0;
            while (true)
            {
                if (FS72_ExecuteCommand(fs, 3) != 0)
                    return 1;
                
                if (isParsingDirectory == storage.isDirectory && tokenLength == storage.stringLength
                && CaseInsensitiveStrncmp(filePath, storage.name, tokenLength) == 0)
                    break;
            }

            if (isParsingDirectory)
            {
                fs->regext.abc = ((FSRegisterSet*)&storage)->abc;
                filePath += tokenLength;
                FS72_ExecuteCommand(fs, 2);
            }
            else
            {
                if (targetIsDirectory)
                    return 1;

                volatile FileDataStore& volStorage = storage;
                FSRegisterSet* writeLocation = (FSRegisterSet*)fs->reg9.ptr;

                NarcHandleInitialPart* nh = volStorage.nitroHandle;
                unsigned int fileID = volStorage.fileOrDirID.s32;
                writeLocation->a.ptr = nh;
                writeLocation->b.u32 = fileID;
                return 0;
            }

        loopEnd:
            filePath += filePath[0] != '\0' ? 1 : 0;
        } while (filePath[0] != '\0');
    }

    // I think we only get here if the final token represents a directory,
    // i.e. we're looking for a directory in the first place
    if (!targetIsDirectory)
        return 1;
    
    ((FSRegisterSet*)fs->reg9.ptr)->abc = fs->regbase.abc;
    return 0;
}