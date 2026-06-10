#include "Filesystem/FileIO.h"
#include "Filesystem/ExtendedNitroVM.h"
#include "Filesystem/FSStructs.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Combat/Main/BattleList.h"
#include "Filesystem/NarcHandle.h"
#include "System/Memory.h"
#include "std_library_functions.h"
#include <globaldefs.h>
#include <asmhacks.h>

extern "C"
{
    // zero memory
    void func_0200f374(void*, unsigned);

    void func_0202f7a8();

    // get system language?
    int func_0200fb08(BattleStruct*);

    // replace <LG> with en, fr, etc.
    void func_020757b4(const char* in, char* out, int lang);

    void func_020d970c();
    void func_020d974c();

    void LZ77UnCompReadNormalWrite8bit(const void* src, void* dst);
}

int MakeCharUpperCase(char ch);

extern unsigned char data_0211e33c[0x30000];

// "ARC"
extern char data_020f0db8[];
// "arc:/"
extern char data_020f0dbc[];

// character to upper case lookup table
extern const char data_020e692c[];

// This doesn't quite match, some register shenanigans to be fixed
/*void* LoadFileIntoMemory(const char* path, void* buffer, unsigned int* outLength)
{
    if (outLength != NULL)
        *outLength = 0;

    BattleStruct* battle = GetBattleStruct();
    
    char replacedPath[128];
    func_0200f374(replacedPath, sizeof(replacedPath));

    int language = func_0200fb08((BattleStruct*)battle);
    func_020757b4(path, replacedPath, language);
    
    void* output = NULL;
    
    if (buffer >= data_0211e33c && buffer < &data_0211e33c[0x30000])
        func_0202f7a8();

    ExtendedNitroVM reader;
    reader.ZeroInitialize();
    
    bool thing;
    __asm("cmp output, 0\n"
        "mov output, 0\n"
        "movne thing, 1\n"
        "moveq thing, output\n");
    DECLARE_ASM_NOP();

    if (reader.PrepareRead(replacedPath, thing))
    {
        output = buffer;
        unsigned int fileSize = reader.LoadToBuffer(buffer, reader.GetFileSize());
        if (outLength != NULL)
            *(unsigned int*)outLength = fileSize;

        reader.MaybeReset();
        if (fileSize == 0)
            return NULL;
    }

    return output;
}*/

void* LoadFileIntoNewAllocation(const char* path, SafeAllocator& alloc, unsigned int* outLength)
{
    BattleStruct* battle = GetBattleStruct();

    char replacedPath[128];
    func_0200f374(replacedPath, sizeof(replacedPath));

    int language = func_0200fb08(battle);
    func_020757b4(path, replacedPath, language);

    if (outLength != NULL)
        *outLength = 0;

    ExtendedNitroVM reader;
    reader.ZeroInitialize();

    void* copyDst = NULL;
    if (reader.PrepareRead(replacedPath, 0))
    {
        unsigned int size = reader.GetFileSize();
        copyDst = alloc.Allocate(size);
        if (copyDst != NULL)
        {
            size = reader.LoadToBuffer(copyDst, size);
            if (outLength != NULL)
                *outLength = size;
        }
        reader.MaybeReset();
        if (size == 0)
            return NULL;
    }

    return copyDst;
}

bool GetFileInNarc(const void *narcBuffer, const char *targetFilePath,
    const void **pOutFilePtr, unsigned int *pOutFileSize, unsigned int firstFileIdx)
{
    bool success = false;
    func_020d970c();
    NarcHandle handle;
    if (handle.Initialize(data_020f0db8, (const unsigned char*)narcBuffer))
    {
        NitroVM machine;
        NitroVM_Initialize(&machine);
        unsigned int idx = firstFileIdx;
        unsigned int prefixLength = strlen(data_020f0dbc);
        while (PrepareReadFileInNARCByID(&machine, &handle, idx))
        {
            char currentFilePath[80];
            NitroVM_WriteOutFilePath(&machine, currentFilePath, sizeof(currentFilePath));
            if (strcmp(currentFilePath + prefixLength, targetFilePath) == 0)
            {
                const void* pFile = handle.GetFileByIndex(idx);
                unsigned int endOffset = machine.regbase_abc.c.u32;
                unsigned int startOffset = machine.regbase_abc.b.u32;
                *pOutFilePtr = pFile;
                *pOutFileSize = endOffset - startOffset;
                strrchr(targetFilePath, '/');
                success = true;
            }
            NitroVM_MaybeCompleteTasks_020cca80(&machine);
            if ((int)success > 0)
                break;
            idx++;
        }
        handle.Destroy();
    }
    func_020d974c();
    return success;
}

