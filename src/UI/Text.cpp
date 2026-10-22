#include "UI/Text.h"
#include "NitroSDK/Graphics.h"
#include "Graphics/Vector.h"
#include "Filesystem/FileIO.h"
#include "Filesystem/BackgroundLoader.h"
#include "Graphics/VRAMStaging.h"
#include "GameState/GameState.h"
#include <globaldefs.h>

extern unsigned int data_021077fc; // palette VRAM offset for glyphs

struct TileMapIndexData
{
    unsigned short backgroundIndexBuffer[0x400];
    unsigned short textBorderIndexBuffer[0x400];
};

struct Struct_02107800
{
    TileMapIndexData* tileMapIndices;
    unsigned char* windowBorderTiles;
    char unk_8[4];
    char* secondaryScratchBuffer;
    char unk_10[8];
    char* primaryScratchBuffer;
    TextManager* manager;
    char unk_20[0xc];
    FontIndexFile* pFontIndexFiles[2];
    FontDataFile* pFontDataFiles[2];
} extern data_02107800;

extern const unsigned char data_020e7a50[]; // probably multiple separate things. We only use stuff at offset 8
extern const int data_020e7a94[]; // { 2, 3 }
extern const char data_020e7a9c[]; // { 1, 1, 1, 1, 1, 1, 1, 1, 15 }
extern const short data_020e7aa6[]; // { 1, 2, 2, 3, 2 }
extern const short data_020e7ab0[]; // { 2, 1, 3, 2, 2 }
extern const unsigned char data_020e7ade[3][8];

extern char data_020efe78[]; // "<X=%d>"
extern char data_020efe7f[]; // "<Y=%d>"
extern char data_020efe86[]; // "<XY=%d,%d>"
extern char data_020efe91[]; // "<W=%d>"
extern char data_020efe98[]; // "<H=%d>"
extern char data_020efe9f[]; // "<WH=%d,%d>"
extern char data_020efeaa[]; // "<ENC=%d>"
extern char data_020efeb3[]; // "<N=%d>%s</N>"
extern char data_020efec0[]; // "<SDRC=%d,%d,%d,%d,%d>"
extern char data_020efed6[]; // "<SOLID=%d,%d>"
extern char data_020efee4[]; // "<FRAME=%d,%d,%d,%d,%d>"
extern char data_020efefb[]; // "<WIRE=%d,%d,%d,%d,%d>"
extern char data_020eff11[]; // "<LINE=%d>"
extern char data_020eff1b[]; // "<LINEX=%d,%d,%d,%d>"
extern char data_020eff2f[]; // "<LINEY=%d,%d,%d,%d>"
extern char data_020eff43[]; // "<SIZE=%d>"
extern char data_020eff4d[]; // "<PLTT=15>"
extern char data_020eff57[]; // "<PLTT=11>"
extern char data_020eff61[]; // "<PLTT=13>"
extern char data_020eff6b[]; // "<PLTT=9>"
extern char data_020eff74[]; // "<PLTT=%d>"
extern char data_020eff7e[]; // "<SLT=%d>"
extern char data_020eff87[]; // "<CURSOR=%d>"
extern char data_020eff93[]; // "<UA=%d,%d,%d>"
extern char data_020effa1[]; // "<UB=%d,%d,%d>"
extern char data_020effaf[]; // "<DA=%d,%d,%d>"
extern char data_020effbd[]; // "<DB=%d,%d,%d>"
extern char data_020effcb[]; // "<TEN=%d,%d>"
extern char data_020effd7[]; // "<TITLE=%d>%s</TITLE>"
extern char data_020effec[]; // "<TALK>%s</TALK>"
extern char data_020efffc[]; // "<XR=%d>%s</XR>"
extern char data_020f000b[]; // "/data/ani/windata3.bncg"
extern char data_020f0023[]; // "/data/pack_lv5/font_lv5.gp2"
extern char data_020f003f[]; // "f8.mes"
extern char data_020f0046[]; // "%d"
extern char data_020f0049[]; // "data/pack_lv5/fi_%s.bin"
extern char data_020f0061[]; // "data/pack_lv5/fd_%s.bin"
extern char data_020f0079[]; // "s7"
extern char data_020f007c[]; // "me"

