#include "Graphics/TextBoxGraphics.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include "NitroSDK/Filesystem/NarcHandle.h"
#include "NitroSDK/HWRegisters.h"

extern AllocatorUnion data_02114e20;

extern "C"
{
    void* func_02012d88(AllocatorUnion*, size_t);

    void func_02012da4(AllocatorUnion*, void*);
}

extern const fix32_t data_020e73fc[8][2];

extern char data_020ef7ac[];
extern char data_020ef7c0[];
extern char data_020ef7c4[];

static inline void WriteVertex16(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_VERTEX_16 = (unsigned short)x | ((unsigned short)y << 16);
    GXFIFO_VERTEX_16 = (unsigned short)z;
}

static inline void WriteTranslate(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_MATRIX_TRANSLATE = x;
    GXFIFO_MATRIX_TRANSLATE = y;
    GXFIFO_MATRIX_TRANSLATE = z;
}

static inline void WriteScale(fix32_t x, fix32_t y, fix32_t z)
{
    GXFIFO_MATRIX_SCALE = x;
    GXFIFO_MATRIX_SCALE = y;
    GXFIFO_MATRIX_SCALE = z;
}

void TextBoxGraphics::Reset()
{
    for (int i = 0; i < 8; i++)
        tiles_[i].Reset();
    activeBox_ = NULL;
    allocator_ = NULL;
}

void TextBoxGraphics::Destroy()
{
    SPRSprite* tile = tiles_;
    for (int i = 0; i < 8; tile++, i++)
        tile->Release();

    if (allocator_ != NULL)
    {
        SignedAllocatorHeader* signedAlloc = allocator_->GetSignedAllocator();
        allocator_->Destroy();
        func_02012da4(&data_02114e20, signedAlloc);
        func_02012da4(&data_02114e20, allocator_);
    }
    Reset();
}

void TextBoxGraphics::Create()
{
    if (allocator_ == NULL)
    {
        allocator_ = (SafeAllocator*)func_02012d88(&data_02114e20, sizeof(SafeAllocator));
        allocator_->ResetAllocatorPointer();
        char* buffer = (char*)func_02012d88(&data_02114e20, 0xf08);
        allocator_->CreateTypeA(buffer, 0xf08);
        allocator_->Reset();
    }
    else
        allocator_->Reset();

    BackgroundLoader::AddLockGlobal();
    NarcHandle narc;
    unsigned int fileLength;
    void* iconArchive = LoadFileIntoMemory(data_020ef7ac, data_0211e33c, &fileLength);
    if (iconArchive != NULL)
    {
        if (narc.Initialize(data_020ef7c0, (const unsigned char*)iconArchive))
        {
            LoadFromFile(allocator_, iconArchive);
            narc.Destroy();
        }
    }
    BackgroundLoader::RemoveLockGlobal();
}

void TextBoxGraphics::LoadFromFile(SafeAllocator* alloc, const void*)
{
    char innerFilename[40];
    SPRSprite* tile = tiles_;
    for (int i = 0; i < 8; tile++, i++)
    {
        sprintf(innerFilename, data_020ef7c4, i);
        const void* innerFile = GetFileFromNARCInMemory(innerFilename);
        tile->Reset();
        tile->LoadFromSPR(innerFile, alloc);
    }
}

void TextBoxGraphics::Update(int numTicks)
{
    if (numTicks == 0)
        numTicks = 1;
    if (activeBox_ == NULL)
        return;

    switch (activeBox_->status)
    {
    case 0:
        break;
    case 1:
        activeBox_->status = 2;
        activeBox_->scaleY = 1 << 12;
        break;
    case 2:
        break;
    case 3:
        activeBox_->scaleY -= (int)(0.1f * 4096) * numTicks;
        if (activeBox_->scaleY <= 0)
        {
            activeBox_->status = 0;
            activeBox_->scaleY = 0;
        }
        break;
    }
}

