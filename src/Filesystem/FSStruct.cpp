#include "Filesystem/FSInnerDefs.h"
#include "System/Interrupts.h"
#include "System/Memory.h"
#include <globaldefs.h>

// -O2,p optimization seems to be needed here
#pragma optimize_for_size off

typedef int(*FixedCommand)(NitroVM*);

// The result of a test seems to be some sort of enum. I don't know all the
// values, but it looks like:
// 0, 1 and 4 are genuine results of some kind, to be stored
// 7 indicates that the default code should run
// 8 indicates that an instruction was invalid and should be skipped in future
#define FS_RESULT_SUCCESS 0
#define FS_RESULT_FAILURE 1

extern "C" void NitroVM_UnlinkAndStoreResult(NitroVM* vm, int result)
{
    int priorIRQState = DisableIRQInterrupts();

    NitroVM* prev = vm->links.pPrev;
    NitroVM* next = vm->links.pNext;

    if (prev != NULL)
        prev->links.pNext = next;
    
    if (next != NULL)
        next->links.pPrev = prev;

    vm->links.pPrev = NULL;
    vm->links.pNext = NULL;
    // Clear flags 0, 1, 2, 3 and 6
    vm->flags &= ~(
        (1 << NITROVM_FLAG_0) |
        (1 << NITROVM_FLAG_1) |
        (1 << NITROVM_FLAG_2) |
        (1 << NITROVM_FLAG_3) |
        (1 << NITROVM_FLAG_6));
    vm->storedResult = result;

    func_020c78e8(&vm->unknown_sublist_18);

    SetIRQInterruptState(priorIRQState);
}

extern "C" int FS72_ExecuteCommand(NitroVM* vm, int opcode)
{
    int result;
    int machineStartingFlags = vm->flags;
    NitroHandle* nitroHandle = vm->linkedHandle;
    int opcodeMask = 1 << opcode;

    // Bit verbose, but this way works with volatile and non-volatile
    unsigned int handleFlagAdjust = nitroHandle->flags;
    if (GET_FLAG_BIT(machineStartingFlags, NITROVM_FLAG_2))
        handleFlagAdjust |= (1 << NITROHANDLE_FLAG_9);
    else
        handleFlagAdjust |= (1 << NITROHANDLE_FLAG_8);
    nitroHandle->flags = handleFlagAdjust;

    if ((nitroHandle->overrideOpcodeFlags & opcodeMask))
    {
        result = nitroHandle->instructionOverride(vm, opcode);
        switch (result)
        {
        case NITRO_RESULT_0:
        case NITRO_RESULT_1:
        case NITRO_RESULT_4:
            vm->storedResult = result;
            break;
        case NITRO_RESULT_8:
            nitroHandle->overrideOpcodeFlags &= ~opcodeMask;
            result = NITRO_RESULT_7;
            break;
        }
    }
    else
        result = NITRO_RESULT_7;

    if (result == NITRO_RESULT_7)
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
        result = defaultCommands[opcode](vm);
    }

    if (result == NITRO_RESULT_6)
    {
        if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_2))
        {
            int priorIRQState = DisableIRQInterrupts();
            if (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_9))
            {
                do
                {
                    func_020c7898(&nitroHandle->unknown_0C);
                } while (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_9));
            }
            result = vm->storedResult;
            SetIRQInterruptState(priorIRQState);
        }
    }
    else
    {
        if (!GET_FLAG_BIT(vm->flags, NITROVM_FLAG_2))
        {
            nitroHandle->flags &= ~(1 << NITROHANDLE_FLAG_8);
            NitroVM_UnlinkAndStoreResult(vm, result);
        }
        else
        {
            nitroHandle->flags &= ~(1 << NITROHANDLE_FLAG_9);
            vm->storedResult = result;
        }
    }
    
    return result;
}

