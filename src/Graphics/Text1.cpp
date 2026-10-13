#include "Graphics/Text.h"

extern "C"
{
    void func_0207df50(void*);
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
        if (unknown_195b_ & 0x80)
        {
            if (rg->unk_16_2)
                return i;
        }
        else if (unknown_195b_ & 2)
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
    unknown_9b4_ = 0;
    for (int i = 0; i < 0x80; i++)
    {
        glyphs_[i].Reset2();
        glyphRefcounts_17b8_[i] = 0;
    }
    if (unknown_1e28_ != NULL)
        func_0207df50(unknown_1e28_);
}
