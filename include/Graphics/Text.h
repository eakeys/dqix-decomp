#pragma once

#include "../Memory/SafeAllocator.h"
#include "std_library_functions.h"
#include "Vector.h"
#include "ClipWindow.h"

// note for posterity: be careful if porting this to 64-bit, the file is loaded
// by memcpy where all pointers hold 32-bit offsets from the start of the file
// and then struct base gets added!
struct FontIndexFile
{
    unsigned int maybeSignature; // seems to hold "1.1\0"
    unsigned int numCharEntries;
    unsigned int numKerningEntries;

    struct KerningEntry
    {
        // low 8 bits: first (left) glyph, high 8 bits: second (right) glyph
        uint16_t glyphPair;
        // negative: move closer together, positive: move further apart
        int8_t kerningAmount;
        char pad_3[1];
    };

    struct Glyph
    {
        // usually a single character, but can be e.g. "<1>". Most likely when
        // this is found in a string, the corresponding glyph is rendered. Points
        // into the tag pool (after setup is done)
        const char* tag;
        int8_t width;
        int8_t tagLength : 6;
        int8_t unk_5_bit_6 : 1;
        int8_t unk_5_bit_7 : 1;
        char unk_6[2];
    };

    KerningEntry* kerning;
    Glyph* glyphs;
    const char* tagPool; // pool of all null-terminated tags
};

struct FontDataFile
{
    unsigned int maybeSignature; // seems to hold "1.0\0"
    char unk_4[8];
    void* ptr_c;
};

// sizeof == 0x1c.
struct RenderGlyph
{
    const void* textureData;
    unsigned int textureVRAMOffset;
    int unknown_8;
    int drawX;
    int drawY;
    short drawZ;
    unsigned char unk_16_low : 1;
    unsigned char unk_16_1 : 1;
    unsigned char unk_16_2 : 1;
    unsigned char unk_16_3 : 1;
    char unknown_17;
    const FontIndexFile::Glyph* indexGlyph;

    void Reset();
    void Reset2();

    const char* GetGlyphTag() const;

    void UploadToVRAM();

    // Not used in all text rendering, only certain NPCs, combat text
    // and item pickup in overland
    void Draw(int color, void* unknown, int alpha);
    
};

// 16-bit value to put in a string representing a special functionality
enum ControlWord
{
    ControlWord_End = 0xff01,
    ControlWord_EndRTurn = 0xff02,
    ControlWord_Close = 0xff03, // closes quest? used alongside </QUEST>
    ControlWord_YesNo = 0xff04,
    ControlWord_NoYes = 0xff05,
    ControlWord_YesNo_NotSe = 0xff06,
    ControlWord_YesNo_NotSe_IIE = 0xff07,
    ControlWord_UkeYame = 0xff08,

    ControlWord_Add = 0xff0a,
    ControlWord_Auto = 0xff0b,
    ControlWord_PageT = 0xff0c,
    ControlWord_Page = 0xff0d,
    ControlWord_Shake = 0xff0e,
    ControlWord_QuestSE = 0xff0f, 
    ControlWord_Quest = 0xff10,
    ControlWord_QuestHan = 0xff11,
    ControlWord_SlashQuest = 0xff12, // "/QUEST" tag
    ControlWord_QuestFailed = 0xff13,
    ControlWord_Yes = 0xff14,
    ControlWord_No = 0xff15,
    ControlWord_Uke = 0xff16,
    ControlWord_Yame = 0xff17,
    ControlWord_LineBreak = 0xff18,

    ControlWord_Time = 0xff1a,
    ControlWord_AllRecover = 0xff1b,
    ControlWord_ST = 0xff1c,
    ControlWord_PadWait = 0xff1d,
    ControlWord_PadT = 0xff1e,
    ControlWord_PadWaitNoCursor = 0xff1f,
    ControlWord_Unknown_ff20 = 0xff20,
    ControlWord_Unknown_ff21 = 0xff21,
    ControlWord_Unknown_ff22 = 0xff22,
    ControlWord_Unknown_ff23 = 0xff23,
    ControlWord_Unknown_ff24 = 0xff24,
    ControlWord_Unknown_ff25 = 0xff25,
    ControlWord_WinOn = 0xff26,
    ControlWord_WinOff = 0xff27,
    ControlWord_CenOn = 0xff28,
    ControlWord_CenOff = 0xff29,
    ControlWord_Turn = 0xff2a,
    ControlWord_NTurn = 0xff2b,
    ControlWord_RTurn = 0xff2c,
    ControlWord_TurnP = 0xff2d,
    ControlWord_Exclamation = 0xff2e,
    ControlWord_Question = 0xff2f,