void TextBoxGraphics::Draw(CBool drawBorder, int unused)
{
    if (activeBox_ == NULL || activeBox_->status == 0)
        return;

    Draw(activeBox_->x << 12, activeBox_->y << 12, activeBox_->width * activeBox_->scaleX,
        activeBox_->height * activeBox_->scaleY, drawBorder, unused);
}

void TextBoxGraphics::Draw(fix32_t x, fix32_t y, fix32_t w, fix32_t h, CBool drawBorder, int unused)
{
    if (w < (8 << 12) || h < (8 << 12))
        return;

    // color 0 transparent, repeat texture in both directions
    GXFIFO_TEXIMAGE_PARAMS = 0x20030000;
    // show both faces, alpha = 18 (0x12), id 61 (0x3b)
    GXFIFO_POLYGON_ATTRIBUTES = 0x3b1200c0;
    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_MATRIX_TRANSLATE = x + (3 << 12);
    GXFIFO_MATRIX_TRANSLATE = y + (3 << 12);
    GXFIFO_MATRIX_TRANSLATE = 1024 << 12;
    GXFIFO_MATRIX_SCALE = w - (6 << 12);
    GXFIFO_MATRIX_SCALE = h - (6 << 12);
    GXFIFO_MATRIX_SCALE = 1 << 12;
    GXFIFO_POLYGON_BEGIN = 1;
    GXFIFO_VERTEX_COLOR = 0;
    WriteVertex16(0, 0, 0);
    WriteVertex16(1 << 12, 0, 0);
    WriteVertex16(1 << 12, 1 << 12, 0);
    WriteVertex16(0, 1 << 12, 0);
    GXFIFO_POLYGON_END = 0;
    GXFIFO_MATRIX_POP = 1;

    if (drawBorder)
    {
        fix32_t xEdgeScale = (w - (8 << 12)) / 6.0f;
        fix32_t yEdgeScale = (h - (8 << 12)) / 6.0f;

        fix32_t floorXEdgeScale = xEdgeScale & ~0xfff;
        fix32_t floorYEdgeScale = yEdgeScale & ~0xfff;

        fix32_t positions[8][2] =
        {
            { x - (1 << 12),                   y - (1 << 12) },
            { x + (3 << 12) - floorXEdgeScale, y - (1 << 12) },
            { x + w - (7 << 12),               y - (1 << 12) },
            { x + w - (7 << 12),               y + (3 << 12) - floorYEdgeScale },
            { x + w - (7 << 12),               y + h - (7 << 12) },
            { x + (3 << 12) - floorXEdgeScale, y + h - (7 << 12) },
            { x - (1 << 12),                   y + h - (7 << 12) },
            { x - (1 << 12),                   y + (3 << 12) - floorYEdgeScale }
        };

        fix32_t dimensions[8][2];
        COPY_ARRAY(dimensions, data_020e73fc);
        dimensions[1][0] = xEdgeScale;
        dimensions[5][0] = xEdgeScale;
        dimensions[3][1] = yEdgeScale;
        dimensions[7][1] = yEdgeScale;

        SPRSprite* tile = tiles_;
        for (int i = 0; i < 8; tile++, i++)
        {
            int* pos = positions[i];
            int* dim = dimensions[i];
            GXFIFO_MATRIX_PUSH = 0;
            WriteTranslate(pos[0], pos[1], 1024 << 12);
            WriteScale(dim[0], dim[1], 1 << 12);
            tile->tint_ = activeBox_->color;
            tile->DrawTextureScaleTranslate(0, true);
            GXFIFO_MATRIX_POP = 1;
        }
    }   
}

void TextBoxGraphics::BeginDisplay()
{
    if (activeBox_ == NULL)
        return;
    activeBox_->scaleX = 1 << 12;
    activeBox_->scaleY = 0;
    activeBox_->status = 1;
}

void TextBoxGraphics::EndDisplay()
{
    if (activeBox_ == NULL)
        return;
    activeBox_->scaleX = 1 << 12;
    activeBox_->scaleY = 1 << 12;
    activeBox_->status = 3;
}