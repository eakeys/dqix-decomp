#ifdef jpn

#include "UI/Text.h"
#include "NitroSDK/Graphics.h"
#include "Graphics/Vector.h"
#include "NitroSDK/BGBases.h"
#include "NitroSDK/ColorEffects.h"
#include "Filesystem/FileIO.h"
#include "Filesystem/BackgroundLoader.h"
#include "Graphics/VRAMStaging.h"
#include "GameState/GameState.h"
#include "Util/StringTests.h"
#include "Resource/PAC.h"
#include "Filesystem/GPC.h"
#include <globaldefs.h>
#include <asmhacks.h>

struct GlyphPixelBuffer
{
    unsigned char pixels[0x80];
};

union StagingSpace
{
    struct
    {
        unsigned short backgroundIndexBuffer[0x400];
        unsigned short textBorderIndexBuffer[0x400];
    };

    GlyphPixelBuffer glyphPixelData[32];
};

extern unsigned int data_0210773c; // palette VRAM offset for glyphs
struct Struct_02107740
{
    char unk_0[4];
    unsigned char* windowBorderTiles;
    char unk_8[0xc];
    JPNFont* f6MesFile;
    char unk_18[8];
    JPNFont* f8MesFile;
    JPNFont* f10MesFile;
    JPNFont* f12MesFile;
    char unk_2c[4];
    StagingSpace* stagingSpace;
    char unk_34[4];
    TextManager* manager;

    // unplaced so far
    
    unsigned short* glyphIndexBuffer;
    char* secondaryScratchBuffer;
    char* fullMessageBuffer;
    char* primaryScratchBuffer;
    char unk_20[4]; // something related to sound
    void* buffer_24;
    char* currentPageMessageBuffer;
    FontIndexFile* pFontIndexFiles[2];
    FontDataFile* pFontDataFiles[2];
} extern data_02107740;
extern SafeAllocator data_0210783c;

extern char data_01ff9740[];

extern const unsigned char data_020e7a50[]; // probably multiple separate things. We only use stuff at offset 8
extern const unsigned char data_020e7a8c[];
extern const int data_020e7a94[]; // { 2, 3 }
extern const char data_020e7a9c[]; // { 1, 1, 1, 1, 1, 1, 1, 1, 15 }
extern const short data_020e7aa6[]; // { 1, 2, 2, 3, 2 }
extern const short data_020e7ab0[]; // { 2, 1, 3, 2, 2 }
extern const unsigned char data_020e7ade[3][8];
extern const unsigned short data_020e7af8[];

extern char data_020efe80[]; // "<X=%d>"
extern char data_020efe87[]; // "<Y=%d>"
extern char data_020efe8e[]; // "<XY=%d,%d>"
extern char data_020efe99[]; // "<W=%d>"
extern char data_020efea0[]; // "<H=%d>"
extern char data_020efea7[]; // "<WH=%d,%d>"
extern char data_020efeb2[]; // "<ENC=%d>", not used in jpn
extern char data_020efebb[]; // "<N=%d>%s</N>"
extern char data_020efec8[]; // "<SDRC=%d,%d,%d,%d,%d>"
extern char data_020efede[]; // "<SOLID=%d,%d>"
extern char data_020efeec[]; // "<FRAME=%d,%d,%d,%d,%d>"
extern char data_020eff03[]; // "<WIRE=%d,%d,%d,%d,%d>"
extern char data_020eff19[]; // "<LINE=%d>"
extern char data_020eff23[]; // "<LINEX=%d,%d,%d,%d>"
extern char data_020eff37[]; // "<LINEY=%d,%d,%d,%d>"
extern char data_020eff4b[]; // "<SIZE=%d>"
extern char data_020eff55[]; // "<WHITE>"
extern char data_020eff5d[]; // "<PINK>"
extern char data_020eff64[]; // "<YELLOW>"
extern char data_020eff6d[]; // "<RED>"
extern char data_020eff73[]; // "<PLTT=%d>"
extern char data_020eff7d[]; // "<SLT=%d>"
extern char data_020eff86[]; // "<CURSOR=%d>"
extern char data_020eff92[]; // "<UA=%d,%d,%d>"
extern char data_020effa0[]; // "<UB=%d,%d,%d>"
extern char data_020effae[]; // "<DA=%d,%d,%d>"
extern char data_020effbc[]; // "<DB=%d,%d,%d>"
extern char data_020effca[]; // "<TEN=%d,%d>"
extern char data_020effd6[]; // "<TITLE=%d>%s</TITLE>"
extern char data_020effeb[]; // "<TALK>%s</TALK>"


extern char data_020f008a[]; // 0x82 0x60, shiftJIS "A"
extern char data_020f008d[]; // shiftJIS "a"
extern char data_020f0090[]; // shiftJIS "0"

extern char data_EN_020efffc[]; // "<XR=%d>%s</XR>"
extern char data_EN_020f000b[]; // "/data/ani/windata3.bncg"
extern char data_EN_020f0023[]; // "/data/pack_lv5/font_lv5.gp2"
extern char data_EN_020f003f[]; // "f8.mes"
extern char data_EN_020f0046[]; // "%d"
extern char data_EN_020f0049[]; // "data/pack_lv5/fi_%s.bin"
extern char data_EN_020f0061[]; // "data/pack_lv5/fd_%s.bin"
extern char data_EN_020f0079[]; // "s7"
extern char data_EN_020f007c[]; // "me"
extern char data_EN_020f007f[]; // "data/ani/obj_tm.gp2"
extern char data_EN_020f0093[]; // "obj_tm_<LG>.pac"
extern char data_EN_020f00a3[]; // "TIME="
extern char data_EN_020f00a9[]; // "<RECT="
extern char data_EN_020f00b0[]; // "<WIN>"
extern char data_EN_020f00b6[]; // "<CEN>"
extern char data_EN_020f00bc[]; // "<MOJI="
extern char data_EN_020f00c3[]; // "<SKIP="

extern char data_EN_02108760[];

static void SetF8MesFile(void* to);
void FixJPNFontPointers(void* fileData);

extern "C"
{
    // probably atoi
    int func_02005a94(const char*);

    void* func_0203dce4(void*, int);
    void* func_0203cf4c();
    void func_02040af0(void*, fix32_t);

    void func_0205e944(void*, int);
    void func_0205eaa0(void*, int, void*);
    void func_0205eabc(void*, void*, int);

    void func_020656e4(TextManager*, int);

    // close quest screen maybe?
    void func_0208beb4(void*);

    void* func_020952d4();
    void func_02094a10(void*);
    void func_02094a20(void*, SafeAllocator*);
    void func_02094a24(void*);
    JPNFont* func_02095b40(void*, int);

    void func_020bc078(void*);

    // alternative strlen
    int func_020d4a54(const char*);
    
    // populate noun from item ID?
    void func_020dcf7c(short, Noun*);
}

void WriteTexCoords(fix32_t x, fix32_t y); // can be made static

void RenderGlyph::Reset()
{
    textureData = NULL;
    textureVRAMOffset = 0;
    bitAllocationIndex = -1;
    drawX = 0;
    drawY = 0;
    drawZ = 0;
    indexGlyph = NULL;
    unk_16_low = 0;
    unk_16_1 = 0;
    unk_16_2 = 0;
    fontIndex = 0;
    indexInFontFile = 0;
}

void RenderGlyph::Reset2()
{
    if (ALWAYS_FALSE() || ALWAYS_FALSE()) // prevents inlining
        Reset();
    Reset();
}

const char* RenderGlyph::GetGlyphTag() const
{
    if (indexGlyph != NULL)
        return indexGlyph->tag;
    return NULL;
}

void RenderGlyph::Populate(unsigned char fontFileIndex, const FontIndexFile::Glyph *glyph,
    const void *pixels, int bitAllocIndex, int textureVRAMOff, unsigned char fontIdx)
{
    if (pixels == NULL)
        return;
    Reset();
    indexInFontFile = fontFileIndex;
    indexGlyph = glyph;
    bitAllocationIndex = bitAllocIndex;
    textureData = pixels;
    textureVRAMOffset = textureVRAMOff;
    unk_16_low = true;
    unk_16_1 = false;
    unk_16_2 = false;
    fontIndex = fontIdx;
}

void RenderGlyph::UploadToVRAM()
{
    if (bitAllocationIndex < 0)
        return;
    if (unk_16_low && textureData != NULL)
    {
        StageMemoryToVRAM(VRAMSubregion_TextureImage, textureData, textureVRAMOffset, 0x80, true, true);
        unk_16_low = false;
    }
}

void RenderGlyph::Draw(int color, void *unknown, int alpha)
{
    if (bitAllocationIndex < 0 || unk_16_low || alpha == 0)
        return;

    GXFIFO_MATRIX_PUSH = 0;

    fix32_t fixPointZ = (drawZ + 1024) << 12;
    fix32_t fixPointX = drawX << 12;
    fix32_t fixPointY = drawY << 12;
    float zero = 0.0f;
    
    GXFifoWriteTranslate(fixPointX, fixPointY, fixPointZ);
    GXFifoWriteScale(MAKEFIX32(1), MAKEFIX32(1), MAKEFIX32(1));

    GXFIFO_MATRIX_PUSH = 0;
    GXFifoWriteTranslate(MAKEFIX32(0), MAKEFIX32(16), MAKEFIX32(0));
    GXFifoWriteScale(MAKEFIX32(1), MAKEFIX32(-1), MAKEFIX32(1));

    GXFIFO_MATRIX_PUSH = 0;

    GXFIFO_DIFFUSE_AMBIENT = 0x7fffffff;
    GXFIFO_SPECULAR_EMISSION = 0x4210;
    GXFIFO_TEXIMAGE_PARAMS = (textureVRAMOffset >> 3) | 0x6c900000;
    GXFIFO_PALETTE_BASE = data_0210773c >> 4;
    GXFIFO_POLYGON_ATTRIBUTES = (alpha << 16) | 0x3e0000c0;

    GXFifoWriteScaleFloat(16.0f, 16.0f, 1.0f);

    GXFIFO_POLYGON_BEGIN = 1;
    GXFIFO_VERTEX_COLOR = color;
    
    int topYTexcoord = 16;
    topYTexcoord *= zero;
    float topYVertexCoord = 1.0f - zero;

    if (unknown != NULL)
    {
        int yAdjust = *(int*)((intptr_t)unknown + 4);
        if (drawY < yAdjust)
            topYTexcoord += yAdjust - drawY;
        if (topYTexcoord > 16)
            topYTexcoord = 16;
        topYVertexCoord = (16.0f - (float)topYTexcoord) / 16.0f;
    }
    // top left
    WriteTexCoords(0, topYTexcoord << 12);
    GXFifoWriteVertex16Float(0, topYVertexCoord, 0);

    // top right
    WriteTexCoords(16 << 12, topYTexcoord << 12);
    GXFifoWriteVertex16(MAKEFIX32(1), topYVertexCoord * 4096.0f, MAKEFIX32(0));

    // bottom right
    WriteTexCoords(16 << 12, 16 << 12);
    GXFifoWriteVertex16(MAKEFIX32(1), MAKEFIX32(0), MAKEFIX32(0));

    // bottom left
    WriteTexCoords(0, 16 << 12);
    GXFifoWriteVertex16Float(0, 0, 0);

    GXFIFO_POLYGON_END = 0;
    GXFIFO_MATRIX_POP = 1;
    GXFIFO_MATRIX_POP = 1;
    GXFIFO_MATRIX_POP = 1;
}

void WriteTexCoords(fix32_t x, fix32_t y)
{
    // TEXCOORD expects x and y as fixed points with 4 places after the point instead of 12
    short xFixed4 = x >> 8;
    short yFixed4 = y >> 8;

    GXFIFO_VERTEX_TEXCOORD = (unsigned short)xFixed4 | ((unsigned short)yFixed4 << 16);
}

namespace StringBuilders
{
int AddXTag(char* buffer, int x)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efe80, x) - buffer;
}

int AddYTag(char* buffer, int y)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efe87, y) - buffer;
}

int AddXYTag(char* buffer, int x, int y)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efe8e, x, y) - buffer;
}

int AddWidthTag(char* buffer, int w)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efe99, w) - buffer;
}

int AddHeightTag(char* buffer, int h)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efea0, h) - buffer;
}

int AddWidthHeightTag(char* buffer, int w, int h)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efea7, w, h) - buffer;
}

// missing function? that uses <ENC=%d>

int AddIndexedTag(char* buffer, int index, const char* value)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efebb, index, value) - buffer;
}

int AddSolidRectTag(char* buffer, int col, int x, int y, int width, int height)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efec8, col, x, y, width, height) - buffer;
}

// missing function? using <SOLID=%d,%d>

// Couldn't find a call site in game, but there's a bunch in overlay 2
int AddFrameTag(char* buffer, int col, int x, int y, int width, int height)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020efeec, col, x, y, width, height) - buffer;
}

// called twice in overlay 2
int AddWireTag(char* buffer, int col, int x, int y, int width, int height)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff03, col, x, y, width, height) - buffer;
}

