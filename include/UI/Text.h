#pragma once

#include "../Memory/SafeAllocator.h"
#include "std_library_functions.h"
#include "../Graphics/Vector.h"
#include "../Graphics/ClipWindow.h"
#include "DialogueOptions.h"
#include "../Util/Grammar.h"
#include "UIElement.h"
#include "../Graphics/Font.h"

// sizeof == 0x1c.
struct RenderGlyph
{
    const void* textureData;
    unsigned int textureVRAMOffset;
    int bitAllocationIndex;
    int drawX;
    int drawY;
    short drawZ;
    unsigned char unk_16_low : 1;
    unsigned char unk_16_1 : 1;
    unsigned char unk_16_2 : 1;
    unsigned char fontIndex : 1;
    unsigned char indexInFontFile;
    const FontIndexFile::Glyph* indexGlyph;

    void Reset();
    void Reset2();

    const char* GetGlyphTag() const;
    void Populate(unsigned char glyphIndexInFile, const FontIndexFile::Glyph* glyph, 
        const void* pixels, int bitAllocIndex, int textureVRAMOff, unsigned char fontIndex);

    void UploadToVRAM();

    // uses 3D system
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
    ControlWord_YesNo_NotSe = 0xff06, // yes/no but doesn't play a sound effect when you press A
    ControlWord_YesNo_NotSe_IIE = 0xff07, // yes/no but doesn't play a sound effect when you press A on 'No'
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

    ControlWord_Unknown_ff19 = 0xff19,
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

    // These are used for control flow: write e.g. <LB_A> to create a label,
    // and use <JP_A> to jump to that label. It appears that A to O are supported.
    // Indeed <JP_[char]> has code JPBase + ([char] - 0x40). I guess technically
    // JP_@ and LB_@ could be used too
    ControlWord_LBBase = 0xffd0,
    ControlWord_JPBase = 0xffe0,
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
unsigned char GetFontIDBySize(int size);
// usa: func_020421b0
const unsigned char* GetWindowBorderTile(int id);

// usa: func_020424ac
// returns a pointer to the '>' closing all open tags. (i.e. add 1 to go past
// the tag entirely)
char* GetTextTagEnd(char*);
inline const char* GetTextTagEnd(const char* x) { return GetTextTagEnd(const_cast<char*>(x)); }
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
    static inline bool IsControlWordJump(unsigned short ctrl)
    {
        return (ctrl & 0xffe0) == 0xffe0;
    }

    static inline bool IsControlWordLabel(unsigned short ctrl)
    {
        return (ctrl & 0xffd0) == 0xffd0;
    }