extern "C" int CaseInsensitiveStrncmp(const unsigned char* first,
    const unsigned char* second, unsigned int len)
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
    NitroHandle* nitroHandle = handle->nitroHandle;
    nitroHandle->flags |= (1 << NITROHANDLE_FLAG_9);

    int result = nitroHandle->loadFileProc_50(nitroHandle, dst, handle->offset, len);
    switch (result)
    {
    case NITRO_RESULT_0:
    case NITRO_RESULT_1:
        nitroHandle->flags &= ~(1 << 9);
        break;
    case NITRO_RESULT_6:
    {
        int priorState = DisableIRQInterrupts();
        if (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_9))
        {
            do
            {
                func_020c7898(&nitroHandle->unknown_0C);
            } while (GET_FLAG_BIT(nitroHandle->flags, NITROHANDLE_FLAG_9));
        }

        SetIRQInterruptState(priorState);
        result = nitroHandle->linkToFirstVM.pNext->storedResult;
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
extern "C" int FS72_LoadDirectoryDataByIndex(NitroVM* vm, unsigned int dirIndex)
{
    vm->flags |= (1 << NITROVM_FLAG_2);
    vm->regext_abc.a.ptr = vm->linkedHandle;
    vm->regext_abc.c.s32 = 0;
    vm->regext_abc.b.u16.high = 0;
    vm->regext_abc.b.u16.low = dirIndex;
    return FS72_ExecuteCommand(vm, NITROVM_OPCODE_2);
}

extern "C" int FS72_Command_Load(NitroVM* vm)
{
    unsigned int oldOffset = vm->regbase_d.u32;
    unsigned int length = vm->regext_abc.c.u32;
    NitroHandle* nitroHandle = vm->linkedHandle;
    void* dst = vm->regext_abc.a.ptr;

    vm->regbase_d.u32 = oldOffset + length;
    return nitroHandle->loadFileProc_48(nitroHandle, dst, oldOffset, length);
}

extern "C" int FS72_Command_Save(NitroVM* vm)
{
    unsigned int oldOffset = vm->regbase_d.u32;
    unsigned int length = vm->regext_abc.c.u32;
    NitroHandle* nitroHandle = vm->linkedHandle;
    void* src = vm->regext_abc.a.ptr;

    vm->regbase_d.u32 = oldOffset + length;
    return nitroHandle->saveFileProc(nitroHandle, src, oldOffset, length);
}

struct FNTMainTableEntry
{
    unsigned int subtableOffset; // relative to FNT base
    unsigned short firstContainedFileID;
    unsigned short numDirectoriesOrParentID; // first (root) entry holds num directories, rest hold parent ID
};

extern "C" int FS72_Command_GetDirectoryData(NitroVM* vm)
{
    NitroHandle* nitroHandle = vm->linkedHandle;
    FSRegisterTriple* extendedRegs = &vm->regext_abc;
    FNTMainTableEntry tableEntry;
    FSReadHandle readHandle;
    
    readHandle.nitroHandle = nitroHandle;
    readHandle.offset = extendedRegs->b.u16.low * 8 + nitroHandle->nameTableOffset_34;
    
    int result = FS_ReadBytes(&readHandle, &tableEntry, 8);

    if (result == NITRO_RESULT_0)
    {
        vm->regbase_abc = *extendedRegs;
    
        if (extendedRegs->b.u16.high == 0 && extendedRegs->c.u32 == 0)
        {
            vm->regbase_abc.b.u16.high = tableEntry.firstContainedFileID;
            vm->regbase_abc.c.u32 = nitroHandle->nameTableOffset_34 + tableEntry.subtableOffset;
        }
        vm->regbase_d.u32 = tableEntry.numDirectoriesOrParentID & 0xfff;
    }

    return result;
}

struct FileDataStore
{
    NitroHandle* nitroHandle;
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

extern "C" int FS72_Command_GetFileOrDirectoryNameData(NitroVM* vm)
{
    FileDataStore* pStorage = (FileDataStore*)vm->regext_abc.a.ptr;
    FSReadHandle readHandle;

    readHandle.nitroHandle = vm->linkedHandle;
    readHandle.offset = vm->regbase_abc.c.u32;

    unsigned char stringLengthAndType;
    int result = FS_ReadBytes(&readHandle, &stringLengthAndType, 1);

    if (result != 0)
        return result;

    pStorage->stringLength = stringLengthAndType & 0x7f;
    pStorage->isDirectory = ((int)stringLengthAndType >> 7) & 1;
    
    if (pStorage->stringLength == 0)
        return NITRO_RESULT_1;

    if (!vm->regext_abc.b.u32) // don't skip copying the string
    {
        result = FS_ReadBytes(&readHandle, pStorage->name, pStorage->stringLength);
        if (result != NITRO_RESULT_0)
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
        if (result != NITRO_RESULT_0)
            return result;
        pStorage->nitroHandle = vm->linkedHandle;
        pStorage->fileOrDirID.u16.low = directoryID & 0xfff;
        pStorage->fileOrDirID.u16.high = 0;
        pStorage->maybeSubtableOffset = 0;
    }
    else
    {
        pStorage->nitroHandle = vm->linkedHandle;
        pStorage->fileOrDirID.u32 = vm->regbase_abc.b.u16.high;
        vm->regbase_abc.b.u16.high++;
    }
    
    vm->regbase_abc.c.u32 = readHandle.offset;
    return result;
}

extern "C" int FS72_Command_GetFileOrDirectoryByName(NitroVM* vm)
{
    FileDataStore storage;
    unsigned char* filePath = (unsigned char*)vm->regext_d.ptr;
    int targetIsDirectory = vm->reg8.s32;

    FS72_ExecuteCommand(vm, NITROVM_OPCODE_2);
    
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
                return NITRO_RESULT_1;
            
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
                    if (vm->regbase_abc.b.u16.low != 0)
                        FS72_LoadDirectoryDataByIndex(vm, vm->regbase_d.u32);
                    filePath += 2;
                    goto loopEnd;
                }
            }

            if (tokenLength > 127)
                return NITRO_RESULT_1;

            vm->regext_abc.a.ptr = &storage;
            vm->regext_abc.b.u32 = 0;
            while (true)
            {
                if (FS72_ExecuteCommand(vm, 3) != NITRO_RESULT_0)
                    return NITRO_RESULT_1;
                
                if (isParsingDirectory == storage.isDirectory && tokenLength == storage.stringLength
                && CaseInsensitiveStrncmp(filePath, storage.name, tokenLength) == 0)
                    break;
            }

            if (isParsingDirectory)
            {
                vm->regext_abc = *((FSRegisterTriple*)&storage);
                filePath += tokenLength;
                FS72_ExecuteCommand(vm, NITROVM_OPCODE_2);
            }
            else
            {
                if (targetIsDirectory)
                    return NITRO_RESULT_1;

                volatile FileDataStore& volStorage = storage;
                NitroFileAccessor* output = (NitroFileAccessor*)vm->reg9.ptr;

                NitroHandle* nh = volStorage.nitroHandle;
                unsigned int fileID = volStorage.fileOrDirID.s32;
                output->handle = nh;
                output->fileID = fileID;
                return NITRO_RESULT_0;
            }

        loopEnd:
            filePath += filePath[0] != '\0' ? 1 : 0;
        } while (filePath[0] != '\0');
    }

    // I think we only get here if the final token represents a directory,
    // i.e. we're looking for a directory in the first place
    if (!targetIsDirectory)
        return NITRO_RESULT_1;
    
    // base registers a, b, c follow the right format for NitroDirectoryMetadata
    *((FSRegisterTriple*)vm->reg9.ptr) = vm->regbase_abc;
    return NITRO_RESULT_0;
}

