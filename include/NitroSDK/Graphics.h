#pragma once

#include "HWRegisters.h"
#include "Matrix.h"

#define BLEND_TARGET_BG0 1
#define BLEND_TARGET_BG1 2
#define BLEND_TARGET_BG2 4
#define BLEND_TARGET_BG3 8
#define BLEND_TARGET_OBJ 0x10
#define BLEND_TARGET_BACKDROP 0x20

#define DISPCNT_MASK_BG_MODE 7
// right shift by 24 for numerical value
#define DISPCNT_MASK_CHARACTER_BASE_64K 0x07000000
// right shift by 27 for numerical value
#define DISPCNT_MASK_SCREEN_BASE_64K 0x38000000
#define DISPCNT_ENABLE_BG_EXTENDED_PALETTE 0x40000000
#define DISPCNT_ENABLE_OBJ_EXTENDED_PALETTE 0x80000000

#define DISP3DCNT_USE_CLEAR_TEXTURES 0x4000

#define BGCNT_MASK_PRIORITY 3
// right shift by 2 for numerical value
#define BGCNT_MASK_CHARACTER_BASE_16K 0x3c
#define BGCNT_MASK_MOSAIC 0x40
#define BGCNT_MASK_PALETTE_256 0x80
// right shift by 8 for numerical value
#define BGCNT_MASK_SCREEN_BASE_2K 0x1f00
#define BGCNT_MASK_BG01_EXTENDED_PALETTE 0x2000
#define BGCNT_MASK_SCREEN_SIZE 0xc000

// returns 0 for touch screen, 1 for top screen
inline int GetCurrentMainScreen()
{
    return (POWCNT & 0x8000) >> 15;
}

static inline void GXFifoWriteVertex16(fix16_t x, fix16_t y, fix16_t z)
{
    GXFIFO_VERTEX_16 = (unsigned short)x | ((unsigned short)y << 16);
    GXFIFO_VERTEX_16 = (unsigned short)z;
}

static inline void GXFifoWriteVertex16Float(float x, float y, float z)
{
    GXFifoWriteVertex16(4096.0f * x, 4096.0f * y, 4096.0f * z);
}

static inline void GXFifoWriteTranslate(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_MATRIX_TRANSLATE = x;
    GXFIFO_MATRIX_TRANSLATE = y;
    GXFIFO_MATRIX_TRANSLATE = z;
}

static inline void GXFifoWriteTranslateFloat(float x, float y, float z)
{
    GXFifoWriteTranslate(MAKEFIX32(x), MAKEFIX32(y), MAKEFIX32(z));
}

static inline void GXFifoWriteScale(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_MATRIX_SCALE = x;
    GXFIFO_MATRIX_SCALE = y;
    GXFIFO_MATRIX_SCALE = z;
}

static inline void GXFifoWriteScaleFloat(float x, float y, float z)
{
    GXFifoWriteScale(4096.0f * x, 4096.0f * y, 4096.0f * z);
}