    static inline unsigned short GetJumpControlIndex(unsigned short ctrl)
    {
        return ctrl ^ 0xffe0;
    }

public:
    Noun* actors_0_[2];
    Noun* actions_8_[2];
    Noun* targets_10_[2];
    Noun* items_18_[2];
    Noun* monsters_20_[2];
    Noun* vocation_28_;
    Noun* reflex_2c_;
    char unknown_30_;
    char unknown_31_;
    char pad_32[2];
    UIElement* mainUIElement_34_;
    void* unknown_38_;
    int unknown_3c_;
    int unknown_40_;
    int unknown_44_;
    char* fullMessage_48_;
    // up to 0x80 different glyphs can be displayed at once (see the
    // RenderGlyph array below), this stores indices into that array
    // for each character of the message. Shared between speaker and
    // main message
    unsigned short* glyphIndexList_4c_;
    char* currentPageMessage_50_;
    char* unknown_54_;
    char* unknown_58_; // accessed as unsigned in 0206b224
    // unknown pointer (copied from 02107824) of size 0x960
    void* ptr_5c;
    // holds pointer at 02107818, size 0x960
    char* pageScratchBuffer_60_;
    // holds pointer at 0210780c, size 0x960
    char* scratchBuffer_64_;
    unsigned int length_68_;
    unsigned int glyphIndexListLength_6c_;
    unsigned int unknown_70_;
    unsigned int unknown_74_;
    unsigned int unknown_78_;
    int unknown_7c_;
    char unk_80[4];
    int mainMessageStartOffset_84_;
    unsigned int glyphIndexListMainMessageStart_88_;
    int unknown_8c_;
    DialogueOptionManager dialogueOptions_90_; // func_0205c790
    int unknown_2c8_;
    unsigned char substruct_2cc_[4];
    int unknown_2d0_;
    // has 0x400 bytes allocated to it
    SafeAllocator* pAllocator_2d4_;
    int unknown_2d8_;
    int unknown_2dc_;
    int unknown_2e0_;
    short unknown_2e4_;
    char unknown_2e6_;
    char unknown_2e7_;
    int unknown_2e8_;
    char tmapSecStrings_2ec_[7][0x40];
    char customStrings_4ac_[16][0x40];
    int unknown_8ac_;
    // not sure about size. Text can have markers <val_1>, <val_2> or similar
    // which get replaced by game quantities e.g. amount of gold, this array
    // stores the replacement values
    int valueLookup[16]; // @ 0x8b0
    char unk_8f0[1];
    unsigned char maybeValuePaddingModes[16];
    char unk_901[1];
    unsigned char valueUnknownArray[16];
    unsigned char unknown_912_; // 4 when current line has a speaker, 3 when not?
    char unk_913[1];
    TextBox mainTextBox_914_; // func_02042fcc 
    TextBox speakerTextBox_930_;
    int unknown_94c_;
    int unknown_950_;
    int unknown_954_;
    int unknown_958_;
    char substructOrArray_95c_[0x20];
    char substructOrArray_97c_[0x10];
    int unknown_98c_;
    int centerX_990_;
    int unknown_994_;
    int unknown_998_;
    int unknown_99c_;
    // should be an enum.
    // 0 = waiting for A (no cursor),
    // 1 = waiting for A (with cursor),
    // 2 = similar to above, used by zoom,
    // 3 = continuation of message?
    // 4 = auto-advance (<AUTO=...>),
    // 5 = ?,
    // 6 = awaiting yes/no or uke/yame choice
    int pauseType_9a0_;
    int gyouParameter_9a4_;
    char unk_9a8[8];
    int unknown_9b0_;
    int numGlyphsInUse_9b4_;
    // holds data about glyphs that are used in the current message
    RenderGlyph glyphs_[0x80];
    // number of times a glyph is used within message
    unsigned char glyphRefcounts_17b8_[0x80];
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
    int waitTimeLengths_186c_[4]; // func_02045824, used by <TIME=..> tag
    unsigned short color_187c_;
    unsigned short recoveryAmount_187e_; // amount of HP and MP to restore (usually 999)
    char partyMemberIndex_1880_;
    char unk_1881[1];
    unsigned short indexBuffer_1882_[0x40];
    char buffer_1902_[0x40];
    unsigned short unknown_1942_;
    short speakerNameWidth_1944_;
    short unknown_1946_;
    short questIndex_1948_;
    char voiceVolumeLookup_194a_[4];
    char tagSTArray1_194e_[4];
    char tagSTArray2_1952_[4];
    char tagSTArrayIndex_1956_;
    char unknown_1957_;
    char unknown_1958_;
    char maybeSoundDuration_1959_;
    char unknown_195a_;
    unsigned char effects_195b_;
    char unknown_195c_;
    // low 2 bits determine which offset among (-2, 0), (0, 0), (0, -3), (0, 0) to apply to speaker name
    unsigned char unknown_195d_;
    char autoValue_195e_;
    unsigned char autoRemainingTime_195f_;
    char pageTValue_1960_; // seems to be time until dialogue advances automatically?
    char unk_1961[1];
    unsigned char unknown_1962_;
    char unk_1963[1];
    ClipWindow windows_[2];
    char unknown_19ac_;
    char waitTimeQueueIndex_19ad_;
    char unknown_19ae_;
    char unknown_19af_;
    unsigned char unknown_19b0_;
    unsigned char showWindow_19b1_; // probably bool
    char unknown_19b2_;
    char unknown_19b3_;
    char unknown_19b4_;
    char unknown_19b5_;
    char maybeDoesNPCTurn_19b6_;
    char maybeDoesSoundPlay_19b7_;
    char cenTag_19b8_;
    char unknown_19b9_;
    unsigned char unknown_19ba_;
    char skipTag_19bb_;
    char unknown_19bc_;
    char unknown_19bd_;
    char unknown_19be_;
    char unknown_19bf_;
    bool maybeBoxShaking_19c0_;
    char unknown_19c1_;
    unsigned char speakerDifference_19c2_;
    unsigned char unknown_19c3_;
    unsigned char unknown_19c4_;
    char unknown_19c5_;
    bool recoveryParam_19c6_;
    bool recoveryParam_19c7_; 
    char unknown_19c8_;
    char unknown_19c9_;
    char unknown_19ca_;
    unsigned char unknown_19cb_;
    unsigned char unknown_19cc_;
    unsigned char unknown_19cd_;
    bool unknown_19ce_;
    char unknown_19cf_;
    unsigned char unknown_19d0_;
    char unknown_19d1_;
    char unknown_19d2_;
    char unknown_19d3_;
    char unknown_19d4_;
    // Q: does this count for a group of enemies too?
    // I couldn't find any uses of the <IF_TARGET_PARTY> tag anywhere...
    bool targetIsParty_19d5_;
    bool actorIsParty_19d6_;
    bool multipleTargets_19d7_;
    // used by <IF_TARGET_MIXED> but I see no usage of this tag
    bool maybeTargetingMultipleGroups_19d8_;
    char unk_19d9[1];
    short unknown_19da_;
    unsigned char fontIndex_19dc_;
    char unk_19dd[3];
    struct Substruct_19e0
    {
        struct InnerElement { char unk_0[0x88]; } elements[8];
        TextBox* pCurrentBox; 
        char unk_444[4];
    } substruct_19e0_; // func_0202f1a4
    void* unknown_1e28_;

