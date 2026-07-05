#include "Filesystem/FileIOPorts.h"
#include "Filesystem/FSInnerDefs.h"

void SendTaskToReadContext(Struct_021118e0::PFNCartridgeRead task)
{
    Struct_021118e0* readManager = (Struct_021118e0*)&data_021118e0;
    ChangeContextPriority(&readManager->cartridgeReadContext, data_021118e0.contextPriority_108);
    readManager->pContext_104 = &readManager->cartridgeReadContext;
    readManager->cartridgeReadProc = task;
    readManager->flags_114 |= (1 << CARTRIDGE_READ_CONTEXT_FLAG_TASK_PENDING);
    MarkContextReadyAndSwitch(&readManager->cartridgeReadContext);
}