// used to draw a horizontal line in skill point menu, parameter is y coordinate relative to container?
int AddLineTag(char* buffer, int y)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff19, y) - buffer;
}

// horizontal line segment, not sure about first argument
int AddLineXTag(char* buffer, int col, int left, int right, int y)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff23, col, left, right, y) - buffer;
}

int AddLineYTag(char* buffer, int col, int x, int top, int bottom)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff37, col, x, top, bottom) - buffer;
}

int AddSizeTag(char* buffer, int size)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff4b, size) - buffer;
}

int AddReducedPaletteTag(char* buffer, int reducedEnum)
{
    char* writePos = buffer + func_020d4a54(buffer);
    switch (reducedEnum)
    {
    case 0:
        writePos += sprintf(writePos, data_020eff55); // white
        break;
    case 5:
        writePos += sprintf(writePos, data_020eff5d); // pink (orange?)
        break;
    case 4:
        writePos += sprintf(writePos, data_020eff64); // yellow
        break;
    case 6:
        writePos += sprintf(writePos, data_020eff6d); // red
        break;
    }
    return writePos - buffer;
}

int AddPaletteTag(char* buffer, int palette)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff73, palette) - buffer;
}

// missing <SLT=%d> function?

int AddCursorTag(char* buffer, int cursorPos)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff86, cursorPos) - buffer;
}

// not sure of purpose, but used in bank when depositing/withdrawing.
// removing it doesn't get rid of the arrows though
int AddUATag(char* buffer, int arg1, int arg2, int arg3)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020eff92, arg1, arg2, arg3) - buffer;
}

// unused functions? <UB=%d,%d,%d>, <DA=%d,%d,%d>

// same call site as UA (bank)
int AddDBTag(char* buffer, int arg1, int arg2, int arg3)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020effbc, arg1, arg2, arg3) - buffer;
}

// used once in overlay 17 with args 3,2, not sure of purpose
int AddTenTag(char* buffer, int x, int y)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020effca, x, y) - buffer;
}

// used for e.g. "Whose?" when assigning skill points out of battle
int AddTitleTag(char* buffer, const char* text, int index)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020effd6, index, text) - buffer;
}

// used somewhere in overlay 3 (shops, inn, bank?) but I couldn't find it
int AddTalkTag(char* buffer, const char* text)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, data_020effeb, text) - buffer;
}

int AddText(char* buffer, const char* text)
{
    char* writePos = buffer + func_020d4a54(buffer);
    return writePos + sprintf(writePos, text) - buffer;
}

int AddCenteredText(char* buffer, const char* text, int containerWidth, int fontSize)
{
    int requiredWidth = func_020d4a54(text);
    requiredWidth *= fontSize;
    requiredWidth >>= 1;
    sprintf(buffer + func_020d4a54(buffer), data_020efe99, ((containerWidth - requiredWidth) >> 1) + 1);
    sprintf(buffer + func_020d4a54(buffer), text);
    return func_020d4a54(buffer);
}   

}

unsigned char GetFontIDBySize(int size)
{
    unsigned char id = 0; // s7
    if (size == 12)
        id = 1; // me
    return id;
}

TextManager* TextManager::GetInstance()
{
    return data_02107740.manager;
}

const unsigned char* GetWindowBorderTile(int id)
{
    return data_02107740.windowBorderTiles + (id * 32);
}

void InitializeTextSystem(SafeAllocator* alloc)
{
    void* allocatorBuffer = alloc->Allocate(0x400);
    data_0210783c.CreateTypeA(allocatorBuffer, 0x400);
    data_02107740.manager = (TextManager*)alloc->Allocate(sizeof(TextManager));
    data_02107740.stagingSpace = (StagingSpace*)alloc->Allocate(0xf99);
    data_02107740.manager->mainUIElement_34_ = (UIElement*)alloc->Allocate(sizeof(UIElement));
    data_02107740.fullMessageBuffer = (char*)alloc->Allocate(0x960);
    data_02107740.glyphIndexBuffer = (unsigned short*)alloc->Allocate(0x280);
    data_02107740.currentPageMessageBuffer = (char*)alloc->Allocate(0x280);
    data_02107740.buffer_24 = alloc->Allocate(0x960);
    data_02107740.primaryScratchBuffer = (char*)alloc->Allocate(0x960);
    data_02107740.secondaryScratchBuffer = (char*)alloc->Allocate(0x960);
    LoadCustomFonts(alloc);
    BackgroundLoader::AddLockGlobal();

    unsigned int winDataSize = 0;
    void* winData = LoadFileIntoMemory(data_EN_020f000b, g_fileScratchSpace, &winDataSize);
    BGLayer::CharacterData charData;
    charData.Reset();
    if (winData != NULL)
        memcpy(&charData, winData, 12);
    
    unsigned int winDataVRAMSize = charData.GetVRAMSize();
    data_02107740.windowBorderTiles = (unsigned char*)alloc->Allocate(winDataVRAMSize);
    unsigned char* borderTileSource = g_fileScratchSpace;
    memcpy(data_02107740.windowBorderTiles, borderTileSource + 0x10, winDataVRAMSize);

    unsigned int fontInnerFileOffset;
    unsigned int zero = 0;
    unsigned int fontInnerFileSize = 0;
    unsigned char* fontLv5Storage = g_fileScratchSpace;
    unsigned int bufferCapacity = 0x30000;
    ScopedGPCReader reader;

    if (LoadAndDecompressGPCHeaderAndInnerFileInfo(&reader.pGPCFile, &reader.machine, data_EN_020f0023,
            fontLv5Storage, &fontInnerFileSize, bufferCapacity, false, NULL))
    {
        fontInnerFileOffset = fontInnerFileSize;
        DecompressFileFromGPCByName(&reader.pGPCFile, &reader.machine, 
            fontLv5Storage + fontInnerFileOffset, &fontInnerFileSize,
            bufferCapacity - fontInnerFileOffset, data_EN_020f003f);
        unsigned int alignedLength = (fontInnerFileSize + 3) & ~3;
        reader.Reset();
        fontInnerFileSize = zero + (zero + (zero + alignedLength));
        memcpy(data_01ff9740, fontLv5Storage + fontInnerFileOffset, fontInnerFileSize);
    }

    BackgroundLoader::RemoveLockGlobal();
    data_0210783c.Reset();
    TextManager* mgr = data_02107740.manager;
    mgr->cursorObjectsAllocator_2d4_ = &data_0210783c;

    void* unknownObject = alloc->Allocate(0x68);
    func_02094a24(unknownObject);
    func_02094a20(unknownObject, alloc);
    func_02094a10(unknownObject);
    FixJPNFontPointers(data_01ff9740 + zero);

    SetF8MesFile(data_01ff9740 + zero);
    
    data_02107740.manager->fullMessage_48_ = data_02107740.fullMessageBuffer;
    data_02107740.manager->glyphIndexList_4c_ = data_02107740.glyphIndexBuffer;
    data_02107740.manager->currentPageMessage_50_ = data_02107740.currentPageMessageBuffer;
    data_02107740.manager->ptr_5c = data_02107740.buffer_24;
    data_02107740.manager->pageScratchBuffer_60_ = data_02107740.primaryScratchBuffer;
    data_02107740.manager->scratchBuffer_64_ = data_02107740.secondaryScratchBuffer;
}

char* GetTextTagEnd(char* input)
{
    if (input != NULL)
    {
        while (*input != 0)
        {
            if (input[0] == '>' && input[1] != '>')
                return input;
            input++;
        }
    }
    return NULL;
}

int GetNextFontGlyphIndex(const char* tag, int fontType)
{
    if (tag == NULL)
        return -1;
    const FontIndexFile* file = data_02107740.pFontIndexFiles[fontType];
    for (unsigned int i = 0; i < file->numCharEntries; i++)
    {
        const FontIndexFile::Glyph& glyph = file->glyphs[i];
        if (memcmp(glyph.tag, tag, glyph.tagLength) == 0)
            return i;
    }
    return -1;
}

FontIndexFile::Glyph* GetNextFontGlyph(const char* tag, int fontType)
{
    if (tag == NULL)
        return NULL;

    FontIndexFile* indexFile = data_02107740.pFontIndexFiles[fontType];
    for (unsigned int i = 0; i < indexFile->numCharEntries; i++)
    {
        FontIndexFile::Glyph* glyph = &indexFile->glyphs[i];
        if (memcmp(glyph->tag, tag, glyph->tagLength) == 0)
            return glyph;
    }

    return NULL;
}

FontIndexFile::Glyph* GetFontGlyphByIndex(int glyph, int fontType)
{
    FontIndexFile* file = data_02107740.pFontIndexFiles[fontType];
    if (glyph >= 0 && glyph < file->numCharEntries)
        return &file->glyphs[glyph];
    return NULL;
}

int GetFontGlyphKerning(int leftGlyph, int rightGlyph, int fontType)
{
    FontIndexFile* file = data_02107740.pFontIndexFiles[fontType];
    unsigned int numKerningEntries = file->numKerningEntries;

    FontIndexFile::KerningEntry* loopEntry = file->kerning;
    uint16_t targetPair = (leftGlyph & 0xff) | ((rightGlyph & 0xff) << 8);
    for (unsigned int i = 0; i < numKerningEntries; i++)
    {
        if (loopEntry->glyphPair == targetPair)
            return loopEntry->kerningAmount;
        loopEntry++;
    }
    return 0;
}

FontDataFile* GetFontDataFile(int fontType)
{
    return data_02107740.pFontDataFiles[fontType];
}

int GetFontSpaceSize(int fontType)
{
    return data_020e7a94[fontType];
}

int MeasureNumberTextWidth(int fontType, int num)
{
    char buffer[32] = {0};
    if (num < 0)
        num = -num;
    sprintf(buffer, data_EN_020f0046, num);
    return MeasureTextWidth(buffer, fontType);
}

int MeasureNumberTextWidthGivenSize(int fontSize, int num)
{
    return MeasureNumberTextWidth(GetFontIDBySize(fontSize), num);
}

int EncodeDQ9Text(char* decoded, unsigned char* output, int fontType)
{
    if (output == NULL)
        return 0;
    if (decoded == NULL)
    {
        if (output != NULL)
            output[0] = '\0';
        return 0;
    }

    int outputLength = 0;
    while (true)
    {
        char decodedChar = *decoded;
        if (decodedChar == '\0')
        {
            *output = '\0';
            break;
        }

        int tagLength = 1;
        unsigned char outByte = 0;

        FontIndexFile::Glyph* glyph = GetNextFontGlyph(decoded, fontType);
        if (glyph != NULL)
        {
            tagLength = glyph->tagLength;
        }
        if (decodedChar == ' ')
            outByte = 0xff;
        int lookupGlyphIndex = GetNextFontGlyphIndex(decoded, fontType);
        if (lookupGlyphIndex > 0)
            outByte = lookupGlyphIndex;

        *output = outByte;
        output++;
        decoded += tagLength;
        outputLength++;
    }
    return outputLength;
}

void DecodeDQ9Text(const unsigned char* encoded, char* output, int fontType)
{
    if (output == NULL)
        return;

    if (encoded == NULL)
    {
        if (output != NULL)
            output[0] = '\0';
        return;
    }
    
    while (true)
    {
        unsigned char encodedByte = *encoded;
        if (encodedByte == '\0')
        {
            *output = '\0';
            return;
        }
        const unsigned char fallback[] = { data_020e7a50[8], data_020e7a50[9] };
        const void* copySource = fallback;
        int copyLength = 1;
        FontIndexFile::Glyph* glyph = GetFontGlyphByIndex(encodedByte, fontType);
        if (glyph != NULL)
        {
            copySource = glyph->tag;
            copyLength = glyph->tagLength;
        }

        memcpy(output, copySource, copyLength);
        encoded++;
        output += copyLength;
    }
}

void LoadCustomFont(SafeAllocator* alloc, const char* name, FontIndexFile** ppIndex, FontDataFile** ppData)
{
    char fileName[64] = {0};
    sprintf(fileName, data_EN_020f0049, name);
    unsigned int fileSize = 0;
    LoadFileIntoMemory(fileName, g_fileScratchSpace, &fileSize);

    if (fileSize != 0)
    {
        *ppIndex = (FontIndexFile*)alloc->Allocate(fileSize);
        memcpy(*ppIndex, g_fileScratchSpace, fileSize);
        FontIndexFile* basePtr = *ppIndex;
        intptr_t baseAddr = (intptr_t)basePtr;
        // convert stored offsets into legitimate pointers
        basePtr->kerning = (FontIndexFile::KerningEntry*)(baseAddr + (intptr_t)basePtr->kerning);
        basePtr->glyphs = (FontIndexFile::Glyph*)(baseAddr + (intptr_t)basePtr->glyphs);
        basePtr->tagPool = (const char*)(baseAddr + (intptr_t)basePtr->tagPool);

        FontIndexFile::Glyph* entry;
        for (unsigned int i = 0; i < basePtr->numCharEntries; i++)
        {
            entry = basePtr->glyphs + i;
            entry->tag = (const char*)(baseAddr + (intptr_t)entry->tag);
        }
    }

    memset(fileName, 0, sizeof(fileName));
    sprintf(fileName, data_EN_020f0061, name);
    fileSize = 0;
    LoadFileIntoMemory(fileName, g_fileScratchSpace, &fileSize);

    if (fileSize != 0)
    {
        *ppData = (FontDataFile*)alloc->Allocate(fileSize);
        memcpy(*ppData, g_fileScratchSpace, fileSize);
        FontDataFile* basePtr = *ppData;
        basePtr->ptr_c = (unsigned char*)((intptr_t)basePtr + (intptr_t)basePtr->ptr_c);
    }
}

