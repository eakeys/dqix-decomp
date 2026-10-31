#include "Graphics/TextBoxGraphics.h"
#include "Filesystem/BackgroundLoader.h"
#include "Filesystem/FileIO.h"
#include "NitroSDK/Filesystem/NarcHandle.h"
#include "NitroSDK/Graphics.h"

#if defined(jpn)
#define data_02114e20 data_02114ac0

#define func_02012d88 func_02012b50
#define func_02012da4 func_02012b6c
#endif

extern AllocatorUnion data_02114e20;

extern "C"
{
    void* func_02012d88(AllocatorUnion*, size_t);
    void func_02012da4(AllocatorUnion*, void*);
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
    void* iconArchive = LoadFileIntoMemory("data/bin/icon.nsarc", data_0211e33c, &fileLength);
    if (iconArchive != NULL)
    {
        if (narc.Initialize("ARC", (const unsigned char*)iconArchive))
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
        sprintf(innerFilename, "ARC:win%d.spr", i);
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
        activeBox_->scaleY = MAKEFIX32(1);
        break;
    case 2:
        break;
    case 3:
        activeBox_->scaleY -= MAKEFIX32(0.1f) * numTicks;
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
    if (w < MAKEFIX32(8) || h < MAKEFIX32(8))
        return;

    // color 0 transparent, repeat texture in both directions
    GXFIFO_TEXIMAGE_PARAMS = 0x20030000;
    // show both faces, alpha = 18 (0x12), id 61 (0x3b)
    GXFIFO_POLYGON_ATTRIBUTES = 0x3b1200c0;
    GXFIFO_MATRIX_PUSH = 0;
    GXFifoWriteTranslate(x + MAKEFIX32(3), y + MAKEFIX32(3), MAKEFIX32(1024));
    GXFifoWriteScale(w - MAKEFIX32(6), h - MAKEFIX32(6), MAKEFIX32(1));
    GXFIFO_POLYGON_BEGIN = 1;
    GXFIFO_VERTEX_COLOR = 0;
    GXFifoWriteVertex16Float(0.0f, 0.0f, 0.0f);
    GXFifoWriteVertex16Float(1.0f, 0.0f, 0.0f);
    GXFifoWriteVertex16Float(1.0f, 1.0f, 0.0f);
    GXFifoWriteVertex16Float(0.0f, 1.0f, 0.0f);
    GXFIFO_POLYGON_END = 0;
    GXFIFO_MATRIX_POP = 1;

    if (drawBorder)
    {
        fix32_t xEdgeScale = (w - MAKEFIX32(8)) / 6.0f;
        fix32_t yEdgeScale = (h - MAKEFIX32(8)) / 6.0f;

        fix32_t floorXScale = xEdgeScale & ~0xfff;
        fix32_t floorYScale = yEdgeScale & ~0xfff;

        fix32_t positions[8][2] =
        {
            { x - MAKEFIX32(1),               y - MAKEFIX32(1) },
            { x + MAKEFIX32(3) - floorXScale, y - MAKEFIX32(1) },
            { x + w - MAKEFIX32(7),           y - MAKEFIX32(1) },
            { x + w - MAKEFIX32(7),           y + MAKEFIX32(3) - floorYScale },
            { x + w - MAKEFIX32(7),           y + h - MAKEFIX32(7) },
            { x + MAKEFIX32(3) - floorXScale, y + h - MAKEFIX32(7) },
            { x - MAKEFIX32(1),               y + h - MAKEFIX32(7) },
            { x - MAKEFIX32(1),               y + MAKEFIX32(3) - floorYScale }
        };

        fix32_t dimensions[8][2] = 
        {
            { MAKEFIX32(1), MAKEFIX32(1) },
            { MAKEFIX32(0), MAKEFIX32(1) },
            { MAKEFIX32(1), MAKEFIX32(1) },
            { MAKEFIX32(1), MAKEFIX32(0) },
            { MAKEFIX32(1), MAKEFIX32(1) },
            { MAKEFIX32(0), MAKEFIX32(1) },
            { MAKEFIX32(1), MAKEFIX32(1) },
            { MAKEFIX32(1), MAKEFIX32(0) }
        };
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
            GXFifoWriteTranslate(pos[0], pos[1], MAKEFIX32(1024));
            GXFifoWriteScale(dim[0], dim[1], MAKEFIX32(1));
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
    activeBox_->scaleX = MAKEFIX32(1);
    activeBox_->scaleY = MAKEFIX32(0);
    activeBox_->status = 1;
}

void TextBoxGraphics::EndDisplay()
{
    if (activeBox_ == NULL)
        return;
    activeBox_->scaleX = MAKEFIX32(1);
    activeBox_->scaleY = MAKEFIX32(1);
    activeBox_->status = 3;
}