#include "UI/OBJTextUI.h"
#include "UI/Text.h"
#include "NitroSDK/Cache.h"
#include "NitroSDK/LoadToVRAM.h"
#include "NitroSDK/BGBases.h"
#include "NitroSDK/OAM.h"
#include "Resource/GameResources.h"
#include "GameState/GameState.h"
#include "Graphics/VRAMStaging.h"
#include "Input/Input.h"
#include "Resource/GameStrings.h"

#ifdef jpn
#define data_020ee94c data_020eea10
#define data_020ee950 data_020eea14
#define data_020ee958 data_020eea1c

#define data_020f2bb0 data_020f2d00

#define func_0200fb8c func_0200f9e8
#define func_0203bd08 func_0203b760
#define func_0203be40 func_0203b898
#define func_0203be4c func_0203b8a4
#define func_0209378c func_020940c0
#define func_020937f0 func_02094124
#endif

extern const unsigned char data_020ee94c[]; // corner tile lookup
extern const VRAMSubregion data_020ee950[]; // { main obj, sub obj }

#ifdef jpn
extern char data_020f2d2c[]; // japanese yes
extern char data_020f2d31[]; // japanese no
#endif

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
    if (regions != NULL)
        return (regions[0].width * regions[0].height) >> 1;
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

unsigned int SubdivideSpace(OBJTextUI::Region* regions, unsigned int capacity,
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
            regions[numPieces].x = left;
            regions[numPieces].y = completedHeight + top;
            regions[numPieces].width = simpleShape->width;
            regions[numPieces].height = simpleShape->height;
            regions[numPieces].shapeSize = simpleShape->unknown_4;
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

void OBJTextUI::Region::Reset()
{
    shapeSize = 0;
    relVramOffset = 0;
    x = y = 0;
    width = height = 0;
}

void OBJTextUI::OBJGraphics::Reset()
{
    blitRegions = NULL;
    blitter = NULL;
    regions = NULL;
    charBuffers.bufferA.Reset();
    charBuffers.bufferB.Reset();
    backgroundRegion.Reset();
    vramOffset = 0;
    charDataSize = 0;
    baseX = baseY = 0;
    width = height = 0;
    numRegions = 0;
    oamIndexBase = 0;
    engine = 0;
    readyToSubmit = false;
    pendingClose = false;
}

void OBJTextUI::Initialize(int screen, int windowID)
{
    stagingBuffer_ = NULL;
    graphics_ = NULL;
    numOptions_ = 0;
    for (int i = 0; i < 3; i++)
        optionBounds_[i].Set(0,0,0,0);
    textColors_[0] = 0x7fff;
    textColors_[1] = 0xf0a; // green
    clipWindow_.Initialize(screen, windowID);
}

void OBJTextUI::LoadColorToPalette(int palettePosition, int index)
{
    if (index < 0 || index >= 2)
        return;
    
    unsigned int loadOffset = 2 * palettePosition;
    const unsigned short* pColor = &textColors_[index];
    CleanInvalidateCacheRange(pColor, 2);
    if (graphics_->engine == 0)
        LoadToMainObjStandardPalette(pColor, loadOffset + 0x1c0, 2);
    else
        LoadToSubObjStandardPalette(pColor, loadOffset + 0x1c0, 2);
    CleanCacheRange(pColor, 2);
}

void OBJTextUI::SetColor(int index, unsigned short value)
{
    if (index >= 0 && index < 2)
        textColors_[index] = value;
}

void OBJTextUI::ReloadDefaultColors()
{
    if (graphics_ == NULL)
        return;
    unsigned short* defaultData = func_0200fb8c(GameState::GetInstance())->paletteData_2c;
    if (defaultData == NULL)
        return;
    unsigned short* loadSource = defaultData + 6;
    CleanInvalidateCacheRange(loadSource, 0x20);
    if (graphics_->engine == 0)
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

    if (graphics_ == NULL)
        return;
    if (!graphics_->readyToSubmit || graphics_->regions == NULL)
        return;

    Region* regions = graphics_->regions;

    void* oamManager = func_0203bd08();
    OAMEntryEXT* pOamEntries;
    (void)func_0203be40(oamManager);    
    int engine = graphics_->engine;
    int baseX = graphics_->baseX;
    int baseY = graphics_->baseY;
    
    (void)func_020937f0(engine);
    int oamBoundaryShift = func_0209378c(engine);
    
    pOamEntries = func_0203be40(oamManager);
    if (engine != 0)
        pOamEntries = func_0203be4c(oamManager);

    pOamEntries += graphics_->oamIndexBase;
    unsigned int objVramBase = graphics_->vramOffset;
    int numRegions = graphics_->numRegions;
    for (int i = 0; i < numRegions; i++)
    {
        Region* region = &regions[i];
        unsigned int tileNumber = (objVramBase + region->relVramOffset);
        tileNumber >>= oamBoundaryShift;
        WriteOAMData(pOamEntries, baseX + region->x, baseY + region->y,
            0, 0, false, 0, region->shapeSize, 0, tileNumber, 14, 0);
        pOamEntries++;
    }

    graphics = graphics_;
    unsigned int backgroundTileNumber = (objVramBase + graphics->charDataSize);
    backgroundTileNumber >>= oamBoundaryShift;
    int x = 0, y = 0;
    int xMax = graphics->width - 4;
    int yMax = graphics->height - 4;
    for (; y < yMax; y += graphics->backgroundRegion.height)
    {
        if (yMax < y + graphics->backgroundRegion.height)
            y = yMax - graphics->backgroundRegion.height;
        for (; x < xMax; x += graphics->backgroundRegion.width)
        {
            if (xMax < x + graphics->backgroundRegion.width)
                x = xMax - graphics->backgroundRegion.width;
            WriteOAMData(pOamEntries, baseX + x + 2, baseY + y + 2, 0, 1, false, 0,
                graphics->backgroundRegion.shapeSize, 0, backgroundTileNumber, 14, 0);
            pOamEntries++;
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
    if (graphics_ == NULL)
        return;

    if (graphics_->pendingClose)
    {
        clipWindow_.SetEnabled(false);
        graphics_->charBuffers.LoadBToA();
        graphics_->pendingClose = false;
    }
    
    if (graphics_->readyToSubmit)
    {
        clipWindow_.SetScreen(graphics_->engine);
        clipWindow_.SetBoundsXYWH(
            graphics_->baseX + 2, graphics_->baseY + 2,
            graphics_->width - 4, graphics_->height - 4
        );
        clipWindow_.Apply();
    }
}

static inline unsigned int AssignRegionOffsets(OBJTextUI* ui)
{
    unsigned int total = 0;
    unsigned char engine = ui->graphics_->engine;
    int i;
    unsigned int numRegions = ui->graphics_->numRegions;
    OBJTextUI::Region* regions = ui->graphics_->regions;
    for (i = 0; i < numRegions; i++)
    {
        OBJTextUI::Region* region = &regions[i];
        region->relVramOffset = total;
        int thisRegionNumBytes = region->width * region->height;
        unsigned int alignMask = 1 << func_0209378c(engine);
        if (alignMask != 0)
            alignMask--;
        total += ~alignMask & (alignMask + (thisRegionNumBytes >> 1));
    }
    return total;
}

OBJTextUI::OBJGraphics* OBJTextUI::GetGraphics()
{
    return graphics_;
}

unsigned int OBJTextUI::CreateInVRAM(unsigned int overrideObjVramSize)
{
    if (graphics_ == NULL)
        return 0;

    ReloadDefaultColors();
    Region* regions = graphics_->regions;
    unsigned int height = graphics_->height;
    unsigned int regionCapacity = graphics_->numRegions;
    unsigned int width = graphics_->width;

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
    graphics_->numRegions = numRegions;

    graphics_->charDataSize = AssignRegionOffsets(this);

    unsigned int objVramSize = func_020937f0(graphics_->engine);
    if (overrideObjVramSize != -1)
        objVramSize = overrideObjVramSize;
    graphics_->vramOffset = objVramSize - (graphics_->charDataSize + graphics_->GetPixelDataSize());
    graphics_->charBuffers.LoadAToB();
    return graphics_->vramOffset;
}

void OBJTextUI::RequestClose()
{
    graphics_->readyToSubmit = false;
    graphics_->pendingClose = true;
}

bool OBJTextUI::CreateYesNoBox(unsigned int objVramSize)
{
    if (graphics_ == NULL)
        return true;
    graphics_->baseX = 194;
    graphics_->baseY = 72;
    graphics_->width = 60;
    graphics_->height = 44;

#if defined(usa)
    const char* yesString = GetGameString(GameString_Yes);
    const char* noString = GetGameString(GameString_No);
    int textWidth = MeasureTextWidth(yesString, 1);
    int otherWidth = MeasureTextWidth(noString, 1);
    if (textWidth < otherWidth)
        textWidth = otherWidth;

    int trueWidth = textWidth + 24;
    graphics_->baseX = 256 - (trueWidth + 2);
    graphics_->width = trueWidth;
#endif
    CreateInVRAM(objVramSize);
    if (BeginRender() != NULL)
    {
        numOptions_ = 0;
        DrawTextBoxBorder();
#if defined(usa)
        DrawText(0, 12, 8, yesString, 9, 1);
        DrawText(1, 12, 24, noString, 11, 1);
#elif defined(jpn)
        DrawText(0, 12, 8, data_020f2d2c, 12, 9);
        DrawText(1, 12, 24, data_020f2d31, 12, 11);
#endif
        EndRender();
        FreeStagingMemory();
        return false;
    }
    return true;
}

void OBJTextUI::GetOptionPosition(short* outX, short* outY, int option)
{
    *outX = graphics_->baseX + 12;
    *outY = graphics_->baseY + 8 + 16 * option;
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
    for (int i = 0; i < numOptions_; i++)
    {
        int left = graphics_->baseX + optionBounds_[i].left;
        int top = graphics_->baseY + optionBounds_[i].top;
        int right = graphics_->baseX + optionBounds_[i].right;
        int bottom = graphics_->baseY + optionBounds_[i].bottom;
        
        if (left <= x && x <= right && top <= y && y <= bottom)
            return i;
    }
    return -1;
}

void* OBJTextUI::BeginRender() 
{
    stagingBuffer_ = (unsigned char*)AllocateVRAMStagingMemory(0x4000);
    if (stagingBuffer_ != NULL)
    {
        OBJGraphics* sub4 = graphics_;
        memset(stagingBuffer_ + sub4->vramOffset, 0, sub4->charDataSize + sub4->GetPixelDataSize());
        Clear();
        int numRegions = graphics_->numRegions;
        for (int i = 0; i < numRegions; i++)
        {
            OBJBlitter::BlitRegion* blitRegion = &graphics_->blitRegions[i];
            Region* sourceData = &graphics_->regions[i];
            blitRegion->Reset();
            blitRegion->offset = sourceData->relVramOffset;
            blitRegion->originX = sourceData->x;
            blitRegion->originY = sourceData->y;
            blitRegion->width = sourceData->width;
            blitRegion->height = sourceData->height;
        }

        OBJBlitter* blitter = graphics_->blitter;
        blitter->Reset();
        blitter->SetDestination(stagingBuffer_ + graphics_->vramOffset);
        blitter->SetRegions(graphics_->blitRegions, numRegions);
    }
    return stagingBuffer_;
}

void OBJTextUI::Clear()
{
    OBJGraphics* graphics = graphics_;
    Region* regions = graphics->regions;
    graphics->backgroundRegion.Reset();
    graphics->backgroundRegion.shapeSize = regions[0].shapeSize;
    graphics->backgroundRegion.relVramOffset = 0;
    graphics->backgroundRegion.x = 0;
    graphics->backgroundRegion.y = 0;
    graphics->backgroundRegion.width = regions[0].width - 4;
    graphics->backgroundRegion.height = regions[0].height - 4;

    int altWidth = graphics_->width - 4;
    if (altWidth <= graphics->backgroundRegion.width)
        graphics->backgroundRegion.width = altWidth;
    int altHeight = graphics_->height - 4;
    if (altHeight <= graphics->backgroundRegion.height)
        graphics->backgroundRegion.height = altHeight;
    
    OBJBlitter::BlitRegion tempRegion;
    tempRegion.Reset();
    tempRegion.width = regions[0].width;
    tempRegion.height = regions[0].height;
    OBJBlitter tempBlitter;
    tempBlitter.Reset();
    tempBlitter.SetDestination(&stagingBuffer_[graphics_->vramOffset + graphics_->charDataSize]);
    tempBlitter.SetRegions(&tempRegion, 1);
    tempBlitter.FillRectangle(0, 0, graphics->backgroundRegion.width, graphics->backgroundRegion.height, 1);
}

void OBJTextUI::DrawTextBoxBorder()
{
    OBJBlitter* blitter = graphics_->blitter;
    unsigned char width = graphics_->width;
    unsigned char height = graphics_->height;

    const void* topTile = GetWindowBorderTile(1);
    const void* bottomTile = GetWindowBorderTile(6);
    if (topTile != NULL && bottomTile != NULL)
    {
        int numTiles = (width - 9) >> 3;
        for (int x = 8, i = 0; i < numTiles; x += 8, i++)
        {
            blitter->BlitTile(topTile, x, 0, 8, 15);
            blitter->BlitTile(bottomTile, x, height - 8, 8, 15);
        }
    }

    const void* leftTile = GetWindowBorderTile(3);
    const void* rightTile = GetWindowBorderTile(4);
    if (leftTile != NULL && rightTile != NULL)
    {
        int numTiles = (height - 9) >> 3;
        for (int y = 8, i = 0; i < numTiles; y += 8, i++)
        {
            blitter->BlitTile(leftTile, 0, y, 8, 15);
            blitter->BlitTile(rightTile, width - 8, y, 8, 15);
        }
    }

    short cornerX[] = { 0, width - 8, 0, width - 8 };
    short cornerY[] = { 0, 0, height - 8, height - 8 };

    for (int i = 0; i < 4; i++)
    {
        const void* cornerTile = GetWindowBorderTile(data_020ee94c[i]);
        if (cornerTile != NULL)
            blitter->BlitTile(cornerTile, cornerX[i], cornerY[i], 8, 15);
    }
}

#if defined(usa)
void OBJTextUI::DrawText(int textID, int x, int y,
    const char* text, unsigned char colorIndex, unsigned char fontIndex)
{
    if (textID >= 0 && textID < 3)
    {
        int width = MeasureTextWidth(text, fontIndex);
        optionBounds_[textID].Set(x, y, x + width, y + 12);
    }
    if (numOptions_ <= textID)
        numOptions_ = textID + 1;
    graphics_->blitter->BlitText(x, y, text, colorIndex, fontIndex);
}
#elif defined(jpn)
void OBJTextUI::DrawText(int textID, int x, int y,
    const char* text, unsigned char size, unsigned char colorIndex)
{
    if (textID >= 0 && textID < 3)
    {
        int length = strlen(text);
        optionBounds_[textID].Set(x, y, x + length * size, y + size);
    }
    if (numOptions_ <= textID)
        numOptions_ = textID + 1;
    graphics_->blitter->BlitText(x, y, text, size, colorIndex);
}
#endif

void OBJTextUI::FillRectangle(unsigned char colorIndex, short left, short top, short right, short bottom)
{
    graphics_->blitter->FillRectangle(left, top, right, bottom, colorIndex);
}

void OBJTextUI::EndRender()
{
    if (graphics_ == NULL || stagingBuffer_ == NULL)
        return;
    
    graphics_->readyToSubmit = true;
    StageMemoryToVRAM(data_020ee950[graphics_->engine], 
        stagingBuffer_ + graphics_->vramOffset,
        graphics_->vramOffset, 
        graphics_->charDataSize + graphics_->GetPixelDataSize(),
        true, false);
}

void OBJTextUI::FreeStagingMemory()
{
    if (stagingBuffer_ != NULL)
        FreeVRAMStagingMemory(stagingBuffer_);
}