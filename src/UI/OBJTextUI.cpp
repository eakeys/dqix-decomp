#include "UI/OBJTextUI.h"
#include "NitroSDK/Cache.h"
#include "NitroSDK/LoadToVRAM.h"
#include "Resource/GameResources.h"
#include "GameState/GameState.h"
#include "Graphics/VRAMStaging.h"
#include "NitroSDK/BGBases.h"
#include "NitroSDK/OAM.h"
#include "Input/Input.h"
#include "Resource/GameStrings.h"

extern const unsigned char data_020ee94c[];
extern const VRAMSubregion data_020ee950[];

extern VRAMLoadProc data_020f2bb0[];

extern "C"
{
    GameResources* func_0200fb8c(GameState*);

    void* func_0203bd08();
    OAMEntryEXT* func_0203be40(void*); // main oam staging buffer
    OAMEntryEXT* func_0203be4c(void*); // sub oam staging buffer

    // log_2(obj mapping boundary) by engine
    int func_0209378c(int);
    // obj vram size by engine
    int func_020937f0(int);

}

unsigned int OBJTextUI::OBJGraphics::GetPixelDataSize() const
{
    if (unknown_8 != NULL)
        return (unknown_8[0].width_a * unknown_8[0].height_b) >> 1;
    else
        return 0;
}

void OBJTextUI::BoundingRect::Set(short l, short t, short r, short b)
{
    left = l;
    top = t;
    right = r;
    bottom = b;
}

unsigned char* OBJTextUI::CharBufferDesc::GetBase() const
{
    unsigned int addr = 0;
    switch (layer)
    {
    case 0: break;
    case 1: 
        addr = GetMainBG0CharacterBase();
        break;
    case 2: 
        addr = GetMainBG1CharacterBase();
        break;
    case 3: 
        addr = GetMainBG2CharacterBase();
        break;
    case 4: 
        addr = GetMainBG3CharacterBase();
        break;
    case 5: 
        addr = 0x06400000; // main OBJ char data
        break;
    case 6: 
        addr = GetSubBG0CharacterBase();
        break;
    case 7: 
        addr = GetSubBG1CharacterBase();
        break;
    case 8: 
        addr = GetSubBG2CharacterBase();
        break;
    case 9: 
        addr = GetSubBG3CharacterBase();
        break;
    case 10:
        addr = 0x06600000; // sub obj char data
        break;
    }

    return (unsigned char*)addr;
}

void OBJTextUI::CharBufferDesc::Reset()
{
    offset = 0;
    length = 0;
    layer = 0;
}

void OBJTextUI::CharacterBuffers::SetBufferA(
    unsigned int offset, unsigned int length, unsigned char layer)
{
    bufferA.offset = offset;
    bufferA.length = length;
    bufferA.layer = layer;
}

void OBJTextUI::CharacterBuffers::SetBufferB(
    unsigned int offset, unsigned int length, unsigned char layer)
{
    bufferB.offset = offset;
    bufferB.length = length;
    bufferB.layer = layer;
}

void OBJTextUI::CharacterBuffers::LoadAToB()
{
    unsigned char* srcBase = bufferA.GetBase();
    if (srcBase != NULL)
    {
        unsigned int srcOffset = bufferA.offset;
        VRAMLoadProc loadProc = data_020f2bb0[bufferB.layer];
        if (loadProc != NULL)
        {
            CleanInvalidateCacheRange(srcBase + srcOffset, bufferA.length);
            loadProc(srcBase + srcOffset, bufferB.offset, bufferA.length);
            CleanCacheRange(srcBase + srcOffset, bufferA.length);
        }
    }
}

void OBJTextUI::CharacterBuffers::LoadBToA()
{
    unsigned char* srcBase = bufferB.GetBase();
    if (srcBase != NULL)
    {
        unsigned int srcOffset = bufferB.offset;
        VRAMLoadProc loadProc = data_020f2bb0[bufferA.layer];
        if (loadProc != NULL)
        {
            CleanInvalidateCacheRange(srcBase + srcOffset, bufferA.length);
            loadProc(srcBase + srcOffset, bufferA.offset, bufferB.length);
            CleanCacheRange(srcBase + srcOffset, bufferA.length);
        }
    }
}

