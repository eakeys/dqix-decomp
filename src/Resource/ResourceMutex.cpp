#include "Resource/ResourceMutex.h"

extern struct Struct_0214e4a8
{
    unsigned char unknown_0;
    bool lockPrerequisiteA_;
    bool lockPrerequisiteB_;
    char padding_3;
    unsigned int lockCount_;
    Mutex mutex_;
} data_0214e4a8;

extern "C"
{
    // set up the main context
    void func_020c745c();
    // check if the main context has been set up
    bool func_020c75a4();
}

void InitializeResourceMutex()
{
    if (!func_020c75a4())
        func_020c745c();

    ZeroInitializeMutex(&data_0214e4a8.mutex_);
    data_0214e4a8.lockPrerequisiteB_ = true;
    data_0214e4a8.unknown_0 = 1;
    data_0214e4a8.lockPrerequisiteA_ = true;
    data_0214e4a8.lockCount_ = 0;
}

bool SetResourceMutexOperational(bool to)
{
    bool oldStatus = data_0214e4a8.lockPrerequisiteA_;
    data_0214e4a8.lockPrerequisiteA_ = to;
    return oldStatus;
}

void LockResourceMutex()
{
    if (data_0214e4a8.lockPrerequisiteB_ && data_0214e4a8.lockPrerequisiteA_)
    {
        LockMutex(&data_0214e4a8.mutex_);
        data_0214e4a8.lockCount_++;
    }
}

void UnlockResourceMutex()
{
    if (data_0214e4a8.lockPrerequisiteB_ && data_0214e4a8.lockPrerequisiteA_)
    {
        data_0214e4a8.lockCount_--;
        UnlockMutex(&data_0214e4a8.mutex_);
    }
}

void SleepIfResourceMutexNotLocked(unsigned int milliseconds)
{
    if (data_0214e4a8.lockCount_ == 0)
        SleepCurrentContext(milliseconds);
}