void LoadCustomFonts(SafeAllocator* alloc)
{
    BackgroundLoader::AddLockGlobal();
    LoadCustomFont(alloc, data_EN_020f0079, &data_02107740.pFontIndexFiles[0], &data_02107740.pFontDataFiles[0]);
    LoadCustomFont(alloc, data_EN_020f007c, &data_02107740.pFontIndexFiles[1], &data_02107740.pFontDataFiles[1]);
    BackgroundLoader::RemoveLockGlobal();
}

static void SetF8MesFile(void* to)
{
    if (to != NULL)
        data_02107740.f8MesFile = (JPNFont*)to;
}

void FixJPNFontPointers(void* fileData)
{
    if (fileData == NULL)
        return;
    JPNFont* font = (JPNFont*)fileData;
    font->sjisKeys = (unsigned char*)((intptr_t)font + (intptr_t)font->sjisKeys);
    font->pixelData = (unsigned char*)((intptr_t)font + (intptr_t)font->pixelData);
}

JPNFont* GetJPNFont(int size)
{
    JPNFont* font = NULL;
    switch (size)
    {
    case 6:
        font = data_02107740.f6MesFile;
        break;
    case 8:
        font = data_02107740.f8MesFile;
        break;
    case 10:
        font = data_02107740.f10MesFile;
        break;
    case 12:
        font = data_02107740.f12MesFile;
        break;
    }
    return font;
}

void* GetFontStagingSpace()
{
    return data_02107740.stagingSpace;
}

struct AsciiConversionRule
{
    char ascii;
    const char* sjis;
} extern const data_020e848c[];


#define WRITE_SJIS_FROM_RANGE(to, sjisBase, asciiBase, asciiChar) do { \
        const unsigned char* data = (const unsigned char*)sjisBase; \
        int diff = asciiChar - asciiBase; \
        to[0] = data[0]; \
        to[1] = data[1] + diff; \
    } while (false)

void ConvertAsciiToShiftJIS(unsigned char* output, const char* input)
{
    if (output == NULL)
        return;
    if (input == NULL)
    {
        output[0] = 0;
        output[1] = 0;
        return;
    }

    int ch = input[0];
    if (ch >= 'A' && ch <= 'Z')
    {
        WRITE_SJIS_FROM_RANGE(output, data_020f008a, 'A', ch);
        return;
    }

    if (ch >= 'a' && ch <= 'z')
    {
        WRITE_SJIS_FROM_RANGE(output, data_020f008d, 'a', ch);
        return;
    }

    if (ch >= '0' && ch <= '9')
    {
        WRITE_SJIS_FROM_RANGE(output, data_020f0090, '0', ch);
        return;
    }

    const char* replacementSjis = input;
    for (const AsciiConversionRule* rule = data_020e848c;; rule++)
    {
        if (rule->ascii == 0)
            break;
        if (input[0] == rule->ascii)
        {
            replacementSjis = rule->sjis;
            break;
        }
    }
    output[0] = ((const unsigned char*)replacementSjis)[0];
    output[1] = ((const unsigned char*)replacementSjis)[1];
}

int GetJPNFontGlyphIndex(const JPNFont* font, const char* text)
{
    if (font == NULL)
        return -1;
    if (text == NULL || text[0] == 0)
        return -1;
    unsigned char converted[2];
    ConvertAsciiToShiftJIS(converted, text);
    int numEntries = font->numEntries;
    const unsigned char* keyLookup = font->sjisKeys;
    for (int i = 0; i < numEntries; i++)
    {
        if (converted[0] == keyLookup[0] && converted[1] == keyLookup[1])
        {
            return i;
        }
        keyLookup += 2;
    }
    return -1;
}

int GetJPNFontGlyphIndex(int fontSize, const char* text)
{
    if (text == NULL || text[0] == 0)
        return -1;
    JPNFont* font = GetJPNFont(fontSize);
    int index = GetJPNFontGlyphIndex(font, text);
    if (index >= 0)
        return index;
    // fallback font or something?
    font = func_02095b40(func_020952d4(), fontSize);
    return GetJPNFontGlyphIndex(font, text) | 0x40000000;
}

const void* GetJPNCharacterBitmap(JPNFont* font, const char* text)
{
    if (font == NULL)
        return NULL;

    unsigned char sjis[2];
    ConvertAsciiToShiftJIS(sjis, text);
    
    int numEntries = font->numEntries;
    const unsigned char* keyLookup = font->sjisKeys;
    for (int i = 0; i < numEntries; keyLookup += 2, i++)
    {
        if (sjis[0] == keyLookup[0] && sjis[1] == keyLookup[1])
            return font->pixelData + i * font->stride;
    }

    return NULL;
}

const void* GetJPNCharacterBitmap(int size, const char* sjis)
{
    if (sjis == NULL)
        return NULL;

    JPNFont* font = GetJPNFont(size);
    const void* bitmap = GetJPNCharacterBitmap(font, sjis);
    if (bitmap != NULL)
        return bitmap;

    font = func_02095b40(func_020952d4(), size);
    bitmap = GetJPNCharacterBitmap(font, sjis);
    return bitmap;
}

const void* GetJPNCharacterBitmap(int size, int index)
{
    if (index < 0)
        return NULL;
    JPNFont* font;
    if (index & 0x40000000)
    {
        font = func_02095b40(func_020952d4(), size);
        index &= ~0x40000000;
    }
    else
        font = GetJPNFont(size);

    if (font == NULL)
        return NULL;
    if (font->numEntries <= index)
        return NULL;
    else
        return font->pixelData + index * font->stride;
}

// keep, it looks like 0204316c
int TextManager::MaybeAcquireGlyphVRAM(unsigned char *bitfield, unsigned int numBytes)
{
    for (unsigned int byteIdx = 0; byteIdx < numBytes; bitfield++, byteIdx++)
    {
        unsigned char currentByte = *bitfield;
        for (int bit = 0; bit < 8; bit++)
        {
            unsigned char mask = 1 << bit;
            if (!(currentByte & mask))
            {
                *bitfield |= mask;
                return byteIdx * 8 + bit;
            }
        }
    }
    return -1;
}

void TextManager::MaybeReleaseGlyphVRAM(unsigned char* bitfield, int idx)
{
    unsigned int lookupIdx = idx >> 3;
    unsigned int lookupBit = idx & 7;
    bitfield[lookupIdx] &= ~(1 << lookupBit);
}

void TextManager::SetActiveTextBox(TextBox* to)
{
    textBoxGraphics_19e0_.activeBox_ = to;
}

void TextManager::SetMainTextBoxActive()
{
    SetActiveTextBox(&mainTextBox_914_);
}

void TextManager::SetTextureArena(TextureArena* to)
{
    textureArena_1e28_ = to;
}

void TextManager::SetModeOverworldMessage()
{
    mojiArg1_1860_ = 12;
    mojiArg2_1864_ = 16;
    ConfigureMainTextBox(2, 146, 252, 74);
    unknown_19b2_ = false;
    showWindow_19b1_ = false;
    effects_195b_ |= 2;
}

void SetViewport(int* viewport, int left, int top, int right, int bottom);

void TextManager::ConfigureMainTextBox(int x, int y, int width, int height)
{
    centerX_990_ = x + (width >> 1);
    textBoxGraphics_19e0_.SetBoxPosition(x, y);
    textBoxGraphics_19e0_.SetBoxSize(width, height);
    unknown_1858_ = x;
    unknown_185c_ = y + 8;
    SetViewport(unknown_1848_, x, y + 8, x + width, y + height);
}

void SetViewport(int* viewport, int left, int top, int right, int bottom)
{
    viewport[0] = left;
    viewport[1] = top;
    viewport[2] = right;
    viewport[3] = bottom;
}

void TextManager::Initialize()
{
    maybePartySize_30_ = 0xff;
    maybeNumAlive_31_ = 0xff;
    objUI_38_ = NULL;
    unknown_3c_ = 9;
    unknown_8c_ = NULL;
    dialogueOptions_90_.Reset();
    unknown_2c8_ = 0;
    memset(substruct_2cc_, 0, sizeof(substruct_2cc_));
    cursorObjectsLoadHandle_2d0_ = -1;
    cursorObjects_2d8_ = NULL;
    cursorMetatiles_2dc_ = NULL;
    cursorSpriteList_2e0_ = NULL;
    unknown_2e4_ = 0;
    unknown_2e6_ = 1;
    cursorObjectLoadStatus_2e7_ = 0;
    unknown_2e8_ = 0;
    memset(customStrings_4ac_, 0, sizeof(customStrings_4ac_));
    unknown_8ac_ = 0;
    memset(valueLookup_8b0_, 0, sizeof(valueLookup_8b0_));
    voiceSoundEffectTimer_912_ = 0;
    unknown_19b0_ = 0;
    fadeRescaledAlpha_9b0_ = 0;
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset();
        glyphRefcounts_17b8_[i] = 0;
    }
    chosenAnswer_954_ = 0;
    unknown_958_ = 0;
    unknown_1962_ = 0;
    clipWindows_[0].Initialize(0, 0);
    clipWindows_[1].Initialize(0, 1);
    mainTextBox_914_.Reset();
    speakerTextBox_930_.Reset();
    textBoxGraphics_19e0_.Reset();
    textBoxGraphics_19e0_.activeBox_ = &mainTextBox_914_;
    textBoxGraphics_19e0_.SetBoxPosition(0, 128);
    textBoxGraphics_19e0_.SetBoxSize(256, 64);
    showWindow_19b1_ = true;
    textureArena_1e28_ = NULL;
    unknown_19cc_ = 0;
    ResetTextBGLayers();
    skipTag_19bb_ = 1;
    unknown_994_ = -1;
    unknown_19bc_ = 0;
    unknown_19b2_ = 1;
    speakerVoiceType_99c_ = 0;
    voicePlaybackState_19b3_ = 0;
    unknown_19b4_ = 0;
    unknown_19b5_ = 0;
    pauseType_9a0_ = 0;
    color_187c_ = 0x7fff;
    gyouParameter_9a4_ = 3;
    numGlyphsInUse_9b4_ = 0;
    unknown_19ac_ = 0;
    waitTimeQueueIndex_19ad_ = 0;
    RecordWaitTimeValue(0, -1);
    cenTag_19b8_ = 0;
    unknown_1848_[0] = 0;
    unknown_1848_[1] = 0;
    unknown_1848_[2] = 256;
    unknown_1848_[3] = 192;
    SetViewport(unknown_1848_, 0, 136, 256, 192);
    unknown_19b9_ = 1;
    unknown_1858_ = 0;
    unknown_185c_ = 136;
    mojiArg1_1860_ = 12;
    mojiArg2_1864_ = 16;
    cursorBlinkTimer_19ae_ = 0;
    cursorVisibleThisFrame_19af_ = 0;
    unknown_1868_ = 0;
    npcID_1838_ = -1;
    npcPriorRotation_183c_ = 0;
    centerX_990_ = 128;
    memset(substructOrArray_95c_, 0, sizeof(substructOrArray_95c_));
    memset(substructOrArray_97c_, 0, sizeof(substructOrArray_97c_));
    speakerDifference_19c2_ = 1;
    memset(indexBuffer_1882_, 0xff01, sizeof(indexBuffer_1882_));
    memset(buffer_1902_, 0, sizeof(buffer_1902_));
    unknown_1942_ = 0;
    speakerNameWidth_1944_ = 0;
    unknown_19be_ = 0;
    questIndex_1948_ = -1;
    voiceVolumeLookup_194a_[0] = voiceVolumeLookup_194a_[1] = voiceVolumeLookup_194a_[2]
        = voiceVolumeLookup_194a_[3] = 127;
    unknown_195a_ = 8;
    effects_195b_ = 0;
    unknown_19bf_ = 0;
    unknown_19cf_ = 0;
    unknown_19cb_ = 0;
    unknown_19cc_ = 0;
    unknown_19ce_ = 0;
    func_020bc078(data_02107740.unk_20);
    unknown_19d0_ = 0;
    unknown_19da_ = -1;
    fontIndex_19dc_ = 1;
    showWindow_19b1_ = false;
}

void TextBox::Reset()
{
    x = y = width = height = 0;
    status = 0;
    unknown_c = 0;
    scaleX = scaleY = 0;
    color = 0x7fff;
}

void TextManager::Destroy()
{
    textBoxGraphics_19e0_.Destroy();
    Close();
    Cleanup();
    objUI_38_ = NULL;
}

