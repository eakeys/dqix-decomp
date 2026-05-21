#include "Filesystem/FSStructs.h"
#include "System/Interrupts.h"
#include "System/Memory.h"
#include <globaldefs.h>

// -O2,p optimization seems to be needed here
#pragma optimize_for_size off

typedef int(*FixedCommand)(FSStruct72*);

extern FixedCommand data_020ed6c8[];

extern "C" void func_020c7898(void*);
extern "C" void func_020c78e8(volatile void*);
extern "C" void func_020cc758(FSStruct72*);

#define GET_FLAG_BIT(what, idx) (((what) & (1 << (idx))) ? 1 : 0)

// The result of a test seems to be some sort of enum. I don't know all the
// values, but it looks like:
// 0, 1 and 4 are genuine results of some kind, to be stored
// 7 indicates that the default code should run
// 8 indicates that an instruction was invalid and should be skipped in future
#define FS_RESULT_SUCCESS 0
#define FS_RESULT_FAILURE 1

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
    int opcodeMask = 1 << opcode;
    
    if (GET_FLAG_BIT(startingFlags, 2))
        nitroHandle->flags_1C |= 0x200;
    else
        nitroHandle->flags_1C |= 0x100;

    if ((nitroHandle->overrideOpcodeFlags & opcodeMask))
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
            nitroHandle->overrideOpcodeFlags &= ~opcodeMask;
            result = 7;
            break;
        }
    }
    else
        result = 7;

    if (result == 7)
    {
        static const FixedCommand defaultCommands[] = {
            &FS72_Command_Load,
            &FS72_Command_Save,
            &FS72_Command_GetDirectoryData,
            &FS72_Command_GetFileOrDirectoryNameData,
            &FS72_Command_GetFileOrDirectoryByName,
            &FS72_Command_GetPath,
            &FS72_Command_GetFATEntry,
            &FS72_Command_CopyExtendedRegisters,
            &FS72_Command_Nop
        };
        result = defaultCommands[opcode](fs);
    }

    if (result == 6)
    {
        if (GET_FLAG_BIT(fs->flags, 2))
        {
            int priorIRQState = DisableIRQInterrupts();
            if (GET_FLAG_BIT(nitroHandle->flags_1C, 9))
            {
                do
                {
                    func_020c7898(&nitroHandle->unknown_0C);
                } while (GET_FLAG_BIT(nitroHandle->flags_1C, 9));
            }
            result = fs->storedResult;
            SetIRQInterruptState(priorIRQState);
        }
    }
    else
    {
        if (!GET_FLAG_BIT(fs->flags, 2))
        {
            nitroHandle->flags_1C &= ~(1 << 8);
            FS72_PopAndUpdateResult(fs, result);
        }
        else
        {
            nitroHandle->flags_1C &= ~(1 << 9);
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
            unsigned int charA = first[index] - 'A';
            unsigned int charB = second[index] - 'A';

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
        if (GET_FLAG_BIT(nitroHandle->flags_1C, 9))
        {
            do
            {
                func_020c7898(&nitroHandle->unknown_0C);
            } while (GET_FLAG_BIT(nitroHandle->flags_1C, 9));
        }

        SetIRQInterruptState(priorState);
        result = nitroHandle->fs_24->storedResult;
    }
    }
    handle->offset += len;
    return result;
}