struct SimpleOBJShape {
    unsigned char width;
    unsigned char height;
    unsigned int unknown_4;
} extern const data_020ee958[];

unsigned int SubdivideSpace(OBJTextUI::RegionStructC* regions, unsigned int capacity,
    unsigned int left, unsigned int top, unsigned int width, unsigned int height)
{
    if (capacity == 0)
        return 0;

    unsigned int numPieces = 0;
    unsigned int completedHeight = 0;
    while (completedHeight < height)
    {
        // Try to fit in a max-width piece
        const SimpleOBJShape* simpleShape = NULL;
        const SimpleOBJShape* loopShape = data_020ee958;
        for (unsigned int i = 0; i < 12; i++, loopShape++)
        {
            if (loopShape->width == width && loopShape->height <= height - completedHeight)
            {
                simpleShape = loopShape;
                break;
            }
        }

        if (simpleShape != NULL)
        {
            regions[numPieces].originX_8 = left;
            regions[numPieces].originY_9 = completedHeight + top;
            regions[numPieces].width_a = simpleShape->width;
            regions[numPieces].height_b = simpleShape->height;
            regions[numPieces].unknown_0 = simpleShape->unknown_4;
            numPieces++;
            completedHeight += simpleShape->height;
        }
        else
        {
            unsigned int halfWidth = width / 2;
            unsigned int remainingHeight = height - completedHeight;
            // bug?: shouldn't it be (left, top + completedHeight)?
            numPieces += SubdivideSpace(&regions[numPieces], capacity - numPieces,
                left, completedHeight, halfWidth, remainingHeight);
            numPieces += SubdivideSpace(&regions[numPieces], capacity - numPieces,
                left + halfWidth, completedHeight, halfWidth, remainingHeight);
            break;
        }

        if (numPieces >= capacity)
            break;
    }
    return numPieces;
}

void OBJTextUI::RegionStructC::Reset()
{
    unknown_0 = 0;
    offset_4 = 0;
    originX_8 = originY_9 = 0;
    width_a = height_b = 0;
}

void OBJTextUI::OBJGraphics::Reset()
{
    blitRegions_0 = NULL;
    blitter_4 = NULL;
    unknown_8 = NULL;
    character_c.bufferA.Reset();
    character_c.bufferB.Reset();
    substruct_24.Reset();
    objVramBase_30 = 0;
    unknown_34 = 0;
    x_38 = y_39 = 0;
    width_3a = height_3b = 0;
    numRegions_3c = 0;
    firstOAMEntry_3d = 0;
    engine_3e = 0;
    unknown_3f_0 = false;
    unknown_3f_1 = false;
}

void OBJTextUI::Initialize(int screen, int windowID)
{
    buffer_0_ = NULL;
    unknown_4_ = NULL;
    unknown_8_ = 0;
    for (int i = 0; i < 3; i++)
        textRects_a_[i].Set(0,0,0,0);
    colors_22_[0] = 0x7fff;
    colors_22_[1] = 0xf0a; // green
    clipWindow_28_.Initialize(screen, windowID);
}

void OBJTextUI::LoadColorToPalette(int palettePosition, int index)
{
    if (index < 0 || index >= 2)
        return;
    
    unsigned int loadOffset = 2 * palettePosition;
    const unsigned short* pColor = &colors_22_[index];
    CleanInvalidateCacheRange(pColor, 2);
    if (unknown_4_->engine_3e == 0)
        LoadToMainObjStandardPalette(pColor, loadOffset + 0x1c0, 2);
    else
        LoadToSubObjStandardPalette(pColor, loadOffset + 0x1c0, 2);
    CleanCacheRange(pColor, 2);
}

void OBJTextUI::SetColor(int index, unsigned short value)
{
    if (index >= 0 && index < 2)
        colors_22_[index] = value;
}