void TextManager::DestroyRenderGlyphs()
{
    memset(substruct_2cc_, 0, sizeof(substruct_2cc_));
    numGlyphsInUse_9b4_ = 0;
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset2();
        glyphRefcounts_17b8_[i] = 0;
    }
    unknown_1868_ = 0;
    pauseType_9a0_ = 0;
    unknown_19b2_ = 0;
    unknown_19b4_ = 0;
}

void TextManager::ResetBoxes()
{
    textBoxGraphics_19e0_.ResetBoxStatus();
    textBoxGraphics_19e0_.activeBox_ = &speakerTextBox_930_;
    textBoxGraphics_19e0_.ResetBoxStatus();
    textBoxGraphics_19e0_.activeBox_ = &mainTextBox_914_;
    showWindow_19b1_ = false;
    cursorBlinkTimer_19ae_ = 0;
    cursorVisibleThisFrame_19af_ = 0;
    unknown_19bd_ = 0;
    npcID_1838_ = -1;
    npcPriorRotation_183c_ = 0;
}

void TextManager::ResetTextBGLayers()
{
    pauseType_9a0_ = 0;
    unknown_998_ = 0;
    if (unknown_19cc_ != 0)
    {
        unknown_19cc_ = 0;
        StagingSpace* indices = data_02107740.stagingSpace;
        memset(indices, 0, sizeof(StagingSpace));
        StageMemoryToVRAM(VRAMSubregion_MainBG2Screen, indices->backgroundIndexBuffer, 0, 0x800, true, false);
        StageMemoryToVRAM(VRAMSubregion_MainBG3Screen, indices->backgroundIndexBuffer, 0, 0x800, true, false);
        void* bg1screen = (void*)GetMainBG1ScreenBase();
        if (bg1screen != NULL)
            memset(bg1screen, 0, 0x800);
        BG2OFS = 0;
        // set enabled layers: enable bg0, bg1, obj, disable bg2, bg3
        DISPCNT = (DISPCNT & ~0x1f00) | 0x1300;
    }
    if (objUI_38_ != NULL)
        objUI_38_->RequestClose();
    objUI_38_ = NULL;
}

void TextManager::Cleanup()
{
    DestroyRenderGlyphs();
    ResetTextBGLayers();
    ResetBoxes();
}

void TextManager::Close()
{
    unknown_19cc_ = false;
    numGlyphsInUse_9b4_ = 0;
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset2();
        glyphRefcounts_17b8_[i] = 0;
    }
    ResetTextBGLayers();
    textBoxGraphics_19e0_.ResetBoxStatus();
    textBoxGraphics_19e0_.activeBox_ = &speakerTextBox_930_;
    textBoxGraphics_19e0_.ResetBoxStatus();
    textBoxGraphics_19e0_.activeBox_ = &mainTextBox_914_;
    showWindow_19b1_ = false;
    npcID_1838_ = -1;
    npcPriorRotation_183c_ = 0;
}

void TextManager::InitializeRenderGlyphs()
{
    if (textureArena_1e28_ != NULL)
        data_0210773c = textureArena_1e28_->GetPaletteVRAMOffset();
    StageMemoryToVRAM(VRAMSubregion_TexturePalette, data_020e7af8, data_0210773c, 0x20, true, true);
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset2();
        glyphRefcounts_17b8_[i] = 0;
    }
    RefreshGraphics(false);
}

void TextManager::LoadCursorGraphics()
{
    if (cursorObjectsAllocator_2d4_ == NULL)
        return;
    
    BackgroundLoader* loader = BackgroundLoader::GetInstance();
    if (cursorObjectLoadStatus_2e7_ == 0)
    {
        if (cursorObjects_2d8_ == NULL)
        {
            GrottoStruct* grotto = GameState::GetInstance()->GetGrottoStruct();
            if (grotto == NULL || grotto->unknown_0 == 0 || grotto->unknown_2 != 0)
            {
                cursorObjectsLoadHandle_2d0_ = loader->QueueLoadFileInGP2(data_EN_020f007f, data_EN_020f0093, NULL);
                cursorObjectLoadStatus_2e7_++;
            }
        }
    }
    else if (cursorObjectLoadStatus_2e7_ == 1)
    {
        if (loader->GetTaskStatus(cursorObjectsLoadHandle_2d0_) != 0)
        {
            cursorObjectsAllocator_2d4_->Reset();
            cursorObjects_2d8_ = (OBJSpriteSet*)cursorObjectsAllocator_2d4_->Allocate(sizeof(OBJSpriteSet));
            cursorMetatiles_2dc_ = (OBJSpriteSet::Metatile*)cursorObjectsAllocator_2d4_->Allocate(6 * sizeof(OBJSpriteSet::Metatile));
            cursorSpriteList_2e0_ = (OBJSpriteSet::List*)cursorObjectsAllocator_2d4_->Allocate(sizeof(OBJSpriteSet::List));
            if (cursorObjects_2d8_ != NULL && cursorMetatiles_2dc_ != NULL && cursorSpriteList_2e0_ != NULL)
            {
                cursorObjects_2d8_->Reset();
                for (unsigned short i = 0; i < 6; i++)
                    cursorMetatiles_2dc_[i].Reset();
                cursorSpriteList_2e0_->Reset();
                cursorObjects_2d8_->engine_ = 0;
                cursorObjects_2d8_->SetMetatiles(cursorMetatiles_2dc_, 6);
                cursorObjects_2d8_->sprites_ = cursorSpriteList_2e0_;
                unknown_2e4_ = 0;
                
                const char* filename;
                void* pacData;
                unsigned int pacLength;
                loader->GetLoadedFileByID(cursorObjectsLoadHandle_2d0_, &pacData, &pacLength);
                int fileCount = GetNumFilesInPAC(pacData);
                for (int i = 0; i < fileCount; i++)
                {
                    unsigned int subfileLength;
                    void* subfile = GetFileInPACByIndex(pacData, i, &filename, &subfileLength);
                    cursorObjects_2d8_->LoadFromNitroFile(subfile, subfileLength, cursorObjectsAllocator_2d4_);
                }
            }
            loader->RemoveTask(cursorObjectsLoadHandle_2d0_);
            cursorObjectsLoadHandle_2d0_ = -1;
            cursorObjectLoadStatus_2e7_++;
        }
    }
    else if (cursorObjectLoadStatus_2e7_ == 2)
    {
        if (cursorObjects_2d8_ == NULL || cursorSpriteList_2e0_ == NULL)
        {
            cursorObjects_2d8_ = NULL;
            cursorMetatiles_2dc_ = NULL;
            cursorSpriteList_2e0_ = NULL;
            unknown_2e4_ = 0;
            cursorObjectsLoadHandle_2d0_ = -1;
            cursorObjectLoadStatus_2e7_ = 0;
        }
    }
}

void TextManager::UpdatePendingMessages(unsigned int minTicks)
{
    InternalUpdatePendingMessages(minTicks);
}

struct TextBoxBounds
{
    short x, y, width, height;

    short Left() const { return x; }
    short Top() const { return y; }

    void Clear()
    {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
    }

    void SetBounds(short ax, short ay, short aw, short ah)
    {
        x = ax;
        y = ay;
        width = aw;
        height = ah;
    }
};

void DrawTextBoxBorderTile(UIElement* renderer, short x, short y, int idx)
{
    if (renderer == NULL)
        return;
    const unsigned char* tileBase = data_02107740.windowBorderTiles;
    unsigned int offset = idx * 32;
    tileBase += offset;
    renderer->BlitBitmap4Bit(tileBase, x, y, 8, 8, 0xf0, 0x0f);
}

void DrawTextBoxBorder2D(UIElement* renderer, const TextBoxBounds* bounds)
{
    if (renderer == NULL || bounds == NULL)
        return;
    // inline accessors necessary for register allocation
    short left = bounds->Left();
    short top = bounds->Top();
    short right = left + bounds->width;
    short bottom = top + bounds->height;
    top -= renderer->topTile_ae_ * 8;
    bottom -= renderer->topTile_ae_ * 8;
    right -= 8;
    bottom -= 8;
    if (left < 0 || top < 0 || bottom < 0 || right < 0) return;
    for (short x = left + 8; x < right; x += 8)
        DrawTextBoxBorderTile(renderer, x, top, 1);
    for (short x = left + 8; x < right; x += 8)
        DrawTextBoxBorderTile(renderer, x, bottom, 6);
    for (short y = top + 8; y < bottom; y += 8)
        DrawTextBoxBorderTile(renderer, left, y, 3);
    for (short y = top + 8; y < bottom; y += 8)
        DrawTextBoxBorderTile(renderer, right, y, 4);
    DrawTextBoxBorderTile(renderer, left, top, 0);
    DrawTextBoxBorderTile(renderer, right, top, 2);
    DrawTextBoxBorderTile(renderer, left, bottom, 5);
    DrawTextBoxBorderTile(renderer, right, bottom, 7);
}

void DrawTextBoxBackground(unsigned short* tileBuffer, const TextBoxBounds* bounds, int boxStyle)
{
    if (tileBuffer == NULL || bounds == NULL)
        return;

    if (bounds->width == 0 || bounds->height == 0)
        return;

    short left = bounds->Left();
    short top = bounds->Top();
    short right = left + bounds->width;
    short bottom = top + bounds->height;
    const unsigned char* tileLookup = data_020e7ade[boxStyle];
    left >>= 3;
    top >>= 3;
    right >>= 3;
    bottom >>= 3;
    short topRow = top * 32;
    short bottomRow = bottom * 32;
    *(tileBuffer + topRow + left) = tileLookup[0] - 0x13;
    *(tileBuffer + topRow + right) = tileLookup[1] - 0x13;
    *(tileBuffer + bottomRow + left) = tileLookup[2] - 0x13;
    *(tileBuffer + bottomRow + right) = tileLookup[3] - 0x13;
    for (short x = left + 1; x < right; x++) {
        *(tileBuffer + topRow + x) = tileLookup[4] - 0x13;
        *(tileBuffer + bottomRow + x) = tileLookup[5] - 0x13;
    }
    for (short y = top + 1; y < bottom; y++) {
        short row = y * 32;
        *(tileBuffer + row + left) = tileLookup[6] - 0x13;
        *(tileBuffer + row + right) = tileLookup[7] - 0x13;
    }
    for (short y = top + 1; y < bottom; y++) {
        short row = y * 32;
        for (short x = left + 1; x < right; x++) {
            *(tileBuffer + row + x) = 1;
        }
    }
}

void ClearGlyphTextureVRAM(TextureArena* arena)
{
    void* staging = AllocateVRAMStagingMemory(0x80 * 96);
    if (staging == NULL)
        return;
    
    unsigned int vramOffset = (unsigned short)arena->GetImageHandle() << 3;
    memset(staging, 0, 0x80 * 96);
    StageMemoryToVRAM(VRAMSubregion_TextureImage, staging, vramOffset, 0x80 * 96, true, false);
    FreeVRAMStagingMemory(staging);
}