    ControlWord_MEBase = 0xff34,

    ControlWord_SEBase = 0xff4b,

    ControlWord_VoiceVolumeBase = 0xff4d, 

    ControlWord_LBBase = 0xffd0, // A character value is added to this
    ControlWord_JPBase = 0xffe0, // A character value is added to this
};

namespace StringBuilders
{
    int AddXTag(char* buffer, int x);
    int AddYTag(char* buffer, int y);
    int AddXYTag(char* buffer, int x, int y);
    int AddWidthTag(char* buffer, int w);
    int AddHeightTag(char* buffer, int h);
    int AddWidthHeightTag(char* buffer, int w, int h);
    // missing function? that uses <ENC=%d>
    int AddIndexedTag(char* buffer, int index, const char* value);
    int AddSolidRectTag(char* buffer, int col, int x, int y, int width, int height);
    // missing function? using <SOLID=%d,%d>
    int AddFrameTag(char* buffer, int col, int x, int y, int width, int height);
    int AddWireTag(char* buffer, int col, int x, int y, int width, int height);
    int AddLineTag(char* buffer, int y);
    int AddLineXTag(char* buffer, int col, int left, int right, int y);
    int AddLineYTag(char* buffer, int col, int x, int top, int bottom);
    int AddSizeTag(char* buffer, int size);
    int AddReducedPaletteTag(char* buffer, int reducedEnum);
    int AddPaletteTag(char* buffer, int palette);
    int AddCursorTag(char* buffer, int cursorPos);
    int AddUATag(char* buffer, int arg1, int arg2, int arg3);
    // missing AddUBTag, AddDATag?
    int AddDBTag(char* buffer, int arg1, int arg2, int arg3);
    int AddTenTag(char* buffer, int x, int y);
    int AddTitleTag(char* buffer, const char* text, int index);
    int AddTalkTag(char* buffer, const char* text);
    int AddXRightTag(char* buffer, const char* text, int xright);
    int AddText(char* buffer, const char* text);
    int AddCenteredText(char* buffer, const char* text, int containerWidth, int fontIndex);
}

// usa: func_02041fe8
int MeasureTextWidth(const char* text, int fontType);
// usa: func_02042190
// converts size 12 to 1 (fd_me.bin), all others to 0 (fd_s7.bin)
int GetFontIDBySize(int size);

// usa: func_020424ac
// returns a pointer to the '>' closing all open tags. (i.e. add 1 to go past
// the tag entirely)
char* GetTextTagEnd(char*);
// usa: func_020424e4
int GetNextFontGlyphIndex(const char* tag, int fontType);
// usa: func_0204254c
// fontType = 0: s7 (small font), 1: me (regular font, size 12)
FontIndexFile::Glyph* GetNextFontGlyph(const char* tag, int fontType);
// usa: func_020425b4
FontIndexFile::Glyph* GetFontGlyphByIndex(int glyph, int fontType);
// usa: func_020425e4
int GetFontGlyphKerning(int leftGlyph, int rightGlyph, int fontType);
// usa: func_02042638
FontDataFile* GetFontDataFile(int fontType);
// usa: func_02042648
int GetFontSpaceSize(int fontType);
// usa: func_02042658
int MeasureNumberTextWidth(int fontType, int num);
// usa: func_020426a4
int MeasureNumberTextWidthGivenSize(int fontSize, int num);
// Converts a normal string with glyph tags into the DQ9 encoding (e.g.
// with A = 0x12, <66> = 0x50)
int EncodeDQ9Text(char* decoded, unsigned char* output, int fontType);
// Converts from the DQ9 encoding (where e.g. A = 0x12) to the glyphs for
// that font type.
void DecodeDQ9Text(const unsigned char* encoded, char* output, int fontType);
// usa: func_02042804
// loads a font fron the data in data/pack_lv5/fi_%s.bin and
// data/pack_lv5/fd_%s.bin
void LoadCustomFont(SafeAllocator* alloc, const char* name, FontIndexFile** ppIndex, FontDataFile** ppData);
// usa: func_02042944
// loads both the s7 and me font from data/pack_lv5
void LoadCustomFonts(SafeAllocator* alloc);

