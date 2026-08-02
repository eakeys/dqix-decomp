#include "Graphics/GeometryFifo.h"
#include "System/Graphics.h"
#include "System/DMA.h"
#include <globaldefs.h>

//#pragma optimize_for_size off
#pragma dont_inline on

extern "C"
{
    void func_020ca0a8(int dmaChannel, const void* source, unsigned int length,
        DMACompletionCallback onCompletion, int callbackUserdata);
    void func_020ca2ac(int dmaChannel, const void* source, unsigned int length,
        DMACompletionCallback onCompletion, int callbackUserdata);

    // inverse memcpy doing 32 bytes at a time, assumes 4 byte alignment
    void func_020ca4b4(const void*, void*, unsigned);
    // repeated u32 memwrite, shifts source ptr along but keeps writing
    // to the same destination
    void func_020ca430(const void* src, volatile void* dst, unsigned int byteCount);

}

void WaitForFifoProcessingDone();

struct QueuedData
{
    uint32_t amountQueued;
    uint32_t data[0xc0];
};

struct Struct_0210cf78
{
    // points to an array in which [0] = amount of data
    // and [1], ..., [pQueuedData[0]] is the queued data
    QueuedData* pQueuedData;
    volatile int fifoProcessingFlag; // 1 = processing, 0 = done
    int unknown_8;
} extern data_0210cf78;

extern "C" void SendQueuedDataToGeometryFifo()
{
    if (data_0210cf78.fifoProcessingFlag)
        WaitForFifoProcessingDone();

    if (data_0210cf78.pQueuedData != NULL && data_0210cf78.pQueuedData->amountQueued != 0)
    {
        func_020ca430(data_0210cf78.pQueuedData->data, &GXFIFO, data_0210cf78.pQueuedData->amountQueued * 4);
        data_0210cf78.pQueuedData->amountQueued = 0;
    }
}

// can be made static. 020b69f4
void WaitForFifoProcessingDone()
{
    while (data_0210cf78.fifoProcessingFlag) {}
}

// can be made static. 020b6a0c
void OnGeometryFifoProcessingDone(int userdata)
{
    volatile int* pFlag = reinterpret_cast<volatile int*>(userdata);
    *pFlag = 0;
}

extern "C" void SendRawDataToGeometryFifo(const uint32_t* data, unsigned int numBytes)
{
    if (numBytes < 256 || data_020f2270 == -1)
        SubmitCommandToGeometryFifo(data[0], &data[1], (numBytes / 4) - 1);
    else
    {
        SendQueuedDataToGeometryFifo();
        data_0210cf78.fifoProcessingFlag = 1;
        if (data_0210cf78.unknown_8)
            func_020ca2ac(data_020f2270, data, numBytes, &OnGeometryFifoProcessingDone,
                reinterpret_cast<int>(&data_0210cf78.fifoProcessingFlag));
        else
            func_020ca0a8(data_020f2270, data, numBytes, &OnGeometryFifoProcessingDone,
                reinterpret_cast<int>(&data_0210cf78.fifoProcessingFlag));
    }
}

extern "C" void SubmitCommandToGeometryFifo(int command, const uint32_t* params, unsigned int numParams)
{
    if (data_0210cf78.pQueuedData != NULL)
    {
        if (data_0210cf78.fifoProcessingFlag)
        {
            uint32_t priorSize = data_0210cf78.pQueuedData->amountQueued;
            if (priorSize + 1 + numParams <= 0xc0)
            {
                data_0210cf78.pQueuedData->amountQueued = priorSize + 1;
                data_0210cf78.pQueuedData->data[priorSize] = command;
                if (numParams != 0)
                {
                    func_020ca4b4(params,
                        &data_0210cf78.pQueuedData->data[data_0210cf78.pQueuedData->amountQueued],
                        numParams * 4);
                    data_0210cf78.pQueuedData->amountQueued += numParams;
                }
                return;
            }
        }

        if (data_0210cf78.pQueuedData->amountQueued != 0)
            SendQueuedDataToGeometryFifo();
        else
        {
            if (data_0210cf78.fifoProcessingFlag)
                WaitForFifoProcessingDone();
        }
    }
    else
    {
        if (data_0210cf78.fifoProcessingFlag)
            WaitForFifoProcessingDone();
    }

    GXFIFO = command;
    func_020ca430(params, &GXFIFO, numParams * 4);
}