extern char data_02108760[];

extern "C"
{
    void func_02042af4(TextManager*, void*, int);
    void func_02042b98(TextManager*, int, int, int, int);
    void func_02042fcc(TextBox*);
    void func_02043040(TextManager*);
    void func_02043124(TextManager*);
    void func_020430b0(TextManager*);
    void func_0204394c(void*);
    void func_02044c74(TextManager*, UIElement*);
    void func_02044d0c();
    void func_02044f3c(TextManager*, const char*, int);
    void func_02045824(TextManager*, int, int);

    void func_0205e944(void*, int);

    void func_02066cf0(TextManager*, int); // maybe bool

    // get offsets among (-3, 0), (0, 0), (0, -2), (0, 0)
    void func_0206b804(TextManager*, int*, int*);

    void func_0207df50(void*);
    void func_0207df90(void*);
    void func_0207dfac(void*);

    // alternative strlen
    int func_020d2ff0(const char*);
    
    // populate noun from item ID?
    void func_020dcf7c(short, Noun*);

    unsigned char* func_020e1b24(void*);
    void func_020e1b2c(void*, int);
    bool func_020e1f1c(void*);
    void func_020e2110(void*);
    void func_020e23f4(void*);
    void func_020e2478(void*);
}

static inline void WriteVertex16(unsigned short x, fix32_t y, unsigned short z)
{
    GXFIFO_VERTEX_16 = x | ((unsigned short)y << 16);
    GXFIFO_VERTEX_16 = z;
}

static inline int Foo(float x)
{
    return 16 * x;
}

void WriteTexCoords(fix32_t x, fix32_t y); // can be made static

void RenderGlyph::Reset()
{
    textureData = NULL;
    textureVRAMOffset = 0;
    unknown_8 = -1;
    drawX = 0;
    drawY = 0;
    drawZ = 0;
    indexGlyph = NULL;
    unk_16_low = 0;
    unk_16_1 = 0;
    unk_16_2 = 0;
    unk_16_3 = 0;
    unknown_17 = 0;
}

void RenderGlyph::Reset2()
{
    Reset();
}

const char* RenderGlyph::GetGlyphTag() const
{
    if (indexGlyph != NULL)
        return indexGlyph->tag;
    return NULL;
}

void RenderGlyph::UploadToVRAM()
{
    if (unknown_8 < 0)
        return;
    if (unk_16_low && textureData != NULL)
    {
        StageMemoryToVRAM(VRAMSubregion_TextureImage, textureData, textureVRAMOffset, 0x80, true, true);
        unk_16_low = false;
    }
}