bool GetFileInNarcPermissive(const void *narcBuffer, const char *targetFilePath,
    const void **pOutFilePtr, unsigned int *pOutFileSize, unsigned int firstFileIdx)
{
    bool success = false;
    func_020d970c();
    NarcHandle handle;
    if (handle.Initialize(data_020f0db8, (const unsigned char*)narcBuffer))
    {
        NitroVM machine;
        char loopFilePathOutput[80];
        char loopFilePathUpperCase[80];
        char targetUpperCase[80];
        NitroVM_Initialize(&machine);

        int minIdx7 = 0;
        int maxIdx1 = 1000;
        if (handle.pFATBSection != NULL)
            maxIdx1 = handle.pFATBSection->numFiles - 1;
        
        
        VectorizedMemset(targetUpperCase, 0, 80);
        for (int i = 0; i < 80 && targetFilePath[i] != '.' && targetFilePath[i] != '\0'; i++)
        {
            targetUpperCase[i] = MakeCharUpperCase(targetFilePath[i]);
        }
        unsigned int prefixLength = strlen(data_020f0dbc);
        while (minIdx7 <= maxIdx1)
        {
            int i = 0;
            unsigned int midpoint = (minIdx7 + maxIdx1) / 2;
            if (PrepareReadFileInNARCByID(&machine, &handle, midpoint))
            {
                NitroVM_WriteOutFilePath(&machine, loopFilePathOutput, 80);
                VectorizedMemset(loopFilePathUpperCase, 0, 80);
                for (i = 0; i < 80 && loopFilePathOutput[i] != '.' && loopFilePathOutput[i] != '\0'; i++)
                {
                    loopFilePathUpperCase[i] = MakeCharUpperCase(loopFilePathOutput[i]);
                }
                int comparison = strcmp(loopFilePathUpperCase + prefixLength, targetUpperCase);
                if (comparison == 0)
                {
                    const void* pFile = handle.GetFileByIndex(midpoint);
                    unsigned int endOffset = machine.regbase_abc.c.u32;
                    unsigned int startOffset = machine.regbase_abc.b.u32;
                    success = true;
                    *pOutFilePtr = pFile;
                    *pOutFileSize = endOffset - startOffset;
                }
                else if (comparison < 0)
                {
                    minIdx7 = midpoint + 1;
                }
                else
                {
                    maxIdx1 = midpoint - 1;
                }
                NitroVM_MaybeCompleteTasks_020cca80(&machine);
            }
            else
                maxIdx1 = midpoint - 1;

            if ((int)success > 0)
                break;
        }

        handle.Destroy();
    }
    func_020d974c();
    return success;
}

int MakeCharUpperCase(char ch)
{
    if (ch < 0 || ch >= 0x80)
        return ch;
    return (unsigned char)data_020e692c[ch];
}

unsigned int FindFilesInNarcBySubstring(const void *narcBuffer, const char *substr,
    const void **pOutFilePtrs, unsigned int *pOutFileSizes, unsigned int maxOut)
{
    int maxOutputs;
    char* pLoopFilename;
    NitroVM* pMachine;
    int numFound = 0;

    func_020d970c();

    NarcHandle handle;
    if (handle.Initialize(data_020f0db8, (const unsigned char*)narcBuffer))
    {
        NitroVM machine;
        char loopFilename[80];
        NitroVM_Initialize(&machine);
        unsigned int idx = numFound; // 0
        DECLARE_ASM_NOP();
        maxOutputs = maxOut;
        pLoopFilename = loopFilename;  
        pMachine = &machine;
        
        while (PrepareReadFileInNARCByID(pMachine, &handle, idx))
        {
            NitroVM_WriteOutFilePath(pMachine, pLoopFilename, 80);
            if (strstr(pLoopFilename, substr))
            {
                const void* pFile = handle.GetFileByIndex(idx);
                unsigned int endOffset = machine.regbase_abc.c.u32;
                unsigned int startOffset = machine.regbase_abc.b.u32;
                pOutFilePtrs[numFound] = pFile;
                pOutFileSizes[numFound] = endOffset - startOffset;
                // unused?
                strrchr(loopFilename, '/');
                numFound++;
                if (maxOutputs <= numFound)
                {
                    NitroVM_MaybeCompleteTasks_020cca80(&machine);
                    break;
                }
            }
            NitroVM_MaybeCompleteTasks_020cca80(&machine);
            idx++;
        }
        handle.Destroy();
    }
    func_020d974c();
    return numFound;
}

void* DecompressLZ77FileIntoScratchSpace(SafeAllocator& allocator,
    const void* fileData, unsigned int& outDecompressedSize)
{
    // First 4 bytes of fileData are a uint32, whose upper 24 bits
    // store the file size
    outDecompressedSize = *(const unsigned int*)fileData >> 8;
    unsigned char* pAllocatorStart = reinterpret_cast<unsigned char*>(allocator.GetSignedAllocator());
    unsigned int allocBufferSize = allocator.GetSize();

    unsigned char* writeDst = pAllocatorStart + allocBufferSize - outDecompressedSize;
    LZ77UnCompReadNormalWrite8bit(fileData, writeDst);
    return writeDst;
}

void* DecompressLZ77FileIntoAllocatedSpace(SafeAllocator& allocator,
    const void* fileData, unsigned int& outDecompressedSize)
{
    outDecompressedSize = *(const unsigned int*)fileData >> 8;
    void* writeDst = allocator.Allocate(outDecompressedSize);
    if (!writeDst)
        return NULL;
    LZ77UnCompReadNormalWrite8bit(fileData, writeDst);
    return writeDst;
}