void TextManager::RefreshGraphics(bool arg)
{
    if (unknown_19cc_ != 0)
    {
        int shakeX, shakeY;
        GetBoxShakeOffset(&shakeX, &shakeY);
        ClearUIElement(mainUIElement_34_);

        UIElement* ui = mainUIElement_34_;
        unsigned char* pixelBuffer = ui->pixelBuffer_8_;
        if (pixelBuffer != NULL)
        {
            ui->pixelBuffer_8_ = pixelBuffer; // wtf?
            unknown_19cc_ = 0;
            InternalDraw(true);
            unknown_19cc_ = 1;
            if (boxShakeActive_19c0_)
                unknown_40_ = 0;

            TextBoxBounds mainBoxBounds, speakerBoxBounds, optionBoxBounds;
            mainBoxBounds.Clear();
            speakerBoxBounds.Clear();
            optionBoxBounds.Clear();

            if ((unknown_998_ != 0 && showWindow_19b1_) || unknown_19cb_ != 0)
            {
                if (showWindow_19b1_)
                {
                    ConfigureMainTextBox(2, 116, 252, 74);
                    mainBoxBounds.SetBounds(shakeX + 2, shakeY + 116, 252, 74);
                    DrawTextBoxBorder2D(mainUIElement_34_, &mainBoxBounds);
                    mainBoxBounds.SetBounds(2, 116, 252, 74);
                }

                if (unknown_19c4_ != 0 && unknown_19c3_ != 0)
                {
                    TextBoxGraphics* substruct = &textBoxGraphics_19e0_;
                    substruct->activeBox_ = &speakerTextBox_930_;
                    // fix this with, most likely, an inline call
                    bool speakerBoxReady = (substruct->activeBox_ != NULL) ? (substruct->activeBox_->status == 2) : false;
                    if (speakerBoxReady)
                    {
                        short boxX, boxY, boxW, boxH;
                        if (substruct->activeBox_ != NULL)
                        {
                            boxX = substruct->activeBox_->x;
                            boxY = substruct->activeBox_->y;
                        }
                        if (substruct->activeBox_ != NULL)
                        {
                            boxW = substruct->activeBox_->width;
                            boxH = substruct->activeBox_->height;
                        }
                        speakerBoxBounds.SetBounds(boxX + shakeX, boxY + shakeY, boxW, boxH);
                        DrawTextBoxBorder2D(mainUIElement_34_, &speakerBoxBounds);
                        speakerBoxBounds.SetBounds(boxX, boxY, boxW, boxH);
                    }
                    substruct->activeBox_ = &mainTextBox_914_;
                }

                if (pauseType_9a0_ == 6 && unknown_19ba_ != 0 && dialogueOptions_90_.unknown_233_ != 0)
                {
                    optionBoxBounds.SetBounds(dialogueOptions_90_.textBox_b4_.x,
                        dialogueOptions_90_.textBox_b4_.y,
                        dialogueOptions_90_.textBox_b4_.width,
                        dialogueOptions_90_.textBox_b4_.height);
                    DrawTextBoxBorder2D(mainUIElement_34_, &optionBoxBounds);
                }
            }

            UIElement* elt = mainUIElement_34_;
            if (elt != NULL)
            {
                short i;
                short tileX = elt->leftTile_ac_;
                short tileY = elt->topTile_ae_;
                short numTilesX = elt->numTilesX_a8_;
                int vramOffset = (tileX + 32 * tileY) * 32;
                const unsigned char* source;
                short numTilesY = elt->numTilesY_aa_;
                source = elt->pixelBuffer_8_;
                for (i = 0; i < numTilesY; i++)
                {
                    int rowSize = numTilesX * 32;
                    StageMemoryToVRAM(VRAMSubregion_MainBG3Character, source, vramOffset, rowSize, true, false);
                    source += rowSize;
                    vramOffset += 32 * 32;
                }
            }

            int oldShakeX = unknown_94c_;
            int oldShakeY = unknown_950_;
            BG2OFS = ((-oldShakeX) & 0x1ff) | (((-oldShakeY) << 16) & 0x1ff0000);
            unknown_94c_ = shakeX;
            unknown_950_ = shakeY;

            if (pauseType_9a0_ != 1 && pauseType_9a0_ != 0)
            {
                memset(data_02107740.stagingSpace->backgroundIndexBuffer, 0, 0x800);
                if (((unknown_998_ != 0 && showWindow_19b1_) || unknown_19cb_))
                {
                    if (showWindow_19b1_)
                    {
                        unsigned short* windowTiles = data_02107740.stagingSpace->backgroundIndexBuffer;
                        DrawTextBoxBackground(windowTiles, &mainBoxBounds, 0);
                        DrawTextBoxBackground(windowTiles, &speakerBoxBounds, 1);
                        DrawTextBoxBackground(windowTiles, &optionBoxBounds, 2);
                        StageMemoryToVRAM(VRAMSubregion_MainBG2Screen, windowTiles, 0, 0x800, true, false);
                    }
                }

                if (!showWindow_19b1_)
                    StageMemoryToVRAM(VRAMSubregion_MainBG2Screen,
                        data_02107740.stagingSpace->backgroundIndexBuffer, 0,
                        sizeof(data_02107740.stagingSpace->backgroundIndexBuffer),
                        true, false);
                
                if (unknown_74_ == unknown_78_ || unknown_998_ == 0)
                {
                    StagingSpace* indexData = data_02107740.stagingSpace;
                    for (unsigned short i = 0; i < 0x100; i++)
                        indexData->textBorderIndexBuffer[i] = 0;
                    for (unsigned short i = 0x100; i < 0x300; i++)
                        indexData->textBorderIndexBuffer[i] = i;
                    StageMemoryToVRAM(VRAMSubregion_MainBG3Screen, indexData->textBorderIndexBuffer, 
                        0, sizeof(indexData->textBorderIndexBuffer), true, false);
                }
            }
            // switch off BG1 display, switch the others on
            DISPCNT = (DISPCNT & ~0x1f00) | 0x1d00;
        }

        FreeVRAMStagingMemory(mainUIElement_34_->pixelBuffer_8_);

        if (unknown_998_ == 0)
        {
            // equivalent to calling Close() and inlining it
            DestroyRenderGlyphs();
            ResetTextBGLayers();
            ResetBoxes();
        }
    }

    if (unknown_19ce_ && unknown_44_ == 0)
        ClearGlyphTextureVRAM(textureArena_1e28_);

    for (int i = 0; i < 2; i++)
        clipWindows_[i].SetEnabled(false);

    if (arg && unknown_998_ == 0)
        return;

    if (textureArena_1e28_ != NULL)
    {
        RenderGlyph* rglyph;
        for (int i = 0; i < 0x80; i++)
        {
            if (glyphRefcounts_17b8_[i] == 0)
                continue;
            rglyph = &glyphs_[i];
            if (!rglyph->unk_16_low)
                continue;
            if (unknown_19cc_ != 0 || unknown_19ce_)
                rglyph->unk_16_low = false;
            else
            {
                textureArena_1e28_->Restore();
                rglyph->UploadToVRAM();
                textureArena_1e28_->Save();
            }
            MaybeReleaseGlyphVRAM(substruct_2cc_, rglyph->bitAllocationIndex);
        }
    }

    if (mainTextBox_914_.status >= 2)
    {
        clipWindows_[0].SetBoundsXYWH(4, 118, 248, 70);
        if (unknown_1962_ != 0)
            clipWindows_[0].SetEnabled(true);
    }

    if (dialogueOptions_90_.textBox_b4_.status >= 2)
    {
        clipWindows_[1].SetBoundsXYWH(dialogueOptions_90_.textBox_b4_.x + 2,
            dialogueOptions_90_.textBox_b4_.y + 2,
            dialogueOptions_90_.textBox_b4_.width - 4,
            dialogueOptions_90_.textBox_b4_.height - 4);
        if (unknown_1962_ != 0)
            clipWindows_[1].SetEnabled(true);
    }

    for (int i = 0; i < 2; i++)
        clipWindows_[i].Apply();
}

void TextManager::CreateTextBoxGraphics()
{
    textBoxGraphics_19e0_.Create();
}

void TextManager::LoadTextBoxGraphics(SafeAllocator *alloc, const void *data)
{
    textBoxGraphics_19e0_.LoadFromFile(alloc, data);
}

bool TextManager::TryAdvanceMessage()
{
    if (unknown_998_ == 0)
        return true;
    if (boxShakePending_19c1_ == 0 && boxShakeActive_19c0_)
        return false;

    unsigned short allowedKeys = KEY_X | KEY_A | KEY_B;
    if (unknown_19bf_ != 0)
        allowedKeys |= KEY_L;
    if (unknown_19cf_ != 0)
        allowedKeys |= KEY_R;

    bool anyInput = false;
    if (g_keyManager.TestKeysPressed(allowedKeys))
        anyInput = true;
    if (g_keyManager.TestKeysPressed(KEY_UP | KEY_DOWN | KEY_LEFT | KEY_RIGHT))
        anyInput = true;
    if (g_touchInput.released_ && g_touchInput.prevTimeHeld_ < 20)
        anyInput = true;

    if (GetNextWord() == ControlWord_PageT)
    {
        unsigned int dt = GameState::GetInstance()->GetTickCount();
        if (pageTRemainingTime_1961_ != 0)
        {
            if (pageTRemainingTime_1961_ <= dt)
            {
                pageTRemainingTime_1961_ = 0;
                anyInput = true;
            }
            else
                pageTRemainingTime_1961_ -= dt;
        }
    }

    if (!anyInput)
        return false;

    switch (pauseType_9a0_)
    {
    case 0: // waiting for A, with cursor
        if (unknown_19be_)
            return false;
        if (GetNextWord() == ControlWord_EndRTurn)
        {
            void* npc = func_0203dce4(func_0203cf4c(), npcID_1838_);
            if (npc != NULL)
                func_02040af0(npc, npcPriorRotation_183c_);
        }
        return true;
    case 1: // waiting for A, no cursor
        cursorBlinkTimer_19ae_ = 0;
        cursorVisibleThisFrame_19af_ = false;
        unknown_19cd_ = 1;
        unknown_40_ = 0;
        unknown_44_ = 0;
        voiceSoundEffectTimer_912_ = 0;
        ReleaseMainMessageGlyphs();
        pauseType_9a0_ = 5;
        LoadNextPage();
        if (unknown_19d3_ == 0)
            func_0205eaa0(&data_EN_02108760, 1, NULL);
        break;
    case 3:
        unknown_19bd_ = 0;
        break;
    case 2: // pad-wait or pad-wait-no-cursor
        if (GetNextWord() == ControlWord_PadWaitNoCursor)
        {
            while (unknown_78_ < glyphIndexListLength_6c_)
            {
                unsigned short nextWord = glyphIndexList_4c_[unknown_78_];
                if (IsNextCharacterControlAlt(&nextWord))
                {
                    int numExtraWords = GetControlWordNumExtraWords(&nextWord);
                    if (numExtraWords != 0)
                    {
                        unknown_78_ += numExtraWords;
                        unknown_78_++;
                        continue;
                    }
                    if (nextWord == ControlWord_End || nextWord == ControlWord_Close)
                        break;
                    if (nextWord == ControlWord_RTurn)
                    {
                        npcAngleToUse_1844_ = npcPriorRotation_183c_;
                        npcTurnInProgress_19b6_ = true;
                        UpdateNPCTurn(1);
                    }
                    else if (nextWord == ControlWord_SlashQuest)
                    {
                        if (unknown_8c_ != NULL)
                            func_0208beb4(unknown_8c_);
                    }
                }
                unknown_78_++;
            }
            StopVoiceSoundEffect();
            pauseType_9a0_ = 0;
            Cleanup();
        }
        else
        {
            if (!unknown_19d3_)
                func_0205eaa0(&data_EN_02108760, 1, 0);
            ResumeTextScroll();
            voiceSoundEffectTimer_912_ = 0;
        }
        break;
    }
    return false;
}

int TextManager::GetControlWordNumExtraWords(const void* text) const
{
    int numExtraWords = 0;

    unsigned short word;
    memcpy(&word, text, 2);
    if (!IsNextCharacterControlAlt(&word))
        return numExtraWords;
    switch (word)
    {
    case ControlWord_Unknown_ff20:
    case ControlWord_Unknown_ff21:
    case ControlWord_Unknown_ff23:
    case ControlWord_Unknown_ff24:
        numExtraWords = 1;
        break;
    case ControlWord_Unknown_ff22:
    case ControlWord_Unknown_ff25:
        numExtraWords = 2;
        break;
    }
    return numExtraWords;
}

bool TextManager::IsNextCharacterControlAlt(const void* text) const
{
    unsigned short word;
    memcpy(&word, text, 2);
    if ((word & 0xff00) == 0xff00)
        return true;
    return false;
}

void TextManager::ResumeTextScroll()
{
    pauseType_9a0_ = 5;
    unknown_78_++;
}

void TextManager::Update(unsigned int numTicks)
{
    InternalUpdate(numTicks);
}

void TextManager::Draw()
{
    if (objUI_38_ != NULL)
    {
        if (objUI_38_->BeginRender() != NULL)
        {
            InternalDraw(false);
            objUI_38_->EndRender();
            objUI_38_->FreeStagingMemory();
        }
    }
    else
        InternalDraw(false);
}

void TextManager::DrawDialogueOptions(bool use2D)
{
    if (pauseType_9a0_ == 6 && unknown_19ba_ != 0)
        dialogueOptions_90_.Draw(use2D);
}

void GetUIElementTopRight(const UIElement* elt, short* outX, short* outY)
{
    if (elt == NULL)
        return;
    short width = elt->numTilesX_a8_;
    short left = elt->leftTile_ac_;
    short height = elt->numTilesY_aa_;
    width *= 8;
    left *= 8;
    height *= 8;
    *outX = left + (width >> 1);
    *outY = height - 10;
}