extern unsigned char data_020f2288[]; // ":/"

#define NITROFS_ID_INVALID 0x10000

extern "C" int FS72_Command_GetPath(NitroVM* vm)
{
    
    FileDataStore storage;
    NitroVM tempVM;

    NitroHandle* nitroHandle = vm->linkedHandle;
    // Initialisation function
    NitroVM_Initialize(&tempVM);
    tempVM.linkedHandle = vm->linkedHandle;
    
    unsigned int targetFileID;

    unsigned int candidateDirID;
    unsigned int numDirectories;
    unsigned int targetDirID;
    
    if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_5)) // looking for a directory
    {
        targetDirID = vm->regbase_abc.b.u16.low;
        targetFileID = NITROFS_ID_INVALID;
    }
    else // looking for a file
    {
        targetFileID = vm->regbase_abc.a.u32;
        if (vm->regext_abc.c.u16.low != 0)
        {
            targetDirID = vm->regext_abc.c.u16.high;
        }
        else
        {
            candidateDirID = 0;
            numDirectories = 0;
            targetDirID = NITROFS_ID_INVALID;
            do
            {
                FS72_LoadDirectoryDataByIndex(&tempVM, candidateDirID);
                if (candidateDirID == 0)
                {
                    // Normally base_d would hold the parent directory id here,
                    // but in the case of id 0 (i.e. the root) it instead holds
                    // the total number of directories
                    numDirectories = tempVM.regbase_d.u32;
                }
                // [for command 3] where to store data about the file/directory entry
                tempVM.regext_abc.a.ptr = &storage;
                // [for command 3] don't bother copying out the name
                tempVM.regext_abc.b.u32 = 1;

                if (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0)
                {
                    do {
                        if (!storage.isDirectory && storage.fileOrDirID.u32 == targetFileID)
                        {
                            // This is equal to candidateDirID. It was written
                            // during the command 2 call inside FS72_LoadDirectoryDataByIndex
                            // and never changed by any of the command 3 calls.
                            targetDirID = tempVM.regbase_abc.b.u16.low;
                            break;
                        }
                    } while (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0);
                }

                if (targetDirID != NITROFS_ID_INVALID)
                    break;
            } while (++candidateDirID < numDirectories);
        }
    }
    
    if (targetDirID == NITROFS_ID_INVALID)
    {
        vm->regext_abc.c.u16.low = 0;
        return NITRO_RESULT_1;
    }

    // Figure out how many bytes we'll need to write.
    // Looks like the assumption is that if not zero, it's because this
    // instruction has previously run with the same file & so the stored
    // values are already correct & don't need to be recomputed
    if (vm->regext_abc.c.u16.low == 0)
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
            FS72_LoadDirectoryDataByIndex(&tempVM, targetDirID);
            do 
            {
                // Get data about the previous directory's parent
                FS72_LoadDirectoryDataByIndex(&tempVM, tempVM.regbase_d.u32);
                tempVM.regext_abc.a.ptr = &storage;
                tempVM.regext_abc.b.u32 = 1; // Don't bother copying the string

                if (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0)
                {
                    do
                    {
                        if (!storage.isDirectory)
                            continue;
                        
                        if (storage.fileOrDirID.u16.low != ancestorDirID)
                            continue;
    
                        totalWriteSize += storage.stringLength + 1;
                        break;
                    } while (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0);
                }
                // This still holds the parent id parameter passed to
                // FS72_LoadDirectoryDataByIndex
                ancestorDirID = tempVM.regbase_abc.b.u16.low;
            } while (ancestorDirID != 0);
        }
        // +1 to account for null terminator
        vm->regext_abc.c.u16.low = totalWriteSize + 1;
        vm->regext_abc.c.u16.high = targetDirID;
    }
    
    if (!vm->regext_abc.a.ptr)
        return NITRO_RESULT_0;

    unsigned int backWriteLocation = vm->regext_abc.c.u16.low;
    unsigned char* writeDst = (unsigned char*)vm->regext_abc.a.ptr;
    if (vm->regext_abc.b.u32 < backWriteLocation)
        return NITRO_RESULT_1;

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

    FS72_LoadDirectoryDataByIndex(&tempVM, targetDirID);
    if (targetFileID != NITROFS_ID_INVALID)
    {
        tempVM.regext_abc.a.ptr = &storage;
        tempVM.regext_abc.b.u32 = 0; // This time, do copy the string
        if (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0)
        {
            do 
            {
                if (!storage.isDirectory && storage.fileOrDirID.u32 == targetFileID)
                    break;
            } while (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0);
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
            FS72_LoadDirectoryDataByIndex(&tempVM, tempVM.regbase_d.u32);
            tempVM.regext_abc.a.ptr = &storage;
            tempVM.regext_abc.b.u32 = 0;
            *(writeDst + backWriteLocation - 1) = '/';
            backWriteLocation--;

            if (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0)
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
                } while (FS72_ExecuteCommand(&tempVM, NITROVM_OPCODE_3) == NITRO_RESULT_0);
            }
            targetDirID = tempVM.regbase_abc.b.u16.low;
        } while (targetDirID != 0);
    }

    return NITRO_RESULT_0;
}