void OBJTextUI::ReloadDefaultColors()
{
    if (unknown_4_ == NULL)
        return;
    unsigned short* defaultData = func_0200fb8c(GameState::GetInstance())->paletteData_2c;
    if (defaultData == NULL)
        return;
    unsigned short* loadSource = defaultData + 6;
    CleanInvalidateCacheRange(loadSource, 0x20);
    if (unknown_4_->engine_3e == 0)
        LoadToMainObjStandardPalette(loadSource, 0x1c0, 0x20);
    else
        LoadToSubObjStandardPalette(loadSource, 0x1c0, 0x20);
    CleanCacheRange(loadSource, 0x20);
    LoadColorToPalette(9, 0);
    LoadColorToPalette(11, 0);
}

void WriteOAMData(OAMEntryEXT* oam, int x, int y, int priority, int objMode,
    bool mosaic, int transformationMode, int shapeSizeParam, int colorDepthMode,
    int tileNumber, int paletteNumber, int rotScaleParam);

void OBJTextUI::SubmitToOAM()
{
    OBJGraphics* graphics;

    if (unknown_4_ == NULL)
        return;
    if (!unknown_4_->unknown_3f_0 || unknown_4_->unknown_8 == NULL)
        return;

    RegionStructC* regions = unknown_4_->unknown_8;

    void* oamManager = func_0203bd08();
    OAMEntryEXT* oamEntries;
    (void)func_0203be40(oamManager);    
    int engine = unknown_4_->engine_3e;
    int baseX = unknown_4_->x_38;
    int baseY = unknown_4_->y_39;
    
    (void)func_020937f0(engine);
    int oamBoundaryShift = func_0209378c(engine);
    
    oamEntries = func_0203be40(oamManager);
    if (engine != 0)
        oamEntries = func_0203be4c(oamManager);

    oamEntries += unknown_4_->firstOAMEntry_3d;
    unsigned int objVramBase = unknown_4_->objVramBase_30;
    int numRegions = unknown_4_->numRegions_3c;
    for (int i = 0; i < numRegions; i++)
    {
        RegionStructC* region = &regions[i];
        unsigned int tileNumber = (objVramBase + region->offset_4);
        tileNumber >>= oamBoundaryShift;
        WriteOAMData(oamEntries, baseX + region->originX_8, baseY + region->originY_9,
            0, 0, false, 0, region->unknown_0, 0, tileNumber, 14, 0);
        oamEntries++;
    }

    graphics = unknown_4_;
    unsigned int backgroundTileNumber = (objVramBase + graphics->unknown_34);
    backgroundTileNumber >>= oamBoundaryShift;
    int x = 0, y = 0;
    int xMax = graphics->width_3a - 4;
    int yMax = graphics->height_3b - 4;
    for (; y < yMax; y += graphics->substruct_24.height_b)
    {
        if (yMax < y + graphics->substruct_24.height_b)
            y = yMax - graphics->substruct_24.height_b;
        for (; x < xMax; x += graphics->substruct_24.width_a)
        {
            if (xMax < x + graphics->substruct_24.width_a)
                x = xMax - graphics->substruct_24.width_a;
            WriteOAMData(oamEntries, baseX + x + 2, baseY + y + 2, 0, 1, false, 0,
                graphics->substruct_24.unknown_0, 0, backgroundTileNumber, 14, 0);
            oamEntries++;
        }
        x = 0;
    }
}

void WriteOAMData(OAMEntryEXT* oam, int x, int y, int priority, int objMode,
    bool mosaic, int transformationMode, int shapeSizeParam, int colorDepthMode,
    int tileNumber, int paletteNumber, int rotScaleParam)
{
    if (transformationMode == 0x100 || transformationMode == 0x300) // rotation/scaling present
    {
        if (objMode == 3) // bitmap
            oam->words01 = (y & 0xff) | (rotScaleParam << 25) | (objMode << 10) |
                (mosaic << 12) | shapeSizeParam | ((x & 0x1ff) << 16) | transformationMode;
        else
            oam->words01 = (y & 0xff) | ((rotScaleParam << 25) | (colorDepthMode << 13)) |
                (objMode << 10) | (mosaic << 12) | shapeSizeParam | ((x & 0x1ff) << 16) |
                transformationMode;
    }
    else
    {
        if (objMode == 3) // bitmap
            oam->words01 = (y & 0xff) | (objMode << 10) | (mosaic << 12) |
                shapeSizeParam | ((x & 0x1ff) << 16) | transformationMode;
        else
            oam->words01 = (y & 0xff) | (colorDepthMode << 13) | (objMode << 10) |
                (mosaic << 12) | shapeSizeParam | ((x & 0x1ff) << 16) | transformationMode;
    }
    oam->base.word2 = tileNumber | (priority << 10) | (paletteNumber << 12);
}