void RenderGlyph::Draw(int color, void *unknown, int alpha)
{
    if (unknown_8 < 0 || unk_16_low || alpha == 0)
        return;

    GXFIFO_MATRIX_PUSH = 0;

    fix32_t fixPointZ = (drawZ + 1024) << 12;
    fix32_t fixPointX = drawX << 12;
    fix32_t fixPointY = drawY << 12;
    float zero = 0.0f;
    
    GXFIFO_MATRIX_TRANSLATE = fixPointX;
    GXFIFO_MATRIX_TRANSLATE = fixPointY;
    GXFIFO_MATRIX_TRANSLATE = fixPointZ;

    GXFIFO_MATRIX_SCALE = 1 << 12;
    GXFIFO_MATRIX_SCALE = 1 << 12;
    GXFIFO_MATRIX_SCALE = 1 << 12;

    GXFIFO_MATRIX_PUSH = 0;
    GXFIFO_MATRIX_TRANSLATE = 0 << 12;
    GXFIFO_MATRIX_TRANSLATE = 16 << 12;
    GXFIFO_MATRIX_TRANSLATE = 0 << 12;

    GXFIFO_MATRIX_SCALE = 1 << 12;
    GXFIFO_MATRIX_SCALE = -1 << 12;
    GXFIFO_MATRIX_SCALE = 1 << 12;

    GXFIFO_MATRIX_PUSH = 0;

    GXFIFO_DIFFUSE_AMBIENT = 0x7fffffff;
    GXFIFO_SPECULAR_EMISSION = 0x4210;
    GXFIFO_TEXIMAGE_PARAMS = (textureVRAMOffset >> 3) | 0x6c900000;
    GXFIFO_PALETTE_BASE = data_021077fc >> 4;
    GXFIFO_POLYGON_ATTRIBUTES = (alpha << 16) | 0x3e0000c0;

    GXFIFO_MATRIX_SCALE = 16 << 12;
    GXFIFO_MATRIX_SCALE = 16 << 12;
    GXFIFO_MATRIX_SCALE = 1 << 12;

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
    WriteVertex16(0, 4096.0f * topYVertexCoord, 0);

    // top right
    WriteTexCoords(16 << 12, topYTexcoord << 12);
    WriteVertex16(1 << 12, 4096.0f * topYVertexCoord, 0);

    // bottom right
    WriteTexCoords(16 << 12, 16 << 12);
    WriteVertex16(1 << 12, 0, 0);

    // bottom left
    WriteTexCoords(0, 16 << 12);
    WriteVertex16(0, 0, 0);

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
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efe78, x) - buffer;
}

int AddYTag(char* buffer, int y)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efe7f, y) - buffer;
}

int AddXYTag(char* buffer, int x, int y)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efe86, x, y) - buffer;
}

int AddWidthTag(char* buffer, int w)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efe91, w) - buffer;
}

int AddHeightTag(char* buffer, int h)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efe98, h) - buffer;
}

int AddWidthHeightTag(char* buffer, int w, int h)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efe9f, w, h) - buffer;
}

// missing function? that uses <ENC=%d>

int AddIndexedTag(char* buffer, int index, const char* value)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efeb3, index, value) - buffer;
}

int AddSolidRectTag(char* buffer, int col, int x, int y, int width, int height)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efec0, col, x, y, width, height) - buffer;
}

// missing function? using <SOLID=%d,%d>

// Couldn't find a call site in game, but there's a bunch in overlay 2
int AddFrameTag(char* buffer, int col, int x, int y, int width, int height)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efee4, col, x, y, width, height) - buffer;
}

// called twice in overlay 2
int AddWireTag(char* buffer, int col, int x, int y, int width, int height)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efefb, col, x, y, width, height) - buffer;
}

// used to draw a horizontal line in skill point menu, parameter is y coordinate relative to container?
int AddLineTag(char* buffer, int y)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff11, y) - buffer;
}

// horizontal line segment, not sure about first argument
int AddLineXTag(char* buffer, int col, int left, int right, int y)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff1b, col, left, right, y) - buffer;
}

int AddLineYTag(char* buffer, int col, int x, int top, int bottom)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff2f, col, x, top, bottom) - buffer;
}

int AddSizeTag(char* buffer, int size)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff43, size) - buffer;
}

int AddReducedPaletteTag(char* buffer, int reducedEnum)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    switch (reducedEnum)
    {
    case 0:
        writePos += sprintf(writePos, data_020eff4d); // PLTT=15
        break;
    case 5:
        writePos += sprintf(writePos, data_020eff57); // PLTT=11, orange?
        break;
    case 4:
        writePos += sprintf(writePos, data_020eff61); // PLTT=13, yellow?
        break;
    case 6:
        writePos += sprintf(writePos, data_020eff6b); // PLTT=9, red?
        break;
    }
    return writePos - buffer;
}

int AddPaletteTag(char* buffer, int palette)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff74, palette) - buffer;
}

// missing <SLT=%d> function?