    static TextManager* GetInstance();

    // usa: func_02042a90
    int AcquireGlyphVRAM(unsigned char* bitfield, unsigned int numBytes);
    // usa: func_02042af4
    void ReleaseGlyphVRAM(unsigned char* bitfield, int idx);

    // usa: func_02042b10
    // sets the pointer in substruct_19e0 at offset 0x440
    void SetActiveTextBox(TextBox*);
    // usa: func_02042b1c
    void SetMainTextBoxActive();

    // usa: func_02042c68
    void Reset();

    // usa: func-020439b0
    void RefreshGraphics(bool arg);

    // usa: func_02044420
    int GetControlWordNumExtraWords(const void* text) const;

    // usa: func_02044494
    // identical to func_0206ac6c, might take unsigned short* instead?
    bool IsNextCharacterControlAlt(const void* text) const;

    // usa: func_0204483c
    // replaces TIME, RECT, WIN, CEN, MOJI and SKIP tags as well as newlines
    // with control characters
    void ConvertBoxModifierTags(const char* in, char* out);

    // usa: func_020449a4
    // process modifiers like <CEN> at the start of some text and return
    // a pointer past these
    const char* HandleImmediateBoxTags(const char* in);

    // usa: func_0204500c
    // graphicsMode = 0 to use the 3D rendering engine, = 1 to use
    // the 2D engine
    void ShowMessage(const char* msg, int graphicsMode, int width);

    // usa: func_020457e8
    void RecordWaitTimeValue(int value);
    // usa: func_02045824
    // slot == -1 to save to all 4 slots
    void RecordWaitTimeValue(int value, int slot);

    // usa: func_020459b8
    int LoadNewGlyph(unsigned char glyphIndex, const FontIndexFile::Glyph* glyph, unsigned char fontIndex);

    // usa: func_02045c00
    int GetGlyphIndex(const FontIndexFile::Glyph* glyph);
    // usa: func_02045cac
    void ReleaseAllGlyphs();

    // usa: func_02045d14
    // converts the text into the 16-bit index encoding (written into outWords)
    // and returns the number of words written
    unsigned short AcquireText16Bit(const char* text, unsigned short* outWords, unsigned char fontIndex);
    int MeasureText16Bit(const unsigned short* words, int numWords, unsigned char fontIndex);
    