extern "C" int FS72_Command_GetFATEntry(NitroVM* vm)
{
    unsigned int offsets[2];
    FSReadHandle readHandle;

    unsigned int entryIdx = vm->regext_abc.b.u32;
    if (vm->linkedHandle->fatSize <= entryIdx * 8)
        return 1;
    
    readHandle.nitroHandle = vm->linkedHandle;

    // Dumb way to write readHandle.offset = fatOffset + entryIdx * 8
    int off = readHandle.nitroHandle->fatOffset_2C;
    readHandle.offset = entryIdx * 8;
    off += readHandle.offset;
    readHandle.offset = off;

    int result = FS_ReadBytes(&readHandle, &offsets, 8);
    if (result != NITRO_RESULT_0)
        return result;

    vm->regext_abc.a.u32 = offsets[0];
    vm->regext_abc.b.u32 = offsets[1];
    vm->regext_abc.c.u32 = entryIdx;

    return FS72_ExecuteCommand(vm, NITROVM_OPCODE_7);
}

extern "C" int FS72_Command_CopyExtendedRegisters(NitroVM* vm)
{
    vm->regbase_abc.b.u32 = vm->regext_abc.a.u32;
    vm->regbase_d.u32 = vm->regext_abc.a.u32;
    vm->regbase_abc.c.u32 = vm->regext_abc.b.u32;
    vm->regbase_abc.a.u32 = vm->regext_abc.c.u32;
    return NITRO_RESULT_0;
}

