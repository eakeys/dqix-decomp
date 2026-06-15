#include "Filesystem/ExtendedNitroVM.h"
#include "Filesystem/FSInnerDefs.h"
#include "System/Cache.h"
#include "std_library_functions.h"
#include <globaldefs.h>

//#pragma optimize_for_size off
#if defined(jpn)
#define func_020d84f8 func_020d9e5c
#define func_020d8524 func_020d9e88

#define func_020d970c func_020db118
#define func_020d974c func_020db158

#define func_020d9788 func_020db194

#define func_020ca95c func_020cc428

#define data_01ffd998 data_01ffd9b8
#define data_01ffd99c data_01ffd9bc
#define data_01ffda90 data_01ffdabc

#define data_020f2384 data_020f24d0
#define data_020f27b8 data_020f2974
#endif

#if defined(usa)
#define NUM_CACHED_FILES 61
#elif defined(jpn)
#define NUM_CACHED_FILES 64
#endif

extern "C"
{
    // Get CRC hash for a null terminated string
    unsigned int func_01ff860c(const char*);

    // Zero memory and flush cache
    void func_020d84f8(void*, unsigned);
    // another memcpy-style function, cleans/invalidates the cache in destination after
    unsigned int func_020d8524(void*, const void*, unsigned);

    void func_020d970c();
    void func_020d974c();

    void func_020d9788(int);

    void DecompressA  (Decompressor*, const void*, unsigned);
    void DecompressB  (Decompressor*, const void*, unsigned);
    void func_020ca95c(Decompressor*, const void*, unsigned);
}
// files to prepare accessors in cache for
extern const char* cachedFilePaths[];
// Seems to hold whether cached file accessors have been saved or not
extern bool data_01ffd998;
// CRC hashes for cached file accessors
extern unsigned int data_01ffd99c[NUM_CACHED_FILES];
// cached file accessors
extern NitroFileAccessor data_01ffda90[NUM_CACHED_FILES];
// holds the intended length of compression metadata (4 bytes)
// there are two copies of it, the first is used in USA version and the
// second in JPN version
extern unsigned int data_020f2384[];

// "data/"
extern char data_020f27b8[];

unsigned int CompressionPrefix::GetDecompressedLength() const
{
    return decompressedLength;
}

bool Decompressor::InitAndDecompress(void *out, unsigned int outCapacity, const void *in, unsigned int inLength)
{
    func_020d84f8(this, sizeof(Decompressor));
    if (out == NULL || in == NULL || inLength < 4)
        return false;
    
    CleanInvalidateCacheRange(in, inLength);
    unsigned int compressionType = ((const CompressionPrefix*)in)->compressionType;
    if (compressionType >= 5)
        return false;
    unsigned int decompLength = ((const CompressionPrefix*)in)->decompressedLength;
    if (outCapacity < ((decompLength + 7) & ~3))
        return false;

    this->writeOutputPtr = (unsigned char*)out;
    this->remainingOutputBytes = decompLength;
    this->probablyDecompressedSize = decompLength;
    this->compressionType = compressionType;
    unsigned int bytesLeftRoundedUp = (remainingOutputBytes + 3) & ~3;
    this->abstractOutputLocation = writeOutputPtr;
    this->abstractOutputLocation = (unsigned char*)this->abstractOutputLocation + bytesLeftRoundedUp;
    switch (compressionType)
    {
    case 0:
        break;
    case 1:
        unknown_11 = 3;
        break;
    case 2:
    case 3:
        decompressB_typeFlag_18 = 1 << compressionType;
        unknown_14 = -1;
        unknown_08 = &unknown_1C[0];
        break;
    case 4:
        break;
    }

    if (inLength > 4)
        ProcessBytes((const unsigned char*)in + 4, inLength - 4);
    
    return true;
}

bool Decompressor::ProcessBytes(const void* input, unsigned int inputLength)
{
    bool success = false;
    func_020d970c();
    unsigned char* writeStart = writeOutputPtr;
    if (writeOutputPtr != NULL && compressionType < 5 && input != NULL && inputLength != NULL)
    {
        switch (compressionType)
        {
        case 1:
            DecompressA(this, input, inputLength);
            break;
        case 2:
        case 3:
            DecompressB(this, input, inputLength);
            break;
        case 4:
            func_020ca95c(this, input, inputLength);
            break;
        case 0: // uncompressed
        default:
            if (inputLength >= remainingOutputBytes)
                inputLength = remainingOutputBytes;
            remainingOutputBytes -= func_020d8524(writeStart + probablyDecompressedSize - remainingOutputBytes, input, inputLength);
            break;
        }
        CleanInvalidateCacheRange(writeStart, writeOutputPtr - writeStart);
        success = true;
    }
    func_020d974c();
    return success;
}