// Sets up and executes command 2 with 'proper' registers (i.e. stuff that
// needs to be zero for proper behaviour is set to zero). This means, afterwards
// base_b_low holds the id of this directory (i.e. input parameter)
// base_b_high holds the id of the first file in the directory
// base_c holds an offset to the relevant FNT subtable (in particular, you're
// ready to iteratively run command 3 to get data about each contained file/subdir)
extern "C" int FS72_LoadDirectoryDataByIndex(FSStruct72* fs, unsigned int dirIndex)
{
    fs->flags |= 4;
    fs->regext.a.ptr = fs->unknown_8;
    fs->regext.c.s32 = 0;
    fs->regext.b.u16.high = 0;
    fs->regext.b.u16.low = dirIndex;
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
                // the check doesn't short-circuit (and the bools cast to int)
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

extern unsigned char data_020f2288[]; // ":/"

#define NITROFS_ID_INVALID 0x10000

extern "C" int FS72_Command_GetPath(FSStruct72* fs)
{
    
    FileDataStore storage;
    FSStruct72 tempFS;

    NarcHandleInitialPart* nitroHandle = fs->unknown_8;
    // Initialisation function
    func_020cc758(&tempFS);
    tempFS.unknown_8 = fs->unknown_8;
    
    unsigned int targetFileID;

    unsigned int candidateDirID;
    unsigned int numDirectories;
    unsigned int targetDirID;
    
    if (GET_FLAG_BIT(fs->flags, 5)) // looking for a directory
    {
        targetDirID = fs->regbase.b.u16.low;
        targetFileID = NITROFS_ID_INVALID;
    }
    else // looking for a file
    {
        targetFileID = fs->regbase.a.u32;
        if (fs->regext.c.u16.low != 0)
        {
            targetDirID = fs->regext.c.u16.high;
        }
        else
        {
            candidateDirID = 0;
            numDirectories = 0;
            targetDirID = NITROFS_ID_INVALID;
            do
            {
                FS72_LoadDirectoryDataByIndex(&tempFS, candidateDirID);
                if (candidateDirID == 0)
                {
                    // Normally base_d would hold the parent directory id here,
                    // but in the case of id 0 (i.e. the root) it instead holds
                    // the total number of directories
                    numDirectories = tempFS.regbase.d.u32;
                }
                // [for command 3] where to store data about the file/directory entry
                tempFS.regext.a.ptr = &storage;
                // [for command 3] don't bother copying out the name
                tempFS.regext.b.u32 = 1;

                if (FS72_ExecuteCommand(&tempFS, 3) == 0)
                {
                    do {
                        if (!storage.isDirectory && storage.fileOrDirID.u32 == targetFileID)
                        {
                            // This is the equal to candidateDirID. It was written
                            // during the command 2 call inside FS72_LoadDirectoryDataByIndex
                            // and never changed by any of the command 3 calls.
                            targetDirID = tempFS.regbase.b.u16.low;
                            break;
                        }
                    } while (FS72_ExecuteCommand(&tempFS, 3) == 0);
                }

                if (targetDirID != NITROFS_ID_INVALID)
                    break;
            } while (++candidateDirID < numDirectories);
        }
    }
    
    if (targetDirID == NITROFS_ID_INVALID)
    {
        fs->regext.c.u16.low = 0;
        return 1;
    }

    // Figure out how many bytes we'll need to write.
    // Looks like the assumption is that if not zero, it's because this
    // instruction has previously run with the same file & so the stored
    // values are already correct & don't need to be recomputed
    if (fs->regext.c.u16.low == 0)
    {
        unsigned int ancestorDirID;
        int totalWriteSize = 0;
        if (nitroHandle->signature <= 0xff)
            totalWriteSize += 1;
        else if (nitroHandle->signature <= 0xff00)
            totalWriteSize += 2;
        else
            totalWriteSize += 3;

        totalWriteSize += 2;
        // If dealing with a file, sum the length of its name now
        if (targetFileID != NITROFS_ID_INVALID)
            totalWriteSize += storage.stringLength;

        ancestorDirID = targetDirID;
        // Loop through ancestor folders (unless we're in the root)
        if (targetDirID != 0)
        {
            FS72_LoadDirectoryDataByIndex(&tempFS, targetDirID);
            do 
            {
                // Get data about the previous directory's parent
                FS72_LoadDirectoryDataByIndex(&tempFS, tempFS.regbase.d.u32);
                tempFS.regext.a.ptr = &storage;
                tempFS.regext.b.u32 = 1; // Don't bother copying the string

                if (FS72_ExecuteCommand(&tempFS, 3) == 0)
                {
                    do
                    {
                        if (!storage.isDirectory)
                            continue;
                        
                        if (storage.fileOrDirID.u16.low != ancestorDirID)
                            continue;
    
                        totalWriteSize += storage.stringLength + 1;
                        break;
                    } while (FS72_ExecuteCommand(&tempFS, 3) == 0);
                }
                // This still holds the parent id parameter passed to
                // FS72_LoadDirectoryDataByIndex
                ancestorDirID = tempFS.regbase.b.u16.low;
            } while (ancestorDirID != 0);
        }
        // +1 to account for null terminator
        fs->regext.c.u16.low = totalWriteSize + 1;
        fs->regext.c.u16.high = targetDirID;
    }
    
    if (!fs->regext.a.ptr)
        return 0;

    unsigned int backWriteLocation = fs->regext.c.u16.low;
    unsigned char* writeDst = (unsigned char*)fs->regext.a.ptr;
    if (fs->regext.b.u32 < backWriteLocation)
        return 1;

    {
        // Ignore the weird assembly, there was a weird quirk of the code that
        // the compiler never produces unless you do something stupid like this.
        // tl;dr signature holds the value nitroHandle->signature, and
        // uselessZero is just 0 and used for a pointless operation later
        unsigned int uselessZero;
        unsigned int signature = (unsigned int)nitroHandle;
        __asm("mov uselessZero, 0");
        signature = *(unsigned int*)signature;
        __asm("b here\nhere:");

        int signatureLength;
        if (signature <= 0xff)
            signatureLength = 1;
        else if (signature <= 0xff00) // shouldn't this really be 0xffff?
            signatureLength = 2;
        else
            signatureLength = 3;
    
        VectorizedInvertedMemcpy(&nitroHandle->signature, writeDst, signatureLength);
        signatureLength = uselessZero + signatureLength;
        // copy ":/"
        VectorizedInvertedMemcpy(data_020f2288, writeDst + signatureLength, 2);
    }

    FS72_LoadDirectoryDataByIndex(&tempFS, targetDirID);
    if (targetFileID != NITROFS_ID_INVALID)
    {
        tempFS.regext.a.ptr = &storage;
        tempFS.regext.b.u32 = 0; // This time, do copy the string
        if (FS72_ExecuteCommand(&tempFS, 3) == 0)
        {
            do 
            {
                if (!storage.isDirectory && storage.fileOrDirID.u32 == targetFileID)
                    break;
            } while (FS72_ExecuteCommand(&tempFS, 3) == 0);
        }
        // The copied string is null-terminated, and we want to also copy the null terminator
        int copySize = storage.stringLength + 1;
        VectorizedInvertedMemcpy(storage.name, writeDst + backWriteLocation - copySize, copySize);
        backWriteLocation -= copySize;
    }
    else
    {
        *(writeDst + backWriteLocation - 1) = '\0';
        backWriteLocation--;
    }

    if (targetDirID != 0)
    {
        do
        {
            FS72_LoadDirectoryDataByIndex(&tempFS, tempFS.regbase.d.u32);
            tempFS.regext.a.ptr = &storage;
            tempFS.regext.b.u32 = 0;
            *(writeDst + backWriteLocation - 1) = '/';
            backWriteLocation--;

            int result = 0;
            if (FS72_ExecuteCommand(&tempFS, 3) == 0)
            {
                do
                {
                    if (!storage.isDirectory) continue;
                    if (storage.fileOrDirID.u16.low != targetDirID) continue;
                    
                    unsigned int tokenLength = storage.stringLength;
                    VectorizedInvertedMemcpy(storage.name,
                        writeDst + backWriteLocation - tokenLength, tokenLength);
                    backWriteLocation -= tokenLength;
                    break;
                } while (FS72_ExecuteCommand(&tempFS, 3) == 0);
            }
            targetDirID = tempFS.regbase.b.u16.low;
        } while (targetDirID != 0);
    }

    return 0;
}

extern "C" int FS72_Command_GetFATEntry(FSStruct72* fs)
{
    unsigned int offsets[2];
    FSReadHandle readHandle;

    unsigned int entryIdx = fs->regext.b.u32;
    if (fs->unknown_8->fatSize <= entryIdx * 8)
        return 1;
    
    readHandle.nitroHandle = fs->unknown_8;

    // Dumb way to write readHandle.offset = fatOffset + entryIdx * 8
    int off = readHandle.nitroHandle->fatOffset_2C;
    readHandle.offset = entryIdx * 8;
    off += readHandle.offset;
    readHandle.offset = off;

    int result = FS_ReadBytes(&readHandle, &offsets, 8);
    if (result != 0)
        return result;

    fs->regext.a.u32 = offsets[0];
    fs->regext.b.u32 = offsets[1];
    fs->regext.c.u32 = entryIdx;

    return FS72_ExecuteCommand(fs, 7);
}

extern "C" int FS72_Command_CopyExtendedRegisters(FSStruct72* fs)
{
    fs->regbase.b.u32 = fs->regext.a.u32;
    fs->regbase.d.u32 = fs->regext.a.u32;
    fs->regbase.c.u32 = fs->regext.b.u32;
    fs->regbase.a.u32 = fs->regext.c.u32;
    return 0;
}

extern "C" int FS72_Command_Nop(FSStruct72* fs)
{
    return 0;
}