void TextManager::DrawBlinkingCursor(bool use2D)
{
    if (pauseType_9a0_ == 1 || pauseType_9a0_ == 3 || pauseType_9a0_ == 2)
    {
        if (!cursorPresent_19ca_)
            return;
        if (use2D)
        {
            short x, y;
            GetUIElementTopRight(mainUIElement_34_, &x, &y);
            int left = x - 3;
            int top = y;
            int right = x + 4;
            int bottom = y + 4;
            mainUIElement_34_->FillRectangle2(0, left, top, right, bottom);
        }
    }
    if (cursorVisibleThisFrame_19af_ != true)
        return;
    
    if (use2D)
    {
        short baseX, baseY;
        GetUIElementTopRight(mainUIElement_34_, &baseX, &baseY);
        if (cursorBlinkTimer_19ae_ > 15)
        {
            for (short scanline = 0; scanline < 4; scanline++)
                mainUIElement_34_->FillRectangle2(15, 
                    baseX - (3 - scanline), baseY + scanline,
                    baseX + (4 - scanline), baseY + scanline + 1);
        }
        return;
    }

    short boxWidth = mainTextBox_914_.width;
    short boxHeight = mainTextBox_914_.height;
    int cursorXMiddle = mainTextBox_914_.x + (boxWidth >> 1);
    int cursorYBottom = mainTextBox_914_.y + boxHeight - 3;
    if (objUI_38_ != NULL)
    {
        if (cursorBlinkTimer_19ae_ > 15)
        {
            int cursorTop = boxHeight - 8;
            for (short scanline = 0; scanline < 4; scanline++)
                objUI_38_->FillRectangle(15,
                (boxWidth >> 1) - (3 - scanline), cursorTop + scanline, 
                (4 - scanline) + (boxWidth >> 1), cursorTop + scanline + 1);
        }
    }
    else
    {
        GXFIFO_MATRIX_PUSH = 0;
        GXFIFO_VERTEX_COLOR = 0x7fff;
        // texture size 16x16
        GXFIFO_TEXIMAGE_PARAMS = (1 << 20) | (1 << 23);
        // show front & back, alpha = 31, polygon ID 0x3e = 62
        GXFIFO_POLYGON_ATTRIBUTES = 0x3e1f00c0;
        GXFIFO_MATRIX_PUSH = 0;
        GXFIFO_MATRIX_TRANSLATE = cursorXMiddle << 12;
        GXFIFO_MATRIX_TRANSLATE = cursorYBottom << 12;
        GXFIFO_MATRIX_TRANSLATE = MAKEFIX32(1024);
        GXFIFO_MATRIX_SCALE = MAKEFIX32(6);
        GXFIFO_MATRIX_SCALE = MAKEFIX32(6);
        GXFIFO_MATRIX_SCALE = MAKEFIX32(1);
        GXFIFO_POLYGON_BEGIN = 0;
        GXFIFO_VERTEX_16 = 0 | 0;
        GXFIFO_VERTEX_16 = 0;
        GXFIFO_VERTEX_16 = (0xf000 << 16) | 0xf000;
        GXFIFO_VERTEX_16 = 0;
        GXFIFO_VERTEX_16 = (0xf000 << 16) | 0x1000;
        GXFIFO_VERTEX_16 = 0;
        GXFIFO_POLYGON_END = 0;
        GXFIFO_MATRIX_POP = 1;
        GXFIFO_MATRIX_POP = 1;
    }
}

void TextManager::GetLineCenteringOffsets(int *outXs, int *outYs) const
{
    InternalGetLineCenteringOffsets(outXs, outYs);
}

void WriteControlWord(char**, int control);

void TextManager::ConvertBoxModifierTags(const char *in, char *out)
{
    if (in == NULL || out == NULL)
        return;

    in = HandleImmediateBoxTags(in);
    while (true)
    {
        if (*in == 0)
            break;
        char ch0 = in[0];
        char ch1 = in[1];
        if ((ch0 == '\\' && ch1 == 'n') || (ch0 == '\r' && ch1 == '\n'))
        {
            WriteControlWord(&out, ControlWord_LineBreak);
            in += 2;
            continue;
        }
        if (ch0 == '\n')
        {
            WriteControlWord(&out, ControlWord_LineBreak);
            in += 1;
            continue;
        }
        if (ch0 == '<')
        {
            bool processedTag = true;
            const char* tagEnd = strchr(in, '>');
            if (tagEnd == NULL)
                break;
            const char* afterTagEnd = tagEnd + 1;
            if (CaseInsensitiveDoesStringBeginWith(in + 1, data_EN_020f00a3))
            {
                int timeArg = func_02005a94(in + 6);
                if (timeArg < 0)
                    timeArg = 0;
                RecordWaitTimeValue(timeArg);
                WriteControlWord(&out, ControlWord_Time);
            }
            else
                processedTag = false;

            if (processedTag)
            {
                in = afterTagEnd;
                continue;
            }
        }

        *out++ = *in++;
    }
    WriteControlWord(&out, ControlWord_End);
}

void WriteControlWord(char** ppText, int control)
{
    unsigned short u16ctrl = control;
    *((*ppText)++) = u16ctrl;
    *((*ppText)++) = u16ctrl >> 8;
}

const char* TextManager::HandleImmediateBoxTags(const char* in)
{
    bool moreToGo = true;
    while (moreToGo)
    {
        if (CaseInsensitiveDoesStringBeginWith(in, data_EN_020f00a9))
        {
            bool prematureEnd = false;
            int readOffset = 6;
            int tagLength = 6;
            while (in[tagLength] != '>')
            {
                if (in[tagLength] == 0)
                    return in + tagLength;
                tagLength++;
            }

            int rectX = func_02005a94(in + readOffset);
            while (in[readOffset] != ',')
            {
                if (in[readOffset] == '>')
                {
                    in += readOffset + 1;
                    prematureEnd = true;
                    break;
                }
                readOffset++;
            }
            if (prematureEnd)
                return in;

            readOffset++;
            int rectY = func_02005a94(in + readOffset);
            while (in[readOffset] != ',')
            {
                if (in[readOffset] == '>')
                {
                    in += readOffset + 1;
                    prematureEnd = true;
                    break;
                }
                readOffset++;
            }
            if (prematureEnd)
                return in;

            readOffset++;
            int rectW = func_02005a94(in + readOffset);
            while (in[readOffset] != ',')
            {
                if (in[readOffset] == '>')
                {
                    in += readOffset + 1;
                    prematureEnd = true;
                    break;
                }
                readOffset++;
            }
            if (prematureEnd)
                return in;

            readOffset++;
            int rectH = func_02005a94(in + readOffset);

            if (rectX < 0)
                rectX = 0;
            if (rectX > 256)
                rectX = 256;
            if (rectY < 0)
                rectY = 0;
            if (rectY > 192)
                rectY = 192;
            if (rectW < 0)
                rectW = 256 - rectX;
            if (rectW > 256)
                rectW = 256 - rectX;
            if (rectH < 0)
                rectH = 192 - rectY;
            if (rectH > 192)
                rectH = 192 - rectY;
            ConfigureMainTextBox(rectX, rectY, rectW, rectH);
            in += tagLength + 1;
        }
        else if (CaseInsensitiveDoesStringBeginWith(in, data_EN_020f00b0))
        {
            showWindow_19b1_ = false;
            in += 5;
        }
        else if (CaseInsensitiveDoesStringBeginWith(in, data_EN_020f00b6))
        {
            cenTag_19b8_ = true;
            in += 5;
        }
        else if (CaseInsensitiveDoesStringBeginWith(in, data_EN_020f00bc))
        {
            int readOffset = 6;
            int tagLength = 6;
            while (in[tagLength] != '>')
            {
                if (in[tagLength] == 0)
                    return in + tagLength;
                tagLength++;
            }
            int mojiArg1 = func_02005a94(in + readOffset);
            if (mojiArg1 <= 0)
                mojiArg1 = 12;
            while (in[readOffset] != ',')
            {
                if (in[readOffset] == '>')
                {
                    in += readOffset + 1;
                    break;
                }
                readOffset++;
            }
            readOffset++;
            int mojiArg2 = func_02005a94(in + readOffset);
            if (mojiArg2 <= 0)
                mojiArg2 = 16;
            mojiArg1_1860_ = mojiArg1;
            mojiArg2_1864_ = mojiArg2;
            in += tagLength + 1;
        }
        else if (CaseInsensitiveDoesStringBeginWith(in, data_EN_020f00c3))
        {
            skipTag_19bb_ = false;
            in += 6;
        }
        else
            moreToGo = false;
    }
    return in;
}

void TextManager::ClearUIElement(UIElement* element)
{
    element->Reset();
    unsigned char* allocation = (unsigned char*)AllocateVRAMStagingMemory(0x4800);
    if (allocation == NULL)
        return;
    if (unknown_19cd_)
    {
        unknown_19cd_ = false;
        memset(allocation, 0, 0x4800);
    }
    element->leftTile_ac_ = 0;
    element->topTile_ae_ = 6;
    element->numTilesX_a8_ = 32;
    element->numTilesY_aa_ = 18;
    element->pixelBuffer_8_ = allocation;
    element->bufferSize_a4_ = 0x4800;
    element->vramOffset_a0_ = 0;
    element->borderPalette_c2_ = 0;
    element->textXOffset_b0_ = 0;
    element->textYOffset_b2_ = 0;
    element->unknown_b4_ = 8;
    element->lineSpacing_b6_ = 8;
    element->unknown_b8_ = 8;
    element->overrideLineSpacing_ba_ = 8;
}

static void ReloadBGLayers()
{
    // enable obj and BG3, disable the rest
    DISPCNT = (DISPCNT & ~0x1f00) | 0x1100;
    BG2OFS = 0;
    BG3OFS = 0;
    // preserve priority and mosaic, set screen base block = 29, rest 0
    BG1CNT = (BG1CNT & 0x43) | 0x1d00;
    BG2CNT = (BG2CNT & 0x43) | 0x1e00; // screen base block = 30
    BG3CNT = (BG3CNT & 0x43) | 0x1f08; // screen base block = 31, char base block 2

    StagingSpace* stagingSpace = data_02107740.stagingSpace;
    const void* windowGraphics = GetGameResources()->paletteData_2c;

    BGLayer layer;
    layer.Reset();
    layer.engine_ = 0;
    layer.bgIndex_ = 3;
    layer.ProcessGraphicsFile(windowGraphics, NULL, 0);

    if (stagingSpace != NULL)
    {
        memset(stagingSpace, 0, 0x1000);
        for (unsigned char i = 0; i < 6; i++)
            StageMemoryToVRAM(VRAMSubregion_MainBG3Character, stagingSpace, 0x1000 * i, 0x1000, true, false);
        
        const void* windowBackgroundData = data_02107740.windowBorderTiles + 0x280;
        if (windowBackgroundData != NULL)
            StageMemoryToVRAM(VRAMSubregion_MainBG2Character, windowBackgroundData, 0x20, 0x900, true, false);
    }

    if (stagingSpace != NULL)
    {
        memset(stagingSpace, 0, 0x1000);
        StageMemoryToVRAM(VRAMSubregion_MainBG2Screen, stagingSpace, 0, 0x800, true, false);
        unsigned short* screenBuffer = stagingSpace->textBorderIndexBuffer;
        for (unsigned short i = 0; i < 0x300; i++)
        {
            *screenBuffer = i;
            screenBuffer++;
        }
        StageMemoryToVRAM(VRAMSubregion_MainBG3Screen, stagingSpace, 0, 0x800, true, false);
        void* bg1Base = (void*)GetMainBG1ScreenBase();
        if (bg1Base != NULL)
            memset(bg1Base, 0, 0x800);
    }

    // blend between BG2 (box background) and BG0 (3D engine) in a 10:6 ratio
    ColorEffect_ConfigureAlphaBlend(ADDR_BLDCNT, 1 << 2, 1 << 0, 10, 6);
    // set priority, BG3 at the top (priority 0 is highest)
    BG0CNT = (BG0CNT & ~3) | 3;
    BG1CNT = (BG1CNT & ~3) | 2;
    BG2CNT = (BG2CNT & ~3) | 1;
    BG3CNT = (BG3CNT & ~3) | 0;
}

void TextManager::AddMessage(const char* msg, int graphicsMode)
{
    unknown_19bd_ = 1;
    ProcessTextContinuation(msg);
    if (!unknown_19cc_ && graphicsMode == 1) // 2d
    {
        cursorBlinkTimer_19ae_ = 0;
        cursorVisibleThisFrame_19af_ = false;
        unknown_19cd_ = true;
        unknown_40_ = 0;
        ReloadBGLayers();
    }
    unknown_19cc_ = false;
    unknown_19ce_ = false;
    if (graphicsMode == 1)
        unknown_19cc_ = true;
    else if (graphicsMode == 1) // ...
        unknown_19ce_ = true;
    if (unknown_19cc_ != 0)
    {
        unknown_19cc_ = true;
        cursorBlinkTimer_19ae_ = 0;
        cursorVisibleThisFrame_19af_ = false;
        unknown_19cd_ = true;
        unknown_40_ = 0;
    }
    if (unknown_19ce_)
    {
        unknown_44_ = 0;
        ClearGlyphTextureVRAM(textureArena_1e28_);
    }
    maybePartySize_30_ = -1;
    maybeNumAlive_31_ = -1;
}

extern const int data_020e7a94[];