    // usa: func_02045f3c
    // args 5 and 6 are unused, in 2D mode only white and green text is supported,
    // and if you repeat the first character 10 times the function will return early. wtf?
    void DrawPreprocessedText(const unsigned short* words, int x, int y, unsigned short color,
        int, int, bool use2D, unsigned char fontIndex, int lineSpacing);
    
    // usa: func_020462d0
    void ReleaseText16Bit(unsigned short* words, int len);

    // usa: func_020652d0
    void ShowBinaryOptionDialogue(int defaultOption, const char* optionA, const char* optionB);

    // usa: func_020654b8
    void RenderGlyph2D(UIElement* renderer, RenderGlyph* glyph, short x, short y, unsigned char colorIndex);

    // usa: func_02065920
    void AdvanceAutoTimer(unsigned int ticks);

    // usa: func_02066a6c
    // The bool probably has some deeper meaning
    void DrawSpeakerName(bool noOffsets);

    // usa: func_020676e0
    void CheckAndHandleDialogueOption();

    // usa: func_020678bc
    // 0 = yes, 1 = no, -1 = undecided, -2 = cancelled (no)
    void ProcessYesNoInput(int input);
    // usa: func_020679d4
    // 0 = yes, 1 = no, -1 = undecided, 2 = cancelled (no)
    void ProcessUkeYameInput(int input);

    // usa: func_02067a9c
    bool IsNextCharacterInteractionControl(const void* input) const;
    // usa: func_02067ad8
    bool IsNextCharacterControlOrJump(const void* input) const;

    // usa: func_0206831c
    void SubstituteValueTags(const char* input, char* output);
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
    void SubstituteNamesAndArticles(char* input, char* output, int fontType);

    // Replaces e.g. "<CAP>hello" with "Hello"
    void SubstituteCaps(char* input, char* output, int fontType);

    // func_0206973c: "PAGE_T=" // declare page end, auto-advance after given time
    // func_02069790: "PAGE>" // declare page end
    // func_020697b8: "AUTO=" // also auto-advances, used with 'but the enemy are too stunned to move'
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
    // func_02069fc4: "SHAKE>" // shakes the text box (see e.g. heavy hatchet quest, aquila gets kafrizzled)
    void SubstituteControlTags(char* input, char* output);
    
    // Handles tags like <EXC> (play exclamation/alert), <COLOR=r,g,b> (presumably
    // sets text color but I've never seen it) that can be processed immediately, and
    // returns a pointer past the processed tags. (Stops as soon as it sees a tag
    // that can't be processed here)
    char* HandleImmediateTags(char* input);

    // usa: func_0206a974
    void LoadFirstPage(const char* fullMessage, int boxWidth);

    // usa: func_0206aa28
    void CopyPage();
    // usa: func_0206abf8
    int GetControlWordNumExtraWords(const char* input) const;
    // usa: func_0206ac6c
    bool IsNextCharacterControl(const char* input) const;
    // usa: func_0206ac94
    // might be supposed to take unsigned char* input
    char* FindControlCharacter(unsigned short control, char* input) const;

    // usa: func_0206ad74
    void ComputeSpeaker();

    // usa: func_0206b104
    void ReleaseMainMessageGlyphs();
    // usa: func_0206b198
    // decreases the refcount for a character (ascii or control)
    void ReleaseGlyphByIndex(unsigned short ch);
    // usa: func_0206b224
    void LoadNextPage();
    // usa: func_0206b2c8
    // Specify the response as a control word (e.g. 0xff14 for yes)
    void SubmitQuestionResponse(unsigned short response, char* searchStart);

    // usa: func_0206b37c
    int MeasureLine(const char* text) const;
    // usa: func_0206b528
    void GetLineCenteringOffsets(int* outXs, int* outYs) const;
    // usa: func_0206b61c
    // bit speculative as I haven't seen this called anywhere
    void AdvanceLine();

    // usa: func_0206b7b0
    void PositionOptionDialogue();

    // usa: func_0206b804
    void GetBoxShakeOffset(int* outX, int* outY);

    // usa: func_0206b974
    void CalculateNewlines(bool arg2, char* input, char* output, int boxWidth, int margin, bool arg7, unsigned char fontType);
};

void CapitalizeFirstGlyph(const char* input, char* output, int fontIndex);