void OBJTextUI::ConfigureClipWindow()
{
    if (unknown_4_ == NULL)
        return;

    if (unknown_4_->unknown_3f_1)
    {
        clipWindow_28_.SetEnabled(false);
        unknown_4_->character_c.LoadBToA();
        unknown_4_->unknown_3f_1 = false;
    }
    
    if (unknown_4_->unknown_3f_0)
    {
        clipWindow_28_.SetScreen(unknown_4_->engine_3e);
        clipWindow_28_.SetBoundsXYWH(
            unknown_4_->x_38 + 2, unknown_4_->y_39 + 2,
            unknown_4_->width_3a - 4, unknown_4_->height_3b - 4
        );
        clipWindow_28_.Apply();
    }
}

static inline unsigned int AssignRegionOffsets(OBJTextUI* ui)
{
    unsigned int total = 0;
    unsigned char engine = ui->unknown_4_->engine_3e;
    int i;
    unsigned int numRegions = ui->unknown_4_->numRegions_3c;
    OBJTextUI::RegionStructC* regions = ui->unknown_4_->unknown_8;
    for (i = 0; i < numRegions; i++)
    {
        OBJTextUI::RegionStructC* region = &regions[i];
        region->offset_4 = total;
        int thisRegionNumBytes = region->width_a * region->height_b;
        unsigned int alignMask = 1 << func_0209378c(engine);
        if (alignMask != 0)
            alignMask--;
        total += ~alignMask & (alignMask + (thisRegionNumBytes >> 1));
    }
    return total;
}

OBJTextUI::OBJGraphics* OBJTextUI::GetGraphics()
{
    return unknown_4_;
}

unsigned int OBJTextUI::CreateInVRAM(unsigned int overrideObjVramSize)
{
    if (unknown_4_ == NULL)
        return 0;

    ReloadDefaultColors();
    RegionStructC* regions = unknown_4_->unknown_8;
    unsigned int height = unknown_4_->height_3b;
    unsigned int regionCapacity = unknown_4_->numRegions_3c;
    unsigned int width = unknown_4_->width_3a;

    int numRegions;
    if (regions == NULL || regionCapacity == 0 || width == 0 || height == 0)
        numRegions = 0;
    else
    {
        numRegions = 0;
        unsigned int pieceWidth;
        unsigned int completedWidth = 0;
        unsigned int alignedWidth = (width + 7) & ~7;
        unsigned int alignedHeight = (height + 7) & ~7;
        while (completedWidth < alignedWidth)
        {
            if (numRegions >= regionCapacity)
                break;
            pieceWidth = 64;
            unsigned int remainingWidth = alignedWidth - completedWidth;
            while (pieceWidth > 8 && pieceWidth > remainingWidth)
                pieceWidth >>= 1;
            numRegions += SubdivideSpace(regions + numRegions, regionCapacity - numRegions, 
                completedWidth, 0, pieceWidth, alignedHeight);
            completedWidth += pieceWidth;
        }
    }
    unknown_4_->numRegions_3c = numRegions;

    unknown_4_->unknown_34 = AssignRegionOffsets(this);

    unsigned int objVramSize = func_020937f0(unknown_4_->engine_3e);
    if (overrideObjVramSize != -1)
        objVramSize = overrideObjVramSize;
    unknown_4_->objVramBase_30 = objVramSize - (unknown_4_->unknown_34 + unknown_4_->GetPixelDataSize());
    unknown_4_->character_c.LoadAToB();
    return unknown_4_->objVramBase_30;
}