void TextManager::ShowMessage(const char *msg, int graphicsMode, int width)
{
    GameState* gs = GameState::GetInstance();
    short probablyItemID = gs->unknown_7f58_;
    Noun itemNoun;
    itemNoun.Reset();
    if (probablyItemID > 0)
    {
        char singular[0x80] = {0};
        char plural[0x80] = {0};
        itemNoun.singularName_ = singular;
        itemNoun.pluralName_ = plural;
        func_020dcf7c(probablyItemID, &itemNoun);
        items_18_[0] = &itemNoun;
    }
    fontIndex_19dc_ = 1;
    if (msg == NULL)
        return;

    // what's the point of this?!
    int length = func_020d4a54(msg);
    for (int i = 0; i < length; )
    {
        int copyLength = 0x40;
        char scratch[67] = {0};
        if (length < i + 0x40)
            copyLength = length - i;
        memcpy(scratch, msg + i, copyLength);
        i += copyLength;
    }

    for (int i = 0; i < 4; i++)
    {
        tagSTArray1_194e_[i] = -1;
        tagSTArray2_1952_[i] = 0;
    }
    tagSTArrayIndex_1956_ = 0;
    unknown_1957_ = 0;
    unknown_19d2_ = 1;
    unknown_19d1_ = 0;
    unknown_1958_ = 0;
    effects_195b_ = 0;
    fadeRescaledAlpha_9b0_ = 0;
    unknown_195a_ = 8;

    if (pauseType_9a0_ == 3)
        AddMessage(msg, graphicsMode);
    else
    {
        unsigned short indexBufferLength = unknown_1942_;
        for (unsigned short i = 0; i < indexBufferLength; i++)
            ReleaseGlyphByIndex(indexBuffer_1882_[i]);

        // bug? this writes 0x01 and not 0xff01
        memset(indexBuffer_1882_, 0xff01, sizeof(indexBuffer_1882_));
        memset(buffer_1902_, 0, sizeof(buffer_1902_));
        unknown_19c4_ = 0;
        unknown_19c3_ = 0;
        unknown_1942_ = 0;
        speakerNameWidth_1944_ = 0;
        unknown_1946_ = 0;
        speakerDifference_19c2_ = true;
        unknown_19d3_ = 0;
        unknown_19b0_ = 0;
        unknown_19bf_ = 1;
        unknown_19cf_ = 1;
        voiceVolumeLookup_194a_[0] = voiceVolumeLookup_194a_[1] =
            voiceVolumeLookup_194a_[2] = voiceVolumeLookup_194a_[3] = 127;
        func_0205e944(data_EN_02108760, 127);
        memset(substruct_2cc_, 0, sizeof(substruct_2cc_));
        memset(fullMessage_48_, 0, 0x960);
        memset(glyphIndexList_4c_, 0, 0x280);
        memset(currentPageMessage_50_, 0, 0x280);
        unknown_19bd_ = 1;
        numGlyphsInUse_9b4_ = 0;
        for (int i = 0; i < 0x80; i++)
        {
            glyphs_[i].Reset2();
            glyphRefcounts_17b8_[i] = 0;
        }
        skipTag_19bb_ = 1;
        mojiArg1_1860_ = 12;
        mojiArg2_1864_ = 16;
        color_187c_ = 0x7fff;
        gyouParameter_9a4_ = 3;
        ConfigureMainTextBox(2, 128, 252, 62);
        speakerTextBox_930_.Reset();
        cenTag_19b8_ = 0;
        showWindow_19b1_ = 1;
        centerX_990_ = 128;
        unknown_1868_ = 0;
        voiceSoundEffectTimer_912_ = 0;
        unknown_994_ = -1;
        unknown_94c_ = 0;
        unknown_950_ = 0;
        chosenAnswer_954_ = 0;
        unknown_958_ = 0;
        unknown_19ac_ = 0;
        waitTimeQueueIndex_19ad_ = 0;
        pageTValue_1960_ = 0;
        RecordWaitTimeValue(0, -1);
        boxShakeActive_19c0_ = false;
        boxShakeTimer_195d_ = 30;
        boxShakePending_19c1_ = 0;
        recoveryParam_19c6_ = 0;
        recoveryParam_19c7_ = 0;
        unknown_19c8_ = 0;
        unknown_19c9_ = 0;
        recoveryAmount_187e_ = 0;
        unknown_19cc_ = 0;
        unknown_19ce_ = false;
        unknown_19cd_ = 0;
        if (graphicsMode == 1)
            unknown_19cc_ = 1;
        else if (graphicsMode == 1)
            unknown_19ce_ = true;

        if (unknown_19cc_)
        {
            unknown_19cd_ = 1;
            unknown_40_ = 0;
            ReloadBGLayers();
        }

        if (unknown_19ce_)
        {
            unknown_44_ = 0;
            ClearGlyphTextureVRAM(textureArena_1e28_);
        }

        char* tempBufferA = pageScratchBuffer_60_;
        char* tempBufferB = scratchBuffer_64_;
        SubstituteValueTags(msg, tempBufferA);
        SubstituteUnsupportedCharacters(tempBufferA, tempBufferB, fontIndex_19dc_);
        SubstituteConditionals(tempBufferB, tempBufferA, fontIndex_19dc_);
        SubstituteNamesAndArticles(tempBufferA, tempBufferB, fontIndex_19dc_);
        SubstituteCaps(tempBufferB, tempBufferA, fontIndex_19dc_);
        SubstituteControlTags(tempBufferA, tempBufferB);
        LoadFirstPage(tempBufferB, width);
        ConfigureMainTextBox(2, 116, 252, 74);
        unknown_1858_ = 2;
        unknown_185c_ = 126;
        mojiArg1_1860_ = 12;
        mojiArg2_1864_ = 20;
        if (objUI_38_ != NULL)
        {
            OBJTextUI::OBJGraphics* gfx = objUI_38_->GetGraphics();
            if (gfx != NULL)
            {
                gfx->baseX = 2;
                gfx->baseY = 116;
                gfx->width = 252;
                gfx->height = 74;
                objUI_38_->CreateInVRAM(-1);
                if (objUI_38_->BeginRender() != NULL)
                {
                    objUI_38_->DrawTextBoxBorder();
                    objUI_38_->EndRender();
                    objUI_38_->FreeStagingMemory();
                }
            }
        }
        memset(substructOrArray_95c_, 0, sizeof(substructOrArray_95c_));
        memset(substructOrArray_97c_, 0, sizeof(substructOrArray_97c_));
        unknown_994_ = -1;
        unknown_19bc_ = 0;
        pauseType_9a0_ = 5;
        unknown_19b2_ = 1;
        speakerVoiceType_99c_ = 0;
        cursorBlinkTimer_19ae_ = 0;
        cursorVisibleThisFrame_19af_ = 0;
        unknown_19b5_ = 0;
        unknown_19c5_ = 0;
        cursorPresent_19ca_ = true;
        unknown_19b4_ = 0;
        for (int i = 0; i < 2; i++)
            clipWindows_[i].SetEnabled(false);
        if (textureArena_1e28_ != NULL)
            textureArena_1e28_->Reset(); 
    }
    maybePartySize_30_ = -1;
    maybeNumAlive_31_ = -1;
    if (probablyItemID > 0)
        gs->unknown_7f58_ = 0;
}

void TextManager::PlayVoiceSoundEffect()
{
    if (voicePlaybackState_19b3_ == 1 || unknown_19b2_ == 0)
        return;
    switch (speakerVoiceType_99c_)
    {
    case 0: // default / neutral pitch
        func_0205eaa0(data_EN_02108760, 10, data_02107740.unk_20);
        break;
    case 1: // male
        func_0205eaa0(data_EN_02108760, 12, data_02107740.unk_20);
        break; 
    case 2: // female
        func_0205eaa0(data_EN_02108760, 11, data_02107740.unk_20);
        break;
    }
    voicePlaybackState_19b3_ = 1;
}

void TextManager::StopVoiceSoundEffect()
{
    if (voicePlaybackState_19b3_ == 0 || unknown_19b2_ == 0)
        return;
    func_0205eabc(data_EN_02108760, data_02107740.unk_20, 0);
    voicePlaybackState_19b3_ = 0;
}

void TextManager::UpdateCursorBlink()
{
    if (pauseType_9a0_ == 1 || pauseType_9a0_ == 3 || pauseType_9a0_ == 2)
    {
        if (cursorPresent_19ca_)
        {
            cursorBlinkTimer_19ae_++;
            cursorBlinkTimer_19ae_ &= 0x1f;
            cursorVisibleThisFrame_19af_ = cursorBlinkTimer_19ae_ > 15;
        }
    }
    else
    {
        cursorBlinkTimer_19ae_ = 0;
        cursorVisibleThisFrame_19af_ = false;
    }
}

void TextManager::HandleUnknownInput_02045740()
{
    unknown_958_++;
    unknown_958_ &= 0x1f;
    if (g_keyManager.TestKeysPressed(KEY_UP))
    {
        chosenAnswer_954_--;
        if (chosenAnswer_954_ < 0)
            chosenAnswer_954_ = gyouParameter_9a4_ - 1;
        unknown_994_ = chosenAnswer_954_;
        unknown_958_ = 0;
    }
    else if (g_keyManager.TestKeysPressed(KEY_DOWN))
    {
        chosenAnswer_954_++;
        if (chosenAnswer_954_ >= gyouParameter_9a4_)
            chosenAnswer_954_ = 0;
        unknown_994_ = chosenAnswer_954_;
        unknown_958_ = 0;
    }
}

int TextManager::GetChosenAnswer() const
{
    return chosenAnswer_954_;
}

void TextManager::RecordWaitTimeValue(int value, int slot)
{
    if (slot == -1)
    {
        // bug: writes to index -1 each time?!
        for (int i = 0; i < 4; i++)
            waitTimeLengths_186c_[slot] = value;
    }
    else if (slot < 0)
    {
        for (int i = 0; i < 4; i++)
            waitTimeLengths_186c_[i] = value;
    }
    else if (slot >= 4)
    {
        for (int i = 0; i < 4; i++)
            waitTimeLengths_186c_[i] = value;
    }
    else
        waitTimeLengths_186c_[slot] = value;
}

void BlitGlyphTo4BitPixelBuffer(int x, int y, const FontIndexFile::Glyph* glyph, unsigned char* dstBuffer,
    int bufferW, int bufferH, unsigned char color, unsigned char fontIndex)
{
    if (glyph == NULL || dstBuffer == NULL)
        return;
    int sourceOffsetBit = glyph->dataOffset;
    const FontDataFile* fontData = data_02107740.pFontDataFiles[fontIndex];
    unsigned char colorMaskHigh = color << 4;
    unsigned char colorMaskLow = color & 0x0f;
    int scanlineStride = fontData->bitsPerScanline >> 3;
    char initialBitShift = 7 - (sourceOffsetBit & 7);
    int dstBufferStride = bufferW >> 1;
    for (int srcY = 0; srcY < 12; srcY++)
    {
        char bitShift = initialBitShift;
        const unsigned char* readPtr = fontData->ptr_c + (scanlineStride * srcY) + (sourceOffsetBit >> 3);
        int dstY = y + srcY;
        int dstBufferYOffset = dstBufferStride * dstY;
        if (dstY < 0 || bufferH <= dstY)
            continue;

        for (int srcX = 0; srcX < glyph->width; srcX++)
        {
            int dstX = x + srcX;
            if (dstX < 0 || bufferW <= dstX)
                continue;
            if (*readPtr & (1 << bitShift))
            {
                if (dstX & 1)
                {
                    *(dstBuffer + dstBufferYOffset + (dstX >> 1)) &= 0x0f;
                    *(dstBuffer + dstBufferYOffset + (dstX >> 1)) |= colorMaskHigh;
                }
                else
                {
                    *(dstBuffer + dstBufferYOffset + (dstX >> 1)) &= 0xf0;
                    *(dstBuffer + dstBufferYOffset + (dstX >> 1)) |= colorMaskLow;
                }
            }
            bitShift--;
            if (bitShift < 0)
            {
                bitShift = 7;
                readPtr++;
            }
        }
    }
}

int TextManager::LoadNewGlyph(unsigned char glyphIndex, const FontIndexFile::Glyph *glyph, unsigned char fontIndex)
{
    if (glyph == NULL)
        return 0;
    
    GlyphPixelBuffer* pixelBufferArray = data_02107740.stagingSpace->glyphPixelData;
    unsigned char* refcountArray = glyphRefcounts_17b8_;
    RenderGlyph* renderGlyphs = glyphs_;
    int vramSlot = MaybeAcquireGlyphVRAM(substruct_2cc_, sizeof(substruct_2cc_));
    if (vramSlot < 0)
        return 0;

    for (int i = 0; i < 0x80; i++)
    {
        if (refcountArray[i] != 0)
            continue;
        if (textureArena_1e28_ != NULL)
        {
            GlyphPixelBuffer* chosenPixelBuffer = pixelBufferArray + vramSlot;
            RenderGlyph* targetRenderGlyph = renderGlyphs + i;
            bool b1 = false;
            bool b2 = false;
            if (unknown_19cc_ == 0)
            {
                memset(chosenPixelBuffer, 0, sizeof(GlyphPixelBuffer));
                if (effects_195b_ & 0x80)
                {
                    b2 = true;
                    for (unsigned char j = 0; j < 5; j++)
                        BlitGlyphTo4BitPixelBuffer(data_020e7aa6[j], data_020e7ab0[j],
                            glyph, chosenPixelBuffer->pixels, 16, 16, data_020e7a8c[j], fontIndex);
                }
                else if (effects_195b_ & 2)
                {
                    b1 = true;
                    BlitGlyphTo4BitPixelBuffer(2, 2, glyph, chosenPixelBuffer->pixels, 16, 16, 1, fontIndex);
                    BlitGlyphTo4BitPixelBuffer(1, 1, glyph, chosenPixelBuffer->pixels, 16, 16, 15, fontIndex);
                }
                else
                    BlitGlyphTo4BitPixelBuffer(1, 1, glyph, chosenPixelBuffer->pixels, 16, 16, 15, fontIndex);
            }
        
            targetRenderGlyph->Populate(glyphIndex, glyph, chosenPixelBuffer->pixels, vramSlot,
                textureArena_1e28_->GetImageVRAMOffset() + i * 128, fontIndex);

            if (b2)
                targetRenderGlyph->unk_16_2 = true;
            if (b1)
                targetRenderGlyph->unk_16_1 = true;
        }
        refcountArray[i] = 1;
        return i;
    }
    return 0;
}

