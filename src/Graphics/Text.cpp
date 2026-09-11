#include "Graphics/Text.h"
#include "System/Graphics.h"
#include "Graphics/Vector.h"
#include "Filesystem/FileIO.h"
#include "Filesystem/BackgroundLoader.h"
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

extern char data_020f0049[]; // "data/pack_lv5/fi_%s.bin"
extern char data_020f0061[]; // "data/pack_lv5/fd_%s.bin"
extern char data_020f0079[]; // "s7"
extern char data_020f007c[]; // "me"

extern "C"
{

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

void Glyph::Draw(int color, void *unknown, int alpha)
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

TextManager* TextManager::GetInstance()
{
    return data_02107800.manager;
}

FontIndexFile::CharacterEntry* GetNextFontCharacterEntry(const char* tag, int fontType)
{
    if (tag == NULL)
        return NULL;

    FontIndexFile* indexFile = data_02107800.pFontIndexFiles[fontType];
    for (unsigned int i = 0; i < indexFile->numCharEntries; i++)
    {
        FontIndexFile::CharacterEntry* entry = &indexFile->characters[i];
        if (memcmp(entry->tag, tag, entry->tagLength) == 0)
            return entry;
    }

    return NULL;
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
        basePtr->unknownEntries = (int*)(baseAddr + (intptr_t)basePtr->unknownEntries);
        basePtr->characters = (FontIndexFile::CharacterEntry*)(baseAddr + (intptr_t)basePtr->characters);
        basePtr->tagPool = (const char*)(baseAddr + (intptr_t)basePtr->tagPool);

        FontIndexFile::CharacterEntry* entry;
        for (unsigned int i = 0; i < basePtr->numCharEntries; i++)
        {
            entry = basePtr->characters + i;
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