void OBJTextUI::ResetStateFlags()
{
    unknown_4_->unknown_3f_0 = false;
    unknown_4_->unknown_3f_1 = true;
}

bool OBJTextUI::CreateYesNoBox(unsigned int objVramSize)
{
    if (unknown_4_ == NULL)
        return true;
    unknown_4_->x_38 = 194;
    unknown_4_->y_39 = 72;
    unknown_4_->width_3a = 60;
    unknown_4_->height_3b = 44;

    const char* yesString = GetGameString(GameString_Yes);
    const char* noString = GetGameString(GameString_No);

    int textWidth = MeasureTextWidth(yesString, 1);
    int otherWidth = MeasureTextWidth(noString, 1);
    if (textWidth < otherWidth)
        textWidth = otherWidth;

    int trueWidth = textWidth + 24;
    unknown_4_->x_38 = 256 - (trueWidth + 2);
    unknown_4_->width_3a = trueWidth;
    CreateInVRAM(objVramSize);
    if (BeginRender() != NULL)
    {
        unknown_8_ = 0;
        DrawTextBoxBorder();
        DrawText(0, 12, 8, yesString, 9, 1);
        DrawText(1, 12, 24, noString, 11, 1);
        EndRender();
        FreeStagingMemory();
        return false;
    }
    return true;
}

void OBJTextUI::GetOptionPosition(short* outX, short* outY, int option)
{
    *outX = unknown_4_->x_38 + 12;
    *outY = unknown_4_->y_39 + 8 + 16 * option;
}

int OBJTextUI::GetTouchSelection()
{
    if (g_touchInput.pressed_)
    {
        int tx, ty;
        g_touchInput.GetTouchPoint(&tx, &ty);
        int choice = GetPointSelection(tx, ty);
        if (choice >= 0)
            return choice;
        else
            return -2;
    }
    return -1;
}

int OBJTextUI::GetPointSelection(short x, short y)
{
    for (int i = 0; i < unknown_8_; i++)
    {
        int left = unknown_4_->x_38 + textRects_a_[i].left;
        int top = unknown_4_->y_39 + textRects_a_[i].top;
        int right = unknown_4_->x_38 + textRects_a_[i].right;
        int bottom = unknown_4_->y_39 + textRects_a_[i].bottom;
        
        if (left <= x && x <= right && top <= y && y <= bottom)
            return i;
    }
    return -1;
}

void* OBJTextUI::BeginRender() 
{
    buffer_0_ = (unsigned char*)AllocateVRAMStagingMemory(0x4000);
    if (buffer_0_ != NULL)
    {
        OBJGraphics* sub4 = unknown_4_;
        memset(buffer_0_ + sub4->objVramBase_30, 0, sub4->unknown_34 + sub4->GetPixelDataSize());
        Clear();
        int numRegions = unknown_4_->numRegions_3c;
        for (int i = 0; i < numRegions; i++)
        {
            OBJBlitter::BlitRegion* blitRegion = &unknown_4_->blitRegions_0[i];
            RegionStructC* sourceData = &unknown_4_->unknown_8[i];
            blitRegion->Reset();
            blitRegion->offset = sourceData->offset_4;
            blitRegion->originX = sourceData->originX_8;
            blitRegion->originY = sourceData->originY_9;
            blitRegion->width = sourceData->width_a;
            blitRegion->height = sourceData->height_b;
        }

        OBJBlitter* blitter = unknown_4_->blitter_4;
        blitter->Reset();
        blitter->SetDestination(buffer_0_ + unknown_4_->objVramBase_30);
        blitter->SetRegions(unknown_4_->blitRegions_0, numRegions);
    }
    return buffer_0_;
}

