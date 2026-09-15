#include "Graphics/Text.h"
#include "System/Graphics.h"
#include "Graphics/Vector.h"
#include "Filesystem/FileIO.h"
#include "Filesystem/BackgroundLoader.h"
#include "Graphics/VRAMStaging.h"
#include <globaldefs.h>

extern unsigned int data_021077fc; // palette VRAM offset for glyphs
struct Struct_02107800
{
    char unk_0[0x1c];
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

extern "C"
{
    // get next glyph index
    int func_020424e4(const char*, int);
    // get character entry from (glyph index, font type)
    FontIndexFile::Glyph* func_020425b4(int, int);
    // get glyph kerning
    int func_020425e4(int leftGlyph, int rightGlyph, int font);

    // alternative strlen
    int func_020d2ff0(const char*);
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
        int glyph = func_020424e4(text, fontType);
        if (glyph < 0)
        {
            lastGlyphIndex = 0xff;
            text++;
        }
        else
        {
            thisGlyphIndex = glyph;
            const FontIndexFile::Glyph* entry = func_020425b4(glyph, fontType);
            int kerning = func_020425e4(lastGlyphIndex, thisGlyphIndex, fontType);
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