int TextManager::GetGlyphIndex(const FontIndexFile::Glyph* glyph)
{
    if (glyph == NULL)
        return -1;
    for (int i = 0; i < 0x80; i++)
    {
        if (glyphRefcounts_17b8_[i] == 0)
            continue;
        RenderGlyph* rg = &glyphs_[i];
        if (glyph != rg->indexGlyph)
            continue;
        if (effects_195b_ & 0x80)
        {
            if (rg->unk_16_2)
                return i;
        }
        else if (effects_195b_ & 2)
        {
            if (rg->unk_16_1)
                return i;
        }
        else
            return i;
    }
    return -1;
}

void TextManager::ReleaseAllGlyphs()
{
    unknown_19b0_ = 1;
    numGlyphsInUse_9b4_ = 0;
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset2();
        glyphRefcounts_17b8_[i] = 0;
    }
    if (textureArena_1e28_ != NULL)
        textureArena_1e28_->Reset();
}

unsigned short TextManager::AcquireText16Bit(const char *text, unsigned short *outWords, unsigned char fontIndex)
{
    if (text == NULL || outWords == NULL)
        return 0;

    char transformedText[512] = {0};
    ConvertBoxModifierTags(text, transformedText);
    unsigned short counter = 0;
    int srcIndex = 0;
    int dstIndex = 0;
    if (textureArena_1e28_ != NULL)
        textureArena_1e28_->Restore();

    while (true)
    {
        if (dstIndex > 0 && outWords[dstIndex - 1] == ControlWord_End)
            break;
        unsigned short controlCopy, control;
        memcpy(&control, transformedText + srcIndex, 2);
        if ((control & 0xff00) == 0xff00)
        {
            memcpy(&controlCopy, transformedText + srcIndex, 2);
            outWords[dstIndex] = controlCopy;
            srcIndex += 2;
            dstIndex++;
            counter++;

            int numExtraWords = GetControlWordNumExtraWords(&controlCopy);
            memcpy(outWords + dstIndex, transformedText + srcIndex, numExtraWords * 2L);
            counter += numExtraWords;
            srcIndex += numExtraWords * 2;
            dstIndex += numExtraWords;
        }
        else
        {
            FontIndexFile::Glyph* glyph = GetNextFontGlyph(transformedText + srcIndex, fontIndex);
            int glyphIndex = GetNextFontGlyphIndex(transformedText + srcIndex, fontIndex);
            if (glyph != NULL && glyphIndex >= 0)
            {
                int resourceIndex = GetGlyphIndex(glyph);
                if (resourceIndex == -1)
                {
                    outWords[dstIndex] = LoadNewGlyph(glyphIndex, glyph, fontIndex);
                    numGlyphsInUse_9b4_++;
                    numGlyphsInUse_9b4_ &= 0x7f;
                }
                else
                {
                    unsigned short outputWord = resourceIndex;
                    outWords[dstIndex] = outputWord;
                    if (glyphRefcounts_17b8_[outputWord] < 0xff)
                        glyphRefcounts_17b8_[outputWord]++;
                }
                srcIndex += glyph->tagLength;
            }
            else
            {
                outWords[dstIndex] = ControlWord_Unknown_ff19;
                srcIndex++;
            }
            dstIndex++;
            counter++;
        }
    }
    if (textureArena_1e28_ != NULL)
        textureArena_1e28_->Save();
    return counter;
}

void TextManager::DrawPreprocessedText(const unsigned short* words, int x, int y, unsigned short color,
    int, int, bool use2D, unsigned char fontIndex, int lineSpacing)
{
    if (words == NULL || !unknown_19b0_ || words[0] == 0xffff)
        return;

    bool longRepeats = true;
    unsigned short firstWord = words[0];
    for (int j = 1; j < 10; j++)
    {
        if (firstWord != words[j])
        {
            longRepeats = false;
            break;
        }
    }

    if (longRepeats)
        return;

    GXFIFO_MATRIX_PUSH = 0;
    RenderGlyph* glyph;
    unsigned char lastGlyphIndex = 0xff;
    unsigned char defaultSpacing = data_020e7a94[fontIndex];
    int wordIdx = 0;
    int currentX = x;
    unsigned short thisWord = words[0];
    while (thisWord != ControlWord_End)
    {
        thisWord = words[wordIdx];
        wordIdx++;
        if ((thisWord & 0xff00) == 0xff00)
        {
            if (thisWord == ControlWord_LineBreak)
            {
                currentX = x;
                y += lineSpacing;
                lastGlyphIndex = 0xff;
                continue;
            }
            if (thisWord == ControlWord_Unknown_ff19)
            {
                currentX += defaultSpacing + 1;
                lastGlyphIndex = 0xff;
                continue;
            }
            break;
        }
        if (thisWord >= 0x80)
            break;
        glyph = &glyphs_[thisWord];
        if (glyph->bitAllocationIndex < 0)
            break;
        unsigned char thisGlyphIndex = glyph->indexInFontFile;
        currentX += GetFontGlyphKerning(lastGlyphIndex, thisGlyphIndex, fontIndex);
        lastGlyphIndex = thisGlyphIndex;
        glyph->drawX = currentX;
        glyph->drawY = y;
        glyph->drawZ = 0;
        if (use2D)
        {
            unsigned char paletteIndex = 15;
            if (color != 0x7fff)
                paletteIndex = 5;
            RenderGlyph2D(mainUIElement_34_, glyph, currentX, y, paletteIndex);
        }
        else
            glyph->Draw(color, NULL, 31);

        if (glyph->indexGlyph != NULL)
            currentX += glyph->indexGlyph->width + 1;
    }
    GXFIFO_MATRIX_POP = 1;
}

int TextManager::MeasureText16Bit(int argFontSize, const unsigned short *wordList, int numWords)
{
    if (wordList == NULL)
        return 0;
    if (numWords < 0)
        return 0;
    int fontSize;
    switch (argFontSize)
    {
    case 10: fontSize = 10; break;
    case 12: fontSize = 12; break;
    default: return 0;
    }

    int currentX = 0;
    int otherX = 0;
    for (int i = 0; i < numWords; i++)
    {
        unsigned short word = wordList[i];
        if (word == ControlWord_End)
            break;
        else if (word == ControlWord_LineBreak || word == ControlWord_Add ||
            word == ControlWord_Time || word == ControlWord_PadWait || 
            word == ControlWord_PadT || word == ControlWord_PadWaitNoCursor ||
            word == ControlWord_YesNo || word == ControlWord_NoYes ||
            word == ControlWord_YesNo_NotSe || word == ControlWord_YesNo_NotSe_IIE)
        {
        }
        else if (word == ControlWord_HalfSpace)
        {
            currentX += fontSize >> 1;
        }
        else if (word == ControlWord_X)
        {
            i++;
            if (otherX < wordList[i])
                otherX = wordList[i];

            int tmp = currentX;
            currentX = otherX;
            otherX = tmp;
        }
        else if (word == ControlWord_Y)
        {
            i++;
        }
        else if (word == ControlWord_XY)
        {
            unsigned short nextWord = wordList[i + 1];
            i += 2;
            if (otherX < nextWord)
                otherX = nextWord;
            
            int tmp = currentX;
            currentX = otherX;
            otherX = tmp;
        }
        else if (word == ControlWord_W)
        {
            unsigned short nextWord = wordList[i + 1];
            i += 2;
            currentX += nextWord;
        }
        else if (word == ControlWord_H)
        {
            i++;
        }
        else if (word == ControlWord_WH)
        {
            unsigned short nextWord = wordList[i + 1];
            i += 2;
            currentX += nextWord;
        }
        else
            currentX += fontSize;
    }
    if (currentX < otherX)
        currentX = otherX;
    return currentX;
}

void TextManager::ReleaseText16Bit(unsigned short *wordList, int len)
{
    if (wordList == NULL || len < 0)
        return;

    for (int i = 0; i < len; i++)
    {
        unsigned short word = wordList[i];
        wordList[i] = ControlWord_End;
        if (word == ControlWord_End)
            return;

        if ((word & 0xff00) == 0xff00 || word >= 0x80)
            continue;

        RenderGlyph* rglyph = &glyphs_[word];
        if (rglyph->bitAllocationIndex < 0)
            continue;
        glyphRefcounts_17b8_[word]--;
        if (glyphRefcounts_17b8_[word] == 0)
        {
            rglyph->Reset2();
            numGlyphsInUse_9b4_--;
            numGlyphsInUse_9b4_ &= 0x7f;
        }
    }
}

void TextManager::ClearCachedData()
{
    maybePartySize_30_ = -1;
    maybeNumAlive_31_ = -1;
    partyMemberIndex_1880_ = -1;
    memset(tmapSecStrings_2ec_, 0, sizeof(tmapSecStrings_2ec_));
    memset(actorBuffer_3ac_, 0, sizeof(actorBuffer_3ac_));
    memset(buffer_3ec_, 0, sizeof(buffer_3ec_));
    memset(buffer_42c_, 0, sizeof(buffer_42c_));
    memset(buffer_46c_, 0, sizeof(buffer_46c_));
    memset(customStrings_4ac_, 0, sizeof(customStrings_4ac_));
    memset(valueLookup_8b0_, 0, sizeof(valueLookup_8b0_));
    memset(valueAlignments_8f1_, 0, sizeof(valueAlignments_8f1_));
    memset(valueWidths_902_, 0, sizeof(valueWidths_902_));
    unknown_8ac_ = 0;
    unknown_901_ = 0;
    unknown_8f0_ = 0;
    unknown_2e8_ = 0;
    for (int i = 0; i < 2; i++)
    {
        actors_0_[i] = NULL;
        actions_8_[i] = NULL;
        targets_10_[i] = NULL;
        items_18_[i] = NULL;
        monsters_20_[i] = NULL;
    }
    vocation_28_ = NULL;
    reflex_2c_ = NULL;
    targetIsParty_19d5_ = false;
    actorIsParty_19d6_ = false;
    multipleTargets_19d7_ = false;
    maybeTargetingMultipleGroups_19d8_ = false;
}

static void CopyStringSafe(const char* src, char* dst, int bufferSize)
{
    if (src == NULL || dst == NULL)
        return;
    strncpy(dst, src, bufferSize - 1);
    dst[bufferSize - 1] = '\0';
}

void TextManager::StorePartyMemberIndex(char idx)
{
    partyMemberIndex_1880_ = idx;
}

void TextManager::StoreMapSectionString(int section, const char *str)
{
    CopyStringSafe(str, tmapSecStrings_2ec_[section], sizeof(tmapSecStrings_2ec_[section]));
}

void TextManager::StoreActorString(const char *str)
{
    CopyStringSafe(str, actorBuffer_3ac_, sizeof(actorBuffer_3ac_));
}

void TextManager::StoreUnknownString_3ec(const char *str)
{
    CopyStringSafe(str, buffer_3ec_, sizeof(buffer_3ec_));
}

void TextManager::StoreUnknownString_42c(const char *str)
{
    CopyStringSafe(str, buffer_42c_, sizeof(buffer_42c_));
}

void TextManager::StoreUnknownString_46c(const char *str)
{
    CopyStringSafe(str, buffer_46c_, sizeof(buffer_46c_));
}

void TextManager::StoreCustomString(int index, const char *str)
{
    if (str == NULL || index < 0 || index >= 16)
        return;
    strncpy(customStrings_4ac_[index], str, sizeof(customStrings_4ac_[index]) - 1);
    customStrings_4ac_[index][sizeof(customStrings_4ac_[index]) - 1] = '\0';
}

void TextManager::StoreValue(int index, int value)
{
    if (index >= 0 && index < 16)
        valueLookup_8b0_[index] = value;
}

void TextManager::SetValueAlignment(int index, int alignment)
{
    if(index >= 0 && index < 16)
        valueAlignments_8f1_[index] = alignment;
}

void TextManager::SetValueWidth(int index, int width)
{
    if (index >= 0 && index < 16)
        valueWidths_902_[index] = width;
}


void TextManager::ProcessMarkup(int unused, const char* input, char* output,
    int boxWidth, int margin, unsigned char fontIndex)
{
    if (input == NULL || output == NULL)
        return;
    char scratch[0x960] = {0};
    SubstituteValueTags(input, scratch);
    SubstituteUnsupportedCharacters(scratch, output, fontIndex);
    SubstituteConditionals(output, scratch, fontIndex);
    SubstituteNamesAndArticles(scratch, output, fontIndex);
    SubstituteCaps(output, scratch, fontIndex);
    CalculateNewlines(false, scratch, output, boxWidth, margin, false, fontIndex);
}

void TextManager::SetObjUI(OBJTextUI* ui)
{
    objUI_38_ = ui;
}

#endif