// sizeof == 0x1e2c == 7724.
// Dynamically allocated by func_020421c4. 
// Initialized by func_02042c68, get member types from here
class TextManager
{
public:
    void* actors_0_[2];
    void* actions_8_[2];
    void* targets_10_[2];
    void* items_18_[2];
    void* monsters_20_[2];
    char unk_28[4];
    void* reflex_2c_;
    char unknown_30_;
    char unknown_31_;
    char unk_32[0x38 - 0x32];
    int unknown_38_;
    int unknown_3c_;
    char unk_40[0x48 - 0x40];
    void* ptr_48;
    void* ptr_4c;
    void* ptr_50;
    char unk_54[8];
    void* ptr_5c;
    void* ptr_60;
    void* ptr_64;
    int length_68_;
    char unk_6c[0x8c - 0x6c];
    int unknown_8c_;
    char substruct_90_[0x238]; // func_0205c790
    int unknown_2c8_;
    char substruct_2cc_[4];
    int unknown_2d0_;
    char unk_2d4[4];
    int unknown_2d8_;
    int unknown_2dc_;
    int unknown_2e0_;
    short unknown_2e4_;
    char unknown_2e6_;
    char unknown_2e7_;
    int unknown_2e8_;
    char unk_2ec[0x4ac - 0x2ec];
    char substructOrArray_4ac_[0x400];
    int unknown_8ac_;
    // not sure about size. Text can have markers <val_1>, <val_2> or similar
    // which get replaced by game quantities e.g. amount of gold, this array
    // stores the replacement values
    int valueLookup[16]; // @ 0x8b0
    char unk_8f0[1];
    unsigned char maybeValuePaddingModes[16];
    char unk_901[1];
    unsigned char valueUnknownArray[16];
    char unknown_912_;
    char unk_913[1];
    char substruct_914_[0x1c]; // func_02042fcc, seems related to main text box in 3d render mode 
    char substruct_930_[0x1c]; // same as above, seems related to speaker name box
    char unk_94c[0x954 - 0x94c];
    int unknown_954_;
    int unknown_958_;
    char substructOrArray_95c_[0x20];
    char substructOrArray_97c_[0x10];
    char unk_98c[0x990 - 0x98c];
    int unknown_990_;
    int unknown_994_;
    char unk_998[4];
    int unknown_99c_;
    int unknown_9a0_;
    int gyouParameter_9a4_;
    char unk_9a8[0x9b0 - 0x9a8];
    int unknown_9b0_;
    int unknown_9b4_;
    // holds data about glyphs that are used in the current message
    RenderGlyph glyphs_[0x80];
    // number of times a glyph is used within message
    char glyphRefcounts_17b8_[0x80];
    int npcID_1838_;
    fix32_t npcPriorRotation_183c_;
    fix32_t turnAngle_1840_;
    fix32_t npcAngleToUse_1844_;
    int unknown_1848_[4]; // probably viewport, and probably a struct
    int unknown_1858_;
    int unknown_185c_;
    int mojiArg1_1860_;
    int mojiArg2_1864_;
    int unknown_1868_;
    int waitTimeLengths_[4]; // func_02045824, used by <TIME=..> tag
    unsigned short color_187c_;
    unsigned short recoveryAmount_187e_; // amount of HP and MP to restore (usually 999)
    char unk_187e[0x1882 - 0x1880];
    char buffer_1882_[0x80];
    char buffer_1902_[0x40];
    short unknown_1942_;
    short unknown_1944_;
    short unknown_1946_;
    short questIndex_1948_;
    char voiceVolumeLookup_194a_[4];
    char tagSTArray1_194e_[4];
    char tagSTArray2_1952_[4];
    char tagSTArrayIndex_1956_;
    char unk_1957[0x59 - 0x57];
    char maybeSoundDuration_1959_;
    char unknown_195a_;
    char unknown_195b_;
    char unknown_195c_;
    char unk_195d[1];
    char autoValue_195e_; // populated by <AUTO=__> tags, I couldn't find any though
    char unk_195f[1];
    char pageTValue_1960_; // seems to be time until dialogue advances automatically?
    char unk_1961[1];
    char unknown_1962_;
    char unk_1963[1];
    ClipWindow windows_[2];
    char unknown_19ac_;
    char unknown_19ad_;
    char unknown_19ae_;
    char unknown_19af_;
    char unk_19b0[0x19b1 - 0x19b0];
    char winTag_19b1_;
    char unknown_19b2_;
    char unknown_19b3_;
    char unknown_19b4_;
    char unknown_19b5_;
    char maybeDoesNPCTurn_19b6_;
    char maybeDoesSoundPlay_19b7_;
    char cenTag_19b8_;
    char unknown_19b9_;
    char unk_19ba[0x19bb - 0x19ba];
    char skipTag_19bb_;
    char unknown_19bc_;
    char unk_19bd[1];
    char unknown_19be_;
    char unknown_19bf_;
    char unk_19c0[2];
    char unknown_19c2_;
    char unk_19c3[0x19c6 - 0x19c3];
    bool recoveryParam_19c6_;
    bool recoveryParam_19c7_; 
    char unk_19c8[0xcb - 0xc8];
    char unknown_19cb_;
    char unknown_19cc_;
    char unk_19cd[1];
    char unknown_19ce_;
    char unknown_19cf_;
    char unknown_19d0_;
    char unk_19d1[0x4 - 0x1];
    char unknown_19d4_;
    char unk_19d5[0xa - 0x5];
    short unknown_19da_;
    unsigned char fontIndex_19dc_;
    char unk_19dd[0xe0 - 0xdd];
    char substruct_19e0_[0x448]; // func_0202f1a4. At 0x440 contains a pointer to this+0x914, position of textbox?
    int unknown_1e28_;

