#include "Filesystem/FileIO.h"
#include "Filesystem/ExtendedNitroVM.h"
#include "Filesystem/FSStructs.h"
#include "Filesystem/LowNitroHandle.h"
#include "Filesystem/FileAccessor.h"
#include "Filesystem/GPC.h"
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

    // some sort of bit test of a struct member at offset 0
    bool func_02046708(void*, unsigned int);
    // returns pointer to some unknown struct (at 02114e04)
    void* func_020d6c00();

    // get system language?
    int func_0200fb08(BattleStruct*);

    // Looks like a custom implementation of strstr
    char* func_020d2f88(char* searchString, const char* targetString);
    // Custom implementation of strlen
    int func_020d2ff0(const char* str);

    void func_020d970c();
    void func_020d974c();

    void LZ77UnCompReadNormalWrite8bit(const void* src, void* dst);
}

int MakeCharUpperCase(char ch);

extern unsigned char data_0211e33c[0x30000];

// character to upper case lookup table
extern const char data_020e692c[];

extern char* data_020f0da0[]; // array holding "ja", "en", "de", "it", "fr", "es"

// "ARC"
extern char data_020f0db8[];
// "arc:/"
extern char data_020f0dbc[];

extern char data_020f0dc2[]; // "<LG>"

void* LoadFileIntoMemory(const char* path, void* buffer, unsigned int* outLength)
{
    if (outLength != NULL)
        *outLength = 0;

    BattleStruct* battle = GetBattleStruct();
    
    char replacedPath[128];
    func_0200f374(replacedPath, sizeof(replacedPath));

    int language = func_0200fb08((BattleStruct*)battle);
    StringReplaceLanguageTag(path, replacedPath, language);
    
    void* alwaysNull = NULL;
    
    if (buffer >= data_0211e33c && buffer < &data_0211e33c[0x30000])
        func_0202f7a8();

    ExtendedNitroVM reader;
    reader.ZeroInitialize();
    
    void* output = NULL;
    bool alwaysFalse = alwaysNull != NULL;
    DECLARE_ASM_NOP();

    if (reader.PrepareRead(replacedPath, alwaysFalse))
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
}

void* LoadFileIntoNewAllocation(const char* path, SafeAllocator& alloc, unsigned int* outLength)
{
    BattleStruct* battle = GetBattleStruct();

    char replacedPath[128];
    func_0200f374(replacedPath, sizeof(replacedPath));

    int language = func_0200fb08(battle);
    StringReplaceLanguageTag(path, replacedPath, language);

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

/*
// Doesn't quite work, parameters passed to 8-parameter function in the wrong
// order.
extern "C" void* ExtractFileFromGP2(const char* gp2Path, const char* innerFilePath, unsigned int* outSize)
{
    if (outSize != NULL)
        *outSize = 0;

    func_0202f7a8();
    BattleStruct* battle = GetBattleStruct();
    char innerFileReplacedPath[128];
    func_0200f374(innerFileReplacedPath, 128);

    int language = func_0200fb08(battle);
    StringReplaceLanguageTag(innerFilePath, innerFileReplacedPath, language);

    unsigned int metadataLength = 0;
    unsigned int metadataLengthCopy;
    
    unsigned char* storageSpace = data_0211e33c;
    unsigned int storageCapacity = 0x30000;  
    
    
    if (func_02046708(func_020d6c00(), 0x02000000))
        storageCapacity -= 0x8000;
    DECLARE_ASM_NOP();
    GPCReadPair readPair;
    GPCFile** ppGPC = &readPair.pGPCFile;
    ZeroInitGPCPointer(&readPair.pGPCFile);
    readPair.ZeroInitializeMachine();

    void* output;

    bool successHeader = LoadAndDecompressGPCHeaderAndInnerFileInfo(ppGPC, readPair.machine,
        gp2Path, storageSpace, metadataLength, storageCapacity, false, NULL);

    if (successHeader)
    {
        //DECLARE_ASM_NOP();
        unsigned int fileSize = 0;
        metadataLengthCopy = metadataLength;
        DecompressFileFromGPCByName(readPair.pGPCFile, readPair.machine, (unsigned char*)storageSpace + metadataLengthCopy, fileSize, storageCapacity - metadataLengthCopy, innerFileReplacedPath);
        *outSize = fileSize;
        readPair.Reset();
        ZeroDestroyGPCPointer(&readPair.pGPCFile);
        return (unsigned char*)storageSpace + metadataLengthCopy;
    }
    else
    {
        output = NULL;
        readPair.Reset();
        ZeroDestroyGPCPointer(&readPair.pGPCFile);
    }
    
    return output;
}

// This function is properly matched, the above function is the issue
char* StringReplaceLanguageTag(const char* input, char* output, int language)
{
    if (input == NULL || output == NULL)
        return output;

    const char* searchTag = data_020f0dc2;
    
    char *copyOfOutputPtr;
    __asm("mov copyOfOutputPtr, output");

    strcpy(copyOfOutputPtr, input);

    __asm("mov copyOfOutputPtr, output");
    DECLARE_ASM_NOP();
    char* firstTagOccurrence = func_020d2f88(copyOfOutputPtr, searchTag);
    
    const char* replacement = data_020f0da0[language];

    int replacementStringLength = func_020d2ff0(replacement);
    // difference between length of e.g. "en" and "<LG>" (in practice always -2)
    int lengthDifference = func_020d2ff0(replacement) - func_020d2ff0(searchTag);
    while (firstTagOccurrence != NULL)
    {
        if (lengthDifference > 0)
        {
            int lengthRemaining = func_020d2ff0(firstTagOccurrence);
            // difference > 0: replacing with a longer substring. Move everything
            // to the right of the tag to the right. For convenience, we can also
            // move the tag itself
            memmove(firstTagOccurrence + lengthDifference, firstTagOccurrence, lengthRemaining + 1);
        }
        else if (lengthDifference < 0)
        {
            int lengthRemaining = func_020d2ff0(firstTagOccurrence);
            // difference < 0: replacing with a shorter substring. Move everything
            // to the left (including the later part of the tag)
            memmove(firstTagOccurrence, firstTagOccurrence - lengthDifference, lengthDifference + lengthRemaining + 1);
        }

        memmove(firstTagOccurrence, replacement, replacementStringLength);
        firstTagOccurrence = func_020d2f88(output, searchTag);
    }
    return output;
}*/