void OBJTextUI::Clear()
{
    OBJGraphics* sub4 = unknown_4_;
    RegionStructC* regions = sub4->unknown_8;
    sub4->substruct_24.Reset();
    sub4->substruct_24.unknown_0 = regions[0].unknown_0;
    sub4->substruct_24.offset_4 = 0;
    sub4->substruct_24.originX_8 = 0;
    sub4->substruct_24.originY_9 = 0;
    sub4->substruct_24.width_a = regions[0].width_a - 4;
    sub4->substruct_24.height_b = regions[0].height_b - 4;

    int altWidth = unknown_4_->width_3a - 4;
    if (altWidth <= sub4->substruct_24.width_a)
        sub4->substruct_24.width_a = altWidth;
    int altHeight = unknown_4_->height_3b - 4;
    if (altHeight <= sub4->substruct_24.height_b)
        sub4->substruct_24.height_b = altHeight;
    
    OBJBlitter::BlitRegion tempRegion;
    tempRegion.Reset();
    tempRegion.width = regions[0].width_a;
    tempRegion.height = regions[0].height_b;
    OBJBlitter tempBlitter;
    tempBlitter.Reset();
    tempBlitter.SetDestination(&buffer_0_[unknown_4_->objVramBase_30 + unknown_4_->unknown_34]);
    tempBlitter.SetRegions(&tempRegion, 1);
    tempBlitter.FillRectangle(0, 0, sub4->substruct_24.width_a, sub4->substruct_24.height_b, 1);
}

void OBJTextUI::DrawTextBoxBorder()
{
    OBJBlitter* blitter = unknown_4_->blitter_4;
    unsigned char width = unknown_4_->width_3a;
    unsigned char height = unknown_4_->height_3b;

    const void* topTile = GetWindowBorderTile(1);
    const void* bottomTile = GetWindowBorderTile(6);
    if (topTile != NULL && bottomTile != NULL)
    {
        int numTiles = (width - 9) >> 3;
        for (int x = 8, i = 0; i < numTiles; x += 8, i++)
        {
            blitter->BlitTile8x8(topTile, x, 0, 8, 15);
            blitter->BlitTile8x8(bottomTile, x, height - 8, 8, 15);
        }
    }

    const void* leftTile = GetWindowBorderTile(3);
    const void* rightTile = GetWindowBorderTile(4);
    if (leftTile != NULL && rightTile != NULL)
    {
        int numTiles = (height - 9) >> 3;
        for (int y = 8, i = 0; i < numTiles; y += 8, i++)
        {
            blitter->BlitTile8x8(leftTile, 0, y, 8, 15);
            blitter->BlitTile8x8(rightTile, width - 8, y, 8, 15);
        }
    }

    short cornerX[] = { 0, width - 8, 0, width - 8 };
    short cornerY[] = { 0, 0, height - 8, height - 8 };

    for (int i = 0; i < 4; i++)
    {
        const void* cornerTile = GetWindowBorderTile(data_020ee94c[i]);
        if (cornerTile != NULL)
            blitter->BlitTile8x8(cornerTile, cornerX[i], cornerY[i], 8, 15);
    }
}

void OBJTextUI::DrawText(int textID, int x, int y,
    const char* text, unsigned char colorIndex, unsigned char fontIndex)
{
    if (textID >= 0 && textID < 3)
    {
        int width = MeasureTextWidth(text, fontIndex);
        textRects_a_[textID].Set(x, y, x + width, y + 12);
    }
    if (unknown_8_ <= textID)
        unknown_8_ = textID + 1;
    unknown_4_->blitter_4->BlitText(x, y, text, colorIndex, fontIndex);
}

void OBJTextUI::FillRectangle(unsigned char colorIndex, short left, short top, short right, short bottom)
{
    unknown_4_->blitter_4->FillRectangle(left, top, right, bottom, colorIndex);
}

void OBJTextUI::EndRender()
{
    if (unknown_4_ == NULL || buffer_0_ == NULL)
        return;

    unknown_4_->unknown_3f_0 = true;
    StageMemoryToVRAM(data_020ee950[unknown_4_->engine_3e], 
        buffer_0_ + unknown_4_->objVramBase_30,
        unknown_4_->objVramBase_30, 
        unknown_4_->unknown_34 + unknown_4_->GetPixelDataSize(),
        true, false);
}

void OBJTextUI::FreeStagingMemory()
{
    if (buffer_0_ != NULL)
        FreeVRAMStagingMemory(buffer_0_);
}