extern "C" int FS72_Command_Nop(NitroVM* vm)
{
    return NITRO_RESULT_0;
}

extern "C" unsigned int Nitro_CalculateSignature(const char* str, int len)
{
    unsigned int signature = 0;

    if (len <= 3)
    {
        int idx = 0; 
        if (len > 0)
        {   
            int bitshift = 0;
            
            do {
                if ((unsigned char)str[idx] == '\0')
                    break;
                
                // Convert to lowercase
                unsigned int charValue = (unsigned char)str[idx] - 'A';
                if (charValue <= 'Z' - 'A')
                    charValue += 'a';
                else
                    charValue += 'A';

                idx++;
                signature |= charValue << bitshift;
                bitshift += 8;
            } while (idx < len);
        }
    }

    return signature;
}

extern "C" int DefaultNitroLoadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len)
{
    VectorizedInvertedMemcpy((const unsigned char*)handle->pFileImage + offset, dst, len);
    return NITRO_RESULT_0;
}

extern "C" int DefaultNitroSaveProc(NitroHandle* handle, const void* src, unsigned int offset, unsigned int len)
{
    VectorizedInvertedMemcpy(src, (unsigned char*)handle->pFileImage + offset, len);
    return NITRO_RESULT_0;
}

extern "C" int OverrideNitroLoadProc(NitroHandle* handle, void* dst, unsigned int offset, unsigned int len)
{
    VectorizedInvertedMemcpy((void*)offset, dst, len);
    return NITRO_RESULT_0;
}

extern "C" NitroVM* NitroHandle_020cbc6c(NitroHandle* handle)
{
    int oldState = DisableIRQInterrupts();
    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_5))
    {
        handle->flags &= ~(1 << NITROHANDLE_FLAG_5);
        NitroVM* currentVM = handle->linkToFirstVM.pNext;
        if (currentVM != NULL)
        {
            do {
                NitroVM* nextVM = currentVM->links.pNext;
                if (GET_FLAG_BIT(currentVM->flags, NITROVM_FLAG_1))
                {
                    if (handle->linkToFirstVM.pNext == currentVM)
                        handle->linkToFirstVM.pNext = nextVM;
                    NitroVM_UnlinkAndStoreResult(currentVM, NITRO_RESULT_3);
                    // Effect of this: if last VM in the list has the flag set,
                    // need to do another pass through...?
                    if (nextVM == NULL)
                        nextVM = handle->linkToFirstVM.pNext;
                }
                currentVM = nextVM;
            } while (currentVM != NULL);
        }
    }

    if (!GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_6) && !GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_3))
    {
        NitroVM* vm = handle->linkToFirstVM.pNext;
        if (vm != NULL)
        {
            int flagCleared = !GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_4);
            if (flagCleared)
                handle->flags |= (1 << NITROHANDLE_FLAG_4);
            SetIRQInterruptState(oldState);
            if (flagCleared && (handle->overrideOpcodeFlags & (1 << NITROVM_OPCODE_9)))
                handle->instructionOverride(vm, NITROVM_OPCODE_9);
            oldState = DisableIRQInterrupts();
            vm->flags |= (1 << NITROVM_FLAG_6);
            if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_2))
            {
                func_020c78e8(&vm->unknown_sublist_18);
                SetIRQInterruptState(oldState);
                return NULL;
            }
            else
            {
                SetIRQInterruptState(oldState);
                return vm;
            }
        }
    }

    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_4))
    {
        handle->flags &= ~(1 << NITROHANDLE_FLAG_4);
        if (handle->overrideOpcodeFlags & (1 << NITROVM_OPCODE_10))
        {
            NitroVM tempVM;
            NitroVM_Initialize(&tempVM);
            tempVM.linkedHandle = handle;
            handle->instructionOverride(&tempVM, NITROVM_OPCODE_10);
        }
    }

    if (GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_6))
    {
        handle->flags = (handle->flags & ~(1 << NITROHANDLE_FLAG_6)) | (1 << NITROHANDLE_FLAG_3);
        func_020c78e8(&handle->unknown_14);
    }

    SetIRQInterruptState(oldState);
    return NULL;
}