void CacheMainFileAccessors()
{
    if (!data_01ffd998)
    {
        char fullFilePath[64];

        const char** pCurrentFilePath = cachedFilePaths;
        
        for (unsigned int i = 0; i < NUM_CACHED_FILES; )
        {
            strcpy(fullFilePath, data_020f27b8);
            strcat(fullFilePath, *pCurrentFilePath);
            unsigned int crc = 0;
            if (CreateFileAccessor(&data_01ffda90[i], fullFilePath))
                crc = func_01ff860c(fullFilePath);
            // silly goofy register hack (see RemoveFurigana)
            unsigned int iplusone = i + 1;
            data_01ffd99c[iplusone - 1] = crc;
            i++;            
            pCurrentFilePath++;
        }

        // bubble sort
        unsigned int passEnd = NUM_CACHED_FILES - 1;
        while (true)
        {
            bool changesMadeThisPass = false;
            NitroFileAccessor* pAccessor = data_01ffda90;
            unsigned int* pCRC = data_01ffd99c;
            for (unsigned int i = 0; i < passEnd; i++, pAccessor++, pCRC++)
            {
                NitroFileAccessor currentAccessor;
                ((NitroFileAccessor*)&currentAccessor)->handle = pAccessor[0].handle;
                ((NitroFileAccessor*)&currentAccessor)->fileID = pAccessor[0].fileID;
                NitroFileAccessor nextAccessor;
                ((NitroFileAccessor*)&nextAccessor)->handle = pAccessor[1].handle;
                ((NitroFileAccessor*)&nextAccessor)->fileID = pAccessor[1].fileID;
                unsigned int currentCRC = pCRC[0];
                
                unsigned int nextCRC = pCRC[1];
                if (currentCRC > nextCRC)
                {
                    pAccessor[0].handle = nextAccessor.handle;
                    pAccessor[0].fileID = nextAccessor.fileID;
                    pAccessor[1].handle = currentAccessor.handle;
                    pAccessor[1].fileID = currentAccessor.fileID;
                    
                    pCRC[0] = nextCRC;
                    pCRC[1] = currentCRC;
                    changesMadeThisPass = true;
                }
            }
            if (!changesMadeThisPass)
                break;
            passEnd--;
        }

        data_01ffd998 = true;
    }
}

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
            searchMax = NUM_CACHED_FILES - 1;
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
        if (cacheIndex < NUM_CACHED_FILES)
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

unsigned int ExtendedNitroVM::DecompressWithScratchSpace(Decompressor &decompressor, 
    unsigned int &outDecompressedLength, unsigned int &remainingInputBytes,
    unsigned int &readPosTracker, void *scratchSpace, unsigned int scratchSpaceCapacity)
{
    // 'easy case': scratch space is big enough to fit all output data and have
    // 256 bytes left. Presumably the extra 256 is to avoid any trouble with 
    // partial decompression overwriting temporary compressed data? 
    if (decompressor.remainingOutputBytes + 256 <= scratchSpaceCapacity)
    {
        void* loadLocation = (void*)(((int)scratchSpace + scratchSpaceCapacity - remainingInputBytes) & ~3);
        unsigned int successfulLoadSize = LoadToBuffer(loadLocation, remainingInputBytes);
        if (unknown_2 != 0)
            return 0;
        decompressor.ProcessBytes(loadLocation, successfulLoadSize);
        remainingInputBytes -= successfulLoadSize;
        readPosTracker += successfulLoadSize;
    }
    // this case seems to implicitly assume the scratch space is large enough
    // for the output to fit, just not with 256 bytes to spare. 
    else 
    {
        if (remainingInputBytes > 256)
        {
            unsigned int loadSize = (remainingInputBytes - 256 + 3) & ~3;
            void* loadLocation = (void*)(((int)scratchSpace + scratchSpaceCapacity - loadSize) & ~3);
            unsigned int successfulLoadSize = LoadToBuffer(loadLocation, loadSize);
            if (unknown_2 != 0)
                return 0;
            decompressor.ProcessBytes(loadLocation, successfulLoadSize);
            remainingInputBytes -= successfulLoadSize;
            readPosTracker += successfulLoadSize;
        }
        // if above assumption violated: stack buffer overrun!
        if (remainingInputBytes != 0)
        {
            char stackScratchSpace[256];
            unsigned int successfulLoadSize = LoadToBuffer(stackScratchSpace, remainingInputBytes);
            if (unknown_2 != 0)
                return 0;
            decompressor.ProcessBytes(stackScratchSpace, successfulLoadSize);
            remainingInputBytes -= successfulLoadSize;
            readPosTracker += successfulLoadSize;
        }
    }
    unsigned int scratchSpaceUsedAmount = (decompressor.probablyDecompressedSize + 4) & ~3;
    if (scratchSpaceUsedAmount >= scratchSpaceCapacity)
        scratchSpaceUsedAmount = scratchSpaceCapacity;
    // zero memory and flush
    func_020d84f8((unsigned char*)scratchSpace + decompressor.probablyDecompressedSize, 
        scratchSpaceUsedAmount - decompressor.probablyDecompressedSize);
    CleanInvalidateCacheRange(scratchSpace, scratchSpaceUsedAmount);
    outDecompressedLength = decompressor.probablyDecompressedSize;
    return readPosTracker; 
}

unsigned int ExtendedNitroVM::DecompressBytes(void *output,
    unsigned int &outDecompressedSize, unsigned int numBytesToRead, unsigned int outputCapacity)
{
    outDecompressedSize = 0;
    if (output == NULL)
        return 0;
    if (!CheckUnknownFlagBit4())
        return 0;

    unsigned int metadataLength = numBytesToRead;
    unsigned int stack_bytesRemaining = numBytesToRead;
    unsigned int readPos = 0;
    unsigned int metadata = 0;

    const unsigned int* pMaxLength = data_020f2384;
#if defined(jpn)
    pMaxLength++;
#endif
    if (metadataLength >= *pMaxLength)
        metadataLength = *pMaxLength;
    
    unsigned int totalReadAmount = LoadToBuffer(&metadata, metadataLength);

    if (unknown_2 != 0)
        return 0;

    Decompressor decompressor;  
    if (!decompressor.InitAndDecompress(output, outputCapacity, &metadata, totalReadAmount))
        return totalReadAmount;

    stack_bytesRemaining -= totalReadAmount;
    readPos += totalReadAmount;
    // readPos will be further incremented and the final value returned
    return DecompressWithScratchSpace(decompressor, outDecompressedSize,
        stack_bytesRemaining, readPos, output, outputCapacity);
}