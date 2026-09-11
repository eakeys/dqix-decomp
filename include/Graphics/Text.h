#pragma once

#include "../Memory/SafeAllocator.h"

// note for posterity: be careful if porting this to 64-bit, the file is loaded
// by memcpy where all pointers hold 32-bit offsets from the start of the file
// and then struct base gets added!
struct FontIndexFile
{
    unsigned int maybeSignature; // seems to hold "1.1\0"
    unsigned int numCharEntries;
    unsigned int numUnknownEntries;

    struct CharacterEntry
    {
        // usually a single character, but can be e.g. "<1>". Most likely when
        // this is found in a string, the corresponding glyph is rendered. Points
        // into the tag pool (after setup is done)
        const char* tag;
        char unk_4;
        char tagLength : 6;
        char unk_6[2];
    };

    int* unknownEntries;
    CharacterEntry* characters;
    const char* tagPool; // pool of all null-terminated tags
};

struct FontDataFile
{
    unsigned int maybeSignature; // seems to hold "1.0\0"
    char unk_4[8];
    void* ptr_c;
};

// sizeof == 0x1c.
struct Glyph
{
    char unk_0[4];
    unsigned int textureVRAMOffset;
    int unknown_8;
    int drawX;
    int drawY;
    short drawZ;
    unsigned char unk_16_low : 1;
    unsigned char unk_16_1 : 1;
    unsigned char unk_16_2 : 1;
    char unk_17[1];
    void* unknown_18;

    // Not used in all text rendering, only certain NPCs, combat text
    // and item pickup in overland
    void Draw(int color, void* unknown, int alpha);
};

// sizeof == 0x1e2c == 7724.
// Dynamically allocated by func_020421c4. 
class TextManager
{
public:
    char unk_0[0x48];
    void* ptr_48;
    void* ptr_4c;
    void* ptr_50;
    char unk_54[8];
    void* ptr_5c;
    void* ptr_60;
    void* ptr_64;
    char unk_68[0x8b0 - 0x68];

    // not sure about size. Text can have markers <val_1>, <val_2> or similar
    // which get replaced by game quantities e.g. amount of gold, this array
    // stores the replacement values
    int valueLookup[16];
    char unk_8f0[1];
    unsigned char maybeValuePaddingModes[16];
    char unk_901[1];
    unsigned char valueUnknownArray[16];

    char unk_912[0x9b8 - 0x912];
    Glyph glyphs_[0x80];
    char unk_17b8[0x1e2c - 0x17b8];

    static TextManager* GetInstance();