extern "C" void NitroVM_020cbe80(NitroVM* vm)
{
    NitroHandle* handle = vm->linkedHandle;
    if (vm != NULL)
    {
        do
        {
            int oldState = DisableIRQInterrupts();
            vm->flags |= (1 << 6);
            if (GET_FLAG_BIT(vm->flags, NITROVM_FLAG_2))
            {
                func_020c78e8(&vm->unknown_sublist_18);
                SetIRQInterruptState(oldState);
                break;
            }
            vm->flags |= (1 << NITROVM_FLAG_3);
            SetIRQInterruptState(oldState);

            if (FS72_ExecuteCommand(vm, vm->maybeScheduledCommand) == NITRO_RESULT_6)
                break;

            vm = NitroHandle_020cbc6c(handle);
        } while (vm != NULL);
    }
}

extern "C" CBool NitroVM_ExecuteAndUnlink_020cbf14(NitroVM* vm)
{
    int result = FS72_ExecuteCommand(vm, vm->maybeScheduledCommand);
    NitroVM_UnlinkAndStoreResult(vm, result);

    NitroVM* maybeMainVM = NitroHandle_020cbc6c(vm->linkedHandle);
    if (maybeMainVM != NULL)
        NitroVM_020cbe80(maybeMainVM);

    return vm->storedResult == NITRO_RESULT_0;
}

extern "C" CBool NitroVM_020cbf58(NitroVM* vm, int opcode)
{
    NitroHandle* handle = vm->linkedHandle;
    vm->maybeScheduledCommand = opcode;
    vm->storedResult = NITRO_RESULT_2;
    int opcodeMask = 1 << opcode;
    vm->flags |= (1 << NITROVM_FLAG_0);

    int oldState = DisableIRQInterrupts();
    if (handle->flags & (1 << NITROHANDLE_FLAG_7))
    {
        NitroVM_UnlinkAndStoreResult(vm, 3);
        SetIRQInterruptState(oldState);
        return false;
    }

    // if opcode is between 2 and 8 (inclusive)
    if (opcodeMask & 0x1fc)
        vm->flags |= (1 << NITROVM_FLAG_2);

    NitroVM* lastAttachedVM;
    // At least one of these needs to be a volatile read
    NitroVM* previous = *(NitroVM* volatile*)&vm->links.pPrev;
    NitroVM* next = *(NitroVM* volatile*)&vm->links.pNext;
    
    if (previous)
        previous->links.pNext = next;
    
    lastAttachedVM = (NitroVM*)&handle->linkToFirstVM;
    if (next)
        next->links.pPrev = previous;
    
    if (lastAttachedVM->links.pNext)
    {
        NitroVM* loopVM = lastAttachedVM->links.pNext;
        do {
            lastAttachedVM = loopVM;
            loopVM = loopVM->links.pNext;
        } while (loopVM != NULL);
    }
    
    lastAttachedVM->links.pNext = vm;
    vm->links.pPrev = lastAttachedVM;
    vm->links.pNext = NULL;

    if (!GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_3) && !GET_FLAG_BIT(handle->flags, NITROHANDLE_FLAG_4))
    {
        handle->flags |= (1 << NITROHANDLE_FLAG_4);
        SetIRQInterruptState(oldState);
        if (handle->overrideOpcodeFlags & (1 << NITROVM_OPCODE_9))
            handle->instructionOverride(vm, NITROVM_OPCODE_9);
        int oldState = DisableIRQInterrupts();
        vm->flags |= (1 << NITROVM_FLAG_6);
        if (!GET_FLAG_BIT(vm->flags, NITROVM_FLAG_2))
        {
            SetIRQInterruptState(oldState);
            NitroVM_020cbe80(vm);
            return true;
        }
        SetIRQInterruptState(oldState);
    }
    else
    {
        if (!GET_FLAG_BIT(vm->flags, NITROVM_FLAG_2))
        {
            SetIRQInterruptState(oldState);
            return true;
        }

        do
        {
            func_020c7898(&vm->unknown_sublist_18);
        } while (!(vm->flags & (1 << NITROVM_FLAG_6)));
        SetIRQInterruptState(oldState);
    }
    return NitroVM_ExecuteAndUnlink_020cbf14(vm);
}