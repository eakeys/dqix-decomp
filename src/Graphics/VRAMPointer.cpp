#include "Graphics/VRAMPointer.h"
#include "NitroSDK/HWRegisters.h"
#include "NitroSDK/LoadToVRAM.h"

typedef void (*VRAMLoadProc)(const void*, unsigned int, unsigned int);

extern VRAMLoadProc data_020f01b0[];
extern VRAMLoadProc data_020f01f8[];
extern VRAMLoadProc data_020f0218[];

void SetBG3SubPriority(int to) { BG3CNTSUB = (BG3CNTSUB & ~3) | to; }
void SetBG2SubPriority(int to) { BG2CNTSUB = (BG2CNTSUB & ~3) | to; }
void SetBG1SubPriority(int to) { BG1CNTSUB = (BG1CNTSUB & ~3) | to; }
void SetBG0SubPriority(int to) { BG0CNTSUB = (BG0CNTSUB & ~3) | to; }
void SetBG3Priority(int to) { BG3CNT = (BG3CNT & ~3) | to; }
void SetBG2Priority(int to) { BG2CNT = (BG2CNT & ~3) | to; }
void SetBG1Priority(int to) { BG1CNT = (BG1CNT & ~3) | to; }
void SetBG0Priority(int to) { BG0CNT = (BG0CNT & ~3) | to; }

void SetBG3SubOffset(int x, int y) { BG3OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG2SubOffset(int x, int y) { BG2OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG1SubOffset(int x, int y) { BG1OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG0SubOffset(int x, int y) { BG0OFSSUB = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG3Offset(int x, int y) { BG3OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG2Offset(int x, int y) { BG2OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG1Offset(int x, int y) { BG1OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }
void SetBG0Offset(int x, int y) { BG0OFS = (x & 0x1ff) | ((y << 16) & 0x1ff0000); }