int AddCursorTag(char* buffer, int cursorPos)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff87, cursorPos) - buffer;
}

// not sure of purpose, but used in bank when depositing/withdrawing.
// removing it doesn't get rid of the arrows though
int AddUATag(char* buffer, int arg1, int arg2, int arg3)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020eff93, arg1, arg2, arg3) - buffer;
}

// unused functions? <UB=%d,%d,%d>, <DA=%d,%d,%d>

// same call site as UA (bank)
int AddDBTag(char* buffer, int arg1, int arg2, int arg3)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020effbd, arg1, arg2, arg3) - buffer;
}

// used once in overlay 17 with args 3,2, not sure of purpose
int AddTenTag(char* buffer, int x, int y)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020effcb, x, y) - buffer;
}

// used for e.g. "Whose?" when assigning skill points out of battle
int AddTitleTag(char* buffer, const char* text, int index)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020effd7, index, text) - buffer;
}

// used somewhere in overlay 3 (shops, inn, bank?) but I couldn't find it
int AddTalkTag(char* buffer, const char* text)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020effec, text) - buffer;
}

// used for percentages in battle records to right-align them, number
// is offset from its (invisible) container
int AddXRightTag(char* buffer, const char* text, int xright)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, data_020efffc, xright, text) - buffer;
}

int AddText(char* buffer, const char* text)
{
    char* writePos = buffer + func_020d2ff0(buffer);
    return writePos + sprintf(writePos, text) - buffer;
}

int AddCenteredText(char* buffer, const char* text, int containerWidth, int fontIndex)
{
    int requiredWidth = MeasureTextWidth(text, fontIndex);
    sprintf(buffer + func_020d2ff0(buffer), data_020efe91, ((containerWidth - requiredWidth) >> 1) + 1);
    sprintf(buffer + func_020d2ff0(buffer), text);
    return func_020d2ff0(buffer);
}   

}

int MeasureTextWidth(const char* text, int fontType)
{
    if (text == NULL)
        return 0;

    unsigned char spaceSize = data_020e7a94[fontType];
    unsigned char lastGlyphIndex = 0xff;
    unsigned char thisGlyphIndex;
    int totalWidth = 0;

    while (true)
    {
        if (*text == 0)
            break;

        int thisCharWidth = spaceSize + 1;
        int glyphIndex = GetNextFontGlyphIndex(text, fontType);
        if (glyphIndex < 0)
        {
            lastGlyphIndex = 0xff;
            text++;
        }
        else
        {
            thisGlyphIndex = glyphIndex;
            const FontIndexFile::Glyph* entry = GetFontGlyphByIndex(glyphIndex, fontType);
            int kerning = GetFontGlyphKerning(lastGlyphIndex, thisGlyphIndex, fontType);
            thisCharWidth = entry->width + 1 + kerning;
            text += entry->tagLength;
            lastGlyphIndex = thisGlyphIndex;
        }
        totalWidth += thisCharWidth;
    }
    return totalWidth - 1;
}

int GetFontIDBySize(int size)
{
    int id = 0; // s7
    if (size == 12)
        id = 1; // me
    return id;
}

TextManager* TextManager::GetInstance()
{
    return data_02107800.manager;
}

const unsigned char* GetWindowBorderTile(int id)
{
    return data_02107800.windowBorderTiles + (id * 32);
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
    const FontIndexFile* file = data_02107800.pFontIndexFiles[fontType];
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

    FontIndexFile* indexFile = data_02107800.pFontIndexFiles[fontType];
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
    FontIndexFile* file = data_02107800.pFontIndexFiles[fontType];
    if (glyph >= 0 && glyph < file->numCharEntries)
        return &file->glyphs[glyph];
    return NULL;
}

int GetFontGlyphKerning(int leftGlyph, int rightGlyph, int fontType)
{
    FontIndexFile* file = data_02107800.pFontIndexFiles[fontType];
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
    return data_02107800.pFontDataFiles[fontType];
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
    sprintf(buffer, data_020f0046, num);
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
    sprintf(fileName, data_020f0049, name);
    unsigned int fileSize = 0;
    LoadFileIntoMemory(fileName, data_0211e33c, &fileSize);

    if (fileSize != 0)
    {
        *ppIndex = (FontIndexFile*)alloc->Allocate(fileSize);
        memcpy(*ppIndex, data_0211e33c, fileSize);
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
    sprintf(fileName, data_020f0061, name);
    fileSize = 0;
    LoadFileIntoMemory(fileName, data_0211e33c, &fileSize);

    if (fileSize != 0)
    {
        *ppData = (FontDataFile*)alloc->Allocate(fileSize);
        memcpy(*ppData, data_0211e33c, fileSize);
        FontDataFile* basePtr = *ppData;
        basePtr->ptr_c = (void*)((intptr_t)basePtr + (intptr_t)basePtr->ptr_c);
    }
}

void LoadCustomFonts(SafeAllocator* alloc)
{
    BackgroundLoader::AddLockGlobal();
    LoadCustomFont(alloc, data_020f0079, &data_02107800.pFontIndexFiles[0], &data_02107800.pFontDataFiles[0]);
    LoadCustomFont(alloc, data_020f007c, &data_02107800.pFontIndexFiles[1], &data_02107800.pFontDataFiles[1]);
    BackgroundLoader::RemoveLockGlobal();
}

void TextManager::SetActiveTextBox(TextBox* to)
{
    substruct_19e0_.pCurrentBox = to;
}

void TextManager::SetMainTextBoxActive()
{
    SetActiveTextBox(&mainTextBox_914_);
}

void TextBox::Reset()
{
    x = y = width = height = 0;
    unknown_8 = 0;
    unknown_c = 0;
    scaleX = scaleY = 0;
    color = 0x7fff;
}

struct TextBoxBounds
{
    short x, y, width, height;

    short Left() const { return x; }
    short Top() const { return y; }

    void Clear();
    void SetBounds(short ax, short ay, short aw, short ah);
};

void DrawTextBoxBorderTile(UIElement* renderer, short x, short y, int idx)
{
    if (renderer == NULL)
        return;
    const unsigned char* tileBase = data_02107800.windowBorderTiles;
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

void TextManager::RefreshGraphics(bool arg)
{
    if (unknown_19cc_ != 0)
    {
        int shakeX, shakeY;
        GetBoxShakeOffset(&shakeX, &shakeY);
        func_02044c74(this, mainUIElement_34_);

        UIElement* ui = mainUIElement_34_;
        unsigned char* pixelBuffer = ui->pixelBuffer_8_;
        if (pixelBuffer != NULL)
        {
            ui->pixelBuffer_8_ = pixelBuffer; // wtf?
            unknown_19cc_ = 0;
            func_02066cf0(this, 1);
            unknown_19cc_ = 1;
            if (maybeBoxShaking_19c0_)
                unknown_40_ = 0;

            TextBoxBounds mainBoxBounds, speakerBoxBounds, optionBoxBounds;
            mainBoxBounds.Clear();
            speakerBoxBounds.Clear();
            optionBoxBounds.Clear();

            if ((unknown_998_ != 0 && showWindow_19b1_) || unknown_19cb_ != 0)
            {
                if (showWindow_19b1_)
                {
                    func_02042b98(this, 2, 116, 252, 74);
                    mainBoxBounds.SetBounds(shakeX + 2, shakeY + 116, 252, 74);
                    DrawTextBoxBorder2D(mainUIElement_34_, &mainBoxBounds);
                    mainBoxBounds.SetBounds(2, 116, 252, 74);
                }

                if (unknown_19c4_ != 0 && unknown_19c3_ != 0)
                {
                    Substruct_19e0* substruct = &substruct_19e0_;
                    substruct->pCurrentBox = &speakerTextBox_930_;
                    // fix this with, most likely, an inline call
                    bool speakerBoxReady = (substruct->pCurrentBox != NULL) ? (substruct->pCurrentBox->unknown_8 == 2) : false;
                    if (speakerBoxReady)
                    {
                        short boxX, boxY, boxW, boxH;
                        if (substruct->pCurrentBox != NULL)
                        {
                            boxX = substruct->pCurrentBox->x;
                            boxY = substruct->pCurrentBox->y;
                        }
                        if (substruct->pCurrentBox != NULL)
                        {
                            boxW = substruct->pCurrentBox->width;
                            boxH = substruct->pCurrentBox->height;
                        }
                        speakerBoxBounds.SetBounds(boxX + shakeX, boxY + shakeY, boxW, boxH);
                        DrawTextBoxBorder2D(mainUIElement_34_, &speakerBoxBounds);
                        speakerBoxBounds.SetBounds(boxX, boxY, boxW, boxH);
                    }
                    substruct->pCurrentBox = &mainTextBox_914_;
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
                memset(data_02107800.tileMapIndices->backgroundIndexBuffer, 0, 0x800);
                if (((unknown_998_ != 0 && showWindow_19b1_) || unknown_19cb_))
                {
                    if (showWindow_19b1_)
                    {
                        unsigned short* windowTiles = data_02107800.tileMapIndices->backgroundIndexBuffer;
                        DrawTextBoxBackground(windowTiles, &mainBoxBounds, 0);
                        DrawTextBoxBackground(windowTiles, &speakerBoxBounds, 1);
                        DrawTextBoxBackground(windowTiles, &optionBoxBounds, 2);
                        StageMemoryToVRAM(VRAMSubregion_MainBG2Screen, windowTiles, 0, 0x800, true, false);
                    }
                }

                if (!showWindow_19b1_)
                    StageMemoryToVRAM(VRAMSubregion_MainBG2Screen,
                        data_02107800.tileMapIndices->backgroundIndexBuffer, 0,
                        sizeof(data_02107800.tileMapIndices->backgroundIndexBuffer),
                        true, false);
                
                if (unknown_74_ == unknown_78_ || unknown_998_ == 0)
                {
                    TileMapIndexData* indexData = data_02107800.tileMapIndices;
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
            func_02043040(this);
            func_02043124(this);
            func_020430b0(this);
        }
    }

    if (unknown_19ce_ && unknown_44_ == 0)
        func_0204394c(unknown_1e28_);

    for (int i = 0; i < 2; i++)
        windows_[i].SetEnabled(false);

    if (arg && unknown_998_ == 0)
        return;

    if (unknown_1e28_ != NULL)
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
                func_0207df90(unknown_1e28_);
                rglyph->UploadToVRAM();
                func_0207dfac(unknown_1e28_);
            }
            func_02042af4(this, &substruct_2cc_, rglyph->unknown_8);
        }
    }

    if (mainTextBox_914_.unknown_8 >= 2)
    {
        windows_[0].SetBoundsXYWH(4, 118, 248, 70);
        if (unknown_1962_ != 0)
            windows_[0].SetEnabled(true);
    }

    if (dialogueOptions_90_.textBox_b4_.unknown_8 >= 2)
    {
        windows_[1].SetBoundsXYWH(dialogueOptions_90_.textBox_b4_.x + 2,
            dialogueOptions_90_.textBox_b4_.y + 2,
            dialogueOptions_90_.textBox_b4_.width - 4,
            dialogueOptions_90_.textBox_b4_.height - 4);
        if (unknown_1962_ != 0)
            windows_[1].SetEnabled(true);
    }

    for (int i = 0; i < 2; i++)
        windows_[i].Apply();
}

void TextBoxBounds::Clear()
{
    x = 0;
    y = 0;
    width = 0;
    height = 0;
}

void TextBoxBounds::SetBounds(short toX, short toY, short toWidth, short toHeight)
{
    x = toX;
    y = toY;
    width = toWidth;
    height = toHeight;
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