    // usa: func_0206831c
    void SubstituteValueTags(const char* input, char* output);
    // substitute any character that is not: 1) within a <tag>; 2) a special character
    // such as ' ', '/' or '\n', or the literal R"\n"; or 3) a recognized tag
    // of the font; with the tag "< >"
    void SubstituteUnsupportedCharacters(const char* input, char* output, int fontType);
    // usa: func_02068f40
    // handle things like <IF_HOST_MALE>he<ELSE_HOST_NOT_MALE>she<ENDIF_HOST_MALE>
    // by evaluating the relevant condition.
    // Accepted conditionals:
    // func_0206852c	<IF_SP><ELSE_NOT_SP><ENDIF_SP>
    // func_02068558	<IF_HOST_MALE><ELSE_HOST_NOT_MALE><ENDIF_HOST_MALE>
    // func_02068580	<IF_ADDRESSEE_MALE><ELSE_ADDRESSEE_NOT_MALE><ENDIF_ADDRESSEE_MALE>
    // func_020685a4	<IF_HERO_MALE><ELSE_HERO_NOT_MALE><ENDIF_HERO_MALE>
    // func_020685cc	<IF_MALE><ELSE_NOT_MALE><ENDIF_MALE>
    // func_020685f0	<IF_FEMALE_PARTY><ELSE_NOT_FEMALE_PARTY><ENDIF_FEMALE_PARTY>
    // func_02068650	<IF_SOLO><ELSE_NOT_SOLO><ENDIF_SOLO>               // this check is simpler than the alone check. Probably this is '1 person in party'. Fires on wipe
    // func_0206867c	<IF_SOLOHOST><ELSE_NOT_SOLOHOST><ENDIF_SOLOHOST>
    // func_020686e0	<IF_ALONE><ELSE_NOT_ALONE><ENDIF_ALONE>            // ..and this is '1 living party member?'
    // func_02068768	<IF_SING VAL_1><ELSE_NOT_SING><ENDIF_SING>
    // func_0206877c	<IF_SING VAL_2><ELSE_NOT_SING><ENDIF_SING>
    // func_02068790	<IF_SING VAL_3><ELSE_NOT_SING><ENDIF_SING>
    // func_020687a4	<IF_SING VAL_4><ELSE_NOT_SING><ENDIF_SING>
    // func_020687b8	<IF_SING VAL_5><ELSE_NOT_SING><ENDIF_SING>
    // func_020687cc	<IF_I_NAME_PLRNOUN><ELSE_NOT_PLRNOUN><ENDIF_PLRNOUN>
    // func_020687cc	<IF_I_NAME1_PLRNOUN><ELSE_NOT_PLRNOUN><ENDIF_PLRNOUN>
    // func_020687f8	<IF_M_NAME_PLRNOUN><ELSE_NOT_PLRNOUN><ENDIF_PLRNOUN>
    // func_02068824	<IF_I_NAME_M><IF_I_NAME_F><IF_I_NAME_N><ENDIF_I_NAME_MFN>
    // func_02068840	<IF_M_NAME_M><IF_M_NAME_F><IF_M_NAME_N><ENDIF_M_NAME_MFN>
    // func_0206885c	<IF_ACTION_M><IF_ACTION_F><IF_ACTION_N><ENDIF_ACTION>
    // func_02068878	<IF_ACTOR_MALE><ELSE_ACTOR_NOT_MALE><ENDIF_ACTOR_MALE>
    // func_02068894	<IF_ACTOR_M><IF_ACTOR_F><IF_ACTOR_N><ENDIF_ACTOR_MFN>
    // func_020688b0	<IF_TARGET_M><IF_TARGET_F><IF_TARGET_N><ENDIF_TARGET_MFN>
    // func_020688cc	<IF_TARGET_PARTY><ELSE_TARGET_NOT_PARTY><ENDIF_TARGET_PARTY>
    // func_020688e4	<IF_ACTOR_PARTY><ELSE_ACTOR_NOT_PARTY><ENDIF_ACTOR_PARTY>
    // func_020688fc	<IF_ACTOR_TARGET><ELSE_ACTOR_NOT_TARGET><ENDIF_ACTOR_TARGET>
    // func_02068928	<IF_ACTOR_PR><ELSE_ACTOR_NOT_PR><ENDIF_ACTOR_PR>
    // func_02068954	<IF_TARGET_PR><ELSE_TARGET_NOT_PR><ENDIF_TARGET_PR>
    // func_02068980	<IF_TARGET_SING><ELSE_TARGET_PLR><ENDIF_TARGET>
    // func_0206898c	<IF_TARGET_MIXED><ELSE_TARGET_NOT_MIXED><ENDIF_TARGET_MIXED>
    // func_020689a4	<IF_VOWEL_FR_HERO><ELSE_CONSONANT_FR_HERO><ENDIF_VOWEL_FR_HERO>
    // func_020689e4	<IF_VOWEL_FR_ADDRESSEE><ELSE_CONSONANT_FR_ADDRESSEE><ENDIF_VOWEL_FR_ADDRESSEE>
    // func_02068a20	<IF_VOWEL_FR_LEADER><ELSE_CONSONANT_FR_LEADER><ENDIF_VOWEL_FR_LEADER>
    // func_02068a5c	<IF_VOWEL_FR_I><ELSE_CONSONANT_FR_I><ENDIF_VOWEL_FR_I>
    // func_02068a5c	<IF_VOWEL_FR_I1><ELSE_CONSONANT_FR_I><ENDIF_VOWEL_FR_I>
    // func_02068a8c	<IF_VOWEL_FR_I2><ELSE_CONSONANT_FR_I><ENDIF_VOWEL_FR_I>
    // func_02068abc	<IF_VOWEL_FR_M><ELSE_CONSONANT_FR_M><ENDIF_VOWEL_FR_M>
    // func_02068aec	<IF_VOWEL_FR_VOCATION><ELSE_CONSONANT_FR_VOCATION><ENDIF_VOWEL_FR_VOCATION>
    // func_02068b1c	<IF_VOWEL_FR_ACTOR><ELSE_CONSONANT_FR_ACTOR><ENDIF_VOWEL_FR_ACTOR>
    // func_02068b4c	<IF_VOWEL_FR_TARGET><ELSE_CONSONANT_FR_TARGET><ENDIF_VOWEL_FR_TARGET>
    // func_02068b7c	<IF_SING_FR VAL_1><ELSE_NOT_SING_FR><ENDIF_SING_FR>
    // func_02068b94	<IF_SING_FR VAL_2><ELSE_NOT_SING_FR><ENDIF_SING_FR>
    // func_02068bac	<IF_SING_FR VAL_3><ELSE_NOT_SING_FR><ENDIF_SING_FR>
    // func_02068bc4	<IF_SING_FR VAL_4><ELSE_NOT_SING_FR><ENDIF_SING_FR>
    // func_02068bdc	<IF_SING_FR VAL_5><ELSE_NOT_SING_FR><ENDIF_SING_FR>
    // func_02068bf4	<IF_VOWEL_HERO><ELSE_CONSONANT_HERO><ENDIF_VOWEL_HERO>
    // func_02068c30	<IF_VOWEL_LEADER><ELSE_CONSONANT_LEADER><ENDIF_VOWEL_LEADER>
    // func_02068c6c	<IF_VOWEL_I><ELSE_CONSONANT_I><ENDIF_VOWEL_I>
    // func_02068c9c	<IF_VOWEL_M><ELSE_CONSONANT_M><ENDIF_VOWEL_M>
    // func_02068ccc	<IF_VOWEL_VOCATION><ELSE_CONSONANT_VOCATION><ENDIF_VOWEL_VOCATION>
    // func_02068cfc	<IF_VOWEL_ACTOR><ELSE_CONSONANT_ACTOR><ENDIF_VOWEL_ACTOR>
    // func_02068d2c	<IF_VOWEL_TARGET><ELSE_CONSONANT_TARGET><ENDIF_VOWEL_TARGET>
    // func_02068d5c	<IF_LAST_LETTER_S_I><ELSE_NOT_LAST_LETTER_S_I><ENDIF_LAST_LETTER_S_I>
    // func_02068d8c	<IF_LAST_LETTER_S_M><ELSE_NOT_LAST_LETTER_S_M><ENDIF_LAST_LETTER_S_M>
    // func_02068dbc	<IF_LAST_LETTER_S_DE_ACTOR><ELSE_NOT_LAST_LETTER_S_DE_ACTOR><ENDIF_LAST_LETTER_S_DE_ACTOR>
    // func_02068dec	<IF_LAST_LETTER_S_DE_TARGET><ELSE_NOT_LAST_LETTER_S_DE_TARGET><ENDIF_LAST_LETTER_S_DE_TARGET>
    void SubstituteConditionals(char* input, char* output, int fontType);
};

// usa: func_0204254c
// fontType = 0: s7, 1: me
FontIndexFile::CharacterEntry* GetNextFontCharacterEntry(const char* tag, int fontType);


// usa: func_02042804
// loads a font fron the data in data/pack_lv5/fi_%s.bin and
// data/pack_lv5/fd_%s.bin
void LoadCustomFont(SafeAllocator* alloc, const char* name, FontIndexFile** ppIndex, FontDataFile** ppData);
// usa: func_02042944
// loads both the s7 and me font from data/pack_lv5
void LoadCustomFonts(SafeAllocator* alloc);