    static TextManager* GetInstance();

    // usa: func_02042c68
    void Reset();

    // usa: func_0206831c
    void SubstituteValueTags(char* input, char* output);
    // substitute any character that is not: 1) within a <tag>; 2) a special character
    // such as ' ', '/' or '\n', or the literal R"\n"; or 3) a recognized tag
    // of the font; with the tag "< >"
    void SubstituteUnsupportedCharacters(char* input, char* output, int fontType);
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

    // missing processing step 02069234 that handles tags like <LEADER>
    // or <TMAP_SEC_1>, as well as some unknown stuff involving articles?

    // Replaces e.g. "<CAP>hello" with "Hello"
    void SubstituteCaps(char* input, char* output, int fontType);

    // func_0206973c: "PAGE_T=" // declare page end, auto-advance after given time
    // func_02069790: "PAGE>" // declare page end
    // func_020697b8: "AUTO=" // ???
    // func_0206980c: "ADD>" // maybe prevents box disappearing during cutscene? see e.g. Grand Lizzier
    // func_02069834: "TIME=" // used to add a delay/block input for a time (used for quest completion)
    // func_02069884: "PAD_WAIT>" // await input?
    // func_020698ac: "PAD_T=" // ???
    // func_020698fc: "PAD_WAIT_NOCUR>" // await input, don't show cursor/down arrow
    // func_02069924: "WIN_ON>"
    // func_0206994c: "WIN_OFF>"
    // func_02069974: "CEN_ON>"
    // func_0206999c: "CEN_OFF>"
    // func_020699c4: "ALL_RECOVER="
    // func_02069a4c: "ST="
    // func_02069ae4: "YESNO>"
    // func_02069b0c: "NOYES>"
    // func_02069b34: "YESNO_NOTSE>"
    // func_02069b5c: "YESNO_NOTSE_IIE>"
    // func_02069b84: "UKEYAME>" // seems to be a variant of yes/no? used in quests
    // func_02069bac: "LB_"
    // func_02069be4: "JP_"
    // func_02069c1c: "TURN="
    // func_02069ce0: "QUEST_SE>" // accepting a quest?
    // func_02069d08: "QUEST=" // makes quest appear on top screen?
    // func_02069d6c: "QUEST_HAN>" // complete quest?
    // func_02069d94: "QUEST_FAILED>"
    // func_02069dbc: "/QUEST>"
    // func_02069de4: "N_TURN>" // might mean 'don't turn'? (normally character turns toward player and stays that way)
    // func_02069e0c: "END_R_TURN>"
    // func_02069e34: "R_TURN>"
    // func_02069e5c: "TURN_P>" // probably turn to player?
    // func_02069e84: "EXC>" // exclamation/alert
    // func_02069eac: "QES>" // question/confused
    // func_02069ed4: "YES>"
    // func_02069efc: "NO>"
    // func_02069f24: "UKE>"
    // func_02069f4c: "YAME>"
    // func_02069f74: "END>"
    // func_02069f9c: "CLOSE>"
    // func_02069fc4: "SHAKE>" // shakes the text box (see e.g. heavy hatchet quest)
    void SubstituteControlTags(char* input, char* output);
    
    // Handles tags like <EXC> (play exclamation/alert), <COLOR=r,g,b> (presumably
    // sets text color but I've never seen it) that can be processed immediately, and
    // returns a pointer past the processed tags. (Stops as soon as it sees a tag
    // that can't be processed here)
    char* HandleImmediateTags(char* input);

    // usa: func_0206abf8
    int GetControlWordNumExtraWords(const char* input) const;
    // usa: func_0206ac6c
    bool IsNextCharacterControl(const char* input) const;
    // usa: func_0206ac94
    // might be supposed to take unsigned char* input
    char* FindControlCharacter(unsigned short control, char* input) const;
};