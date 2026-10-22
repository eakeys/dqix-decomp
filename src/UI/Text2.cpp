#include "UI/Text.h"
#include "std_library_functions.h"
#include "Util/StringTests.h"
#include "GameState/GameState.h"
#include "NitroSDK/Graphics.h"
#include <globaldefs.h>

void SetRenderGlyphPosition(RenderGlyph* glyph, int x, int y, short z);

extern "C"
{
    // probably atoi
    int func_02005a94(const char*);
    // probably atof
    double func_020055d4(const char*);

    // get language
    int func_0200fb08(GameState*);
    GameResources* func_0200fb8c(GameState*);
    void* func_020100f8(GameState*);
    // get party data struct?
    unsigned char* func_02010828(GameState*);
    // get total number of player characters, including companions
    // and wifi players. (Q: Does this count Ivor?)
    unsigned char func_02011538(GameState*);

    void func_0202f404(void*, int, int);

    void* func_0203dce4(void*, int);
    void* func_0203cf4c();
    Vector3fix func_020406f8(void*);

    // set textbox dimensions?
    void func_02042b98(TextManager*, int, int, int, int);
    void func_02042c24(void*, short, short);
    void func_02043124(TextManager*);
    void func_02043204(TextManager*);
    void func_020457e8(TextManager*, int);

    int func_020546a8(PartyMember*);
    void func_0205c77c(void*, int);
    void func_0205c840(void*);
    void func_0205c904(DialogueOptionManager*);
    void func_0205cc50(DialogueOptionManager*, int, int);
    void func_0205cd94(DialogueOptionManager*);
    bool func_0205cde8(DialogueOptionManager*);
    // play sound effect?
    void func_0205eaa0(void*, int effect, int);

    void func_02066cc0(void*, short*, short*);

    // probably get quest manager instance or something?
    void* func_02094d6c();
    // maybe convert quest ID to real quest ID?
    unsigned char func_020965c0(void*, unsigned char);

    // alternative strlen implementation
    int func_020d2ff0(const char*);
}

extern char data_02108760[]; // sound effect system?

template<class FuncPtr>
struct TextProcessingCallback
{
    const char* tag;
    FuncPtr callback;
};

struct Struct_020e7e04
{
    char unk_0[2];
    unsigned short newlineWord;
    char unk_4[0x8];
    char shakeXValues[12];
    char shakeYValues[12];
    short array_24[2];
} extern const data_020e7e04;

extern const char data_020e7e10[12];
extern const char data_020e7e1c[12];
extern const char data_020e7e30[5];
extern const char data_020e7e35[7];
extern const short data_020e7e44[];
extern const short data_020e7e4c[];
extern const char* data_020e7e74[6];

extern char data_020f0746[]; // "<INN="
extern char data_020f074c[]; // "<SHOP"
extern char data_020f0752[]; // "TURN="
extern char data_020f0758[]; // "TIME="
extern char data_020f075e[]; // "SIZE="
extern char data_020f0764[]; // "ACTOR"
extern char data_020f076a[]; // "PAGE>"
extern char data_020f0770[]; // "AUTO="
extern char data_020f0776[]; // "PAD_T="
extern char data_020f077d[]; // "CLOSE>"
extern char data_020f0784[]; // "SHAKE>"
extern char data_020f078b[]; // "/TITLE"
extern char data_020f0792[]; // "YESNO>"
extern char data_020f0799[]; // "NOYES>"
extern char data_020f07a0[]; // "<BANK>"
extern char data_020f07a7[]; // "RENKIN"
extern char data_020f07ae[]; // "LEADER"
extern char data_020f07b5[]; // "QUEST="
extern char data_020f07bc[]; // "INDEF_"
extern char data_020f07c3[]; // "I_NAME"
extern char data_020f07ca[]; // "M_NAME"
extern char data_020f07d1[]; // "TARGET"
extern char data_020f07d8[]; // "ACTION"
extern char data_020f07df[]; // "REFLEX"
extern char data_020f07e6[]; // "WIN_ON>"
extern char data_020f07ee[]; // "CEN_ON>"
extern char data_020f07f6[]; // "/QUEST>"
extern char data_020f07fe[]; // "N_TURN>"
extern char data_020f0806[]; // "R_TURN>"
extern char data_020f080e[]; // "TURN_P>"
extern char data_020f0816[]; // "PAGE_T="
extern char data_020f081e[]; // "WIN_OFF>"
extern char data_020f0827[]; // "CEN_OFF>"
extern char data_020f0830[]; // "<CHURCH="
extern char data_020f0839[]; // "UKEYAME>"
extern char data_020f0842[]; // "TMAP_SEC"
extern char data_020f084b[]; // "PAD_WAIT>"
extern char data_020f0855[]; // "QUEST_SE>"
extern char data_020f085f[]; // "ADDRESSEE"
extern char data_020f0869[]; // "QUEST_HAN>"
extern char data_020f0874[]; // "END_R_TURN>"
extern char data_020f0880[]; // "ALL_RECOVER="
extern char data_020f088d[]; // "YESNO_NOTSE>"
extern char data_020f089a[]; // "QUEST_FAILED>"
extern char data_020f08a8[]; // "PAD_WAIT_NOCUR>"
extern char data_020f08dc[]; // "//"
extern char data_020f08df[]; // "%d<W=%d>"
extern char data_020f08e8[]; // "<W=%d>%d"
extern char data_020f08f1[]; // "%d"
extern char data_020f08f4[]; // "< >"
extern char data_020f08f8[]; // "_"
extern char data_020f08fa[]; // "<%s>"
extern char data_020f08ff[]; // "<IF_"
extern char data_020f0904[]; // "[WPN"
extern char data_020f0909[]; // "[ACTER" (typo?)
extern char data_020f0910[]; // "error"
extern char data_020f0916[]; // "CAP>"
extern char data_020f091b[]; // "<VOICE_VOLUME="
extern char data_020f092a[]; // "<ME_"
extern char data_020f092f[]; // "<SE_"
extern char data_020f0934[]; // "<N_TURN>"
extern char data_020f093d[]; // "<EXC>"
extern char data_020f0943[]; // "<QES>"
extern char data_020f0949[]; // "<RECT="
extern char data_020f0950[]; // "<WIN>"
extern char data_020f0956[]; // "<CEN>"
extern char data_020f095c[]; // "<GYOU="
extern char data_020f0963[]; // "<MOJI="
extern char data_020f096a[]; // "<COLOR="
extern char data_020f0972[]; // "<SKIP>"
extern char data_020f0979[]; // "*:"
extern char data_020f097c[]; // "*<:>"
extern char data_020f0981[]; // "<PAGE>"
extern char data_020f0988[]; // "//"
extern char data_020f098c[]; // "[-]"
extern char data_020f0990[]; // "-"
extern char data_020f0992[]; // "\n" (the special character, not backslash and an n)

void ShowBinaryOptionDialogue(DialogueOptionManager* dialogue, int defaultOption, const char* optionA, const char* optionB)
{
    dialogue->Reset();
    dialogue->gridMenu_1c_.touchInputEnabled_94_ = dialogue->gridMenu_1c_.keyInputEnabled_95_ = true;
    dialogue->gridMenu_1c_.touchInput_4_.SetSize(1, 2, BasicGridMenu::ElementOrdering_ColumnMajor);
    dialogue->gridMenu_1c_.keyInput_54_.SetSize(1, 2, BasicGridMenu::ElementOrdering_ColumnMajor);
    dialogue->gridMenu_1c_.touchInput_4_.SetElementCount(2);
    dialogue->gridMenu_1c_.keyInput_54_.SetElementCount(2);
    dialogue->AddOption(optionA);
    dialogue->AddOption(optionB);
    dialogue->textBox_b4_.x = 0;
    dialogue->textBox_b4_.y = 0;
    dialogue->ConfigureTouchRegions(true, -4);
    TextManager::GetInstance()->PositionOptionDialogue();
    dialogue->ConfigureTouchRegions(true, -4);
    dialogue->gridMenu_1c_.touchInput_4_.SetCurrentElementAndRemoveHighlight(defaultOption);
    dialogue->gridMenu_1c_.keyInput_54_.SetCurrentElementIndex(defaultOption);
    dialogue->unknown_234_ = true;
    func_0205c77c(&dialogue->substruct_4_, 1);
    dialogue->unknown_233_ = true;
    dialogue->unknown_234_ = true;
    func_0205c77c(&dialogue->substruct_4_, 1);
    dialogue->ShowBackground3D();
    if (GetCurrentMainScreen() == 1)
        dialogue->gridMenu_1c_.touchInputEnabled_94_ = false;
}

void TextManager::RenderGlyph2D(UIElement *renderer, RenderGlyph *glyph, short x, short preShiftY, unsigned char colorIndex)
{
    if (glyph->indexGlyph == NULL)
        return;

    const char* str = glyph->indexGlyph->tag;
    short y = preShiftY - (short)(mainUIElement_34_->topTile_ae_ * 8);
    
    unsigned char effects = TextManager::GetInstance()->unknown_195b_;

    short outX, outY;

    if (effects & 0x80)
    {
        for (unsigned char i = 0; i < 4; i++)
        {
            renderer->RenderText(x + data_020e7e4c[i], y + data_020e7e44[i], str,
                10, 1, &outX, &outY, fontIndex_19dc_);
        }
        renderer->RenderText(x + 1, y + 1, str, 10, colorIndex, &outX, &outY, fontIndex_19dc_);
    } else if (effects & 2)
    {
        int shadowX = x + 1, shadowY = y + 1;
        int mainX = x, mainY = y;
        renderer->RenderText(shadowX, shadowY, str, 10, 1, &outX, &outY, fontIndex_19dc_);
        renderer->RenderText(mainX, mainY, str, 10, colorIndex, &outX, &outY, fontIndex_19dc_);
    } else
    {
        renderer->RenderText(x, y, str, 10, colorIndex, &outX, &outY, fontIndex_19dc_);
    }
}

void TextManager::AdvanceAutoTimer(unsigned int ticks)
{
    if (unknown_19d0_ != 0 || pauseType_9a0_ != 4)
        return;

    unknown_912_ = 0;
    if (ticks < autoRemainingTime_195f_)
    {
        autoRemainingTime_195f_ -= ticks;
        return;
    }

    // timer ran out
    autoRemainingTime_195f_ = 0;
    unknown_19ae_ = 0;
    unknown_19af_ = 0;
    unknown_19cd_ = 1;
    unknown_40_ = 0;
    unknown_44_ = 0;

    ReleaseMainMessageGlyphs();
    pauseType_9a0_ = 5;
    LoadNextPage();
}

void TextManager::DrawSpeakerName(bool noOffsets)
{
    if (!unknown_19c4_)
        return;
    Substruct_19e0* boxHandler = &substruct_19e0_;
    int count;
    unsigned short* indices;
    RenderGlyph* glyph;
    int i;
    count = 0;
    indices = NULL;
    if (unknown_19c3_ != 0)
    {
        indices = indexBuffer_1882_;
        count = unknown_1942_ & 255;
    }
    if (indices != NULL) {
        int shakeX, shakeY;
        GetBoxShakeOffset(&shakeX, &shakeY);
        short x, y;
        func_02066cc0(boxHandler, &x, &y);
        y -= 19;
        boxHandler->pCurrentBox = &speakerTextBox_930_;
        if (boxHandler->pCurrentBox != NULL)
        {
            boxHandler->pCurrentBox->scaleX = 1 << 12;
            boxHandler->pCurrentBox->scaleY = 1 << 12;
        }
        if (boxHandler->pCurrentBox != NULL)
            boxHandler->pCurrentBox->unknown_8 = 2;
        if (noOffsets)
            func_02042c24(boxHandler, x, y);
        else
        {
            func_02042c24(boxHandler, x + shakeX, y + shakeY);
            func_0202f404(boxHandler, 1, 0);
        }
        short widthRounding = 8 - (speakerNameWidth_1944_ & 7);
        x += (widthRounding + 18) >> 1;
        y += 5;
        unsigned char fontWidth = GetFontSpaceSize(fontIndex_19dc_);
        for (i = 0; i < count; i++) {
            unsigned short code = indices[i];
            if ((code & 0xff00) == 0xff00)
            {
                if (code == ControlWord_End)
                    break;
                if (code == ControlWord_Unknown_ff19)
                {
                    x += fontWidth + 1;
                    continue;
                }
            }
            glyph = &glyphs_[code];
            if (noOffsets)
                RenderGlyph2D(mainUIElement_34_, glyph, x, y, 15);
            else
            {
                SetRenderGlyphPosition(glyph, x, y, 0);
                glyph->Draw(color_187c_, 0, 31);
            }
            if (glyph->indexGlyph != NULL)
                x += glyph->indexGlyph->width + 1;
        }
    }
    boxHandler->pCurrentBox = &mainTextBox_914_;
}

void SetRenderGlyphPosition(RenderGlyph* glyph, int x, int y, short z)
{
    glyph->drawX = x;
    glyph->drawY = y;
    glyph->drawZ = z;
}

void TextManager::ProcessYesNoInput(int input)
{
    unsigned short control = glyphIndexList_4c_[unknown_78_];
    bool silentYesNo = (control == ControlWord_YesNo_NotSe);
    bool silentNo = (control == ControlWord_YesNo_NotSe_IIE);

    switch (input)
    {
    case 0: // yes
        ReleaseMainMessageGlyphs();
        if (!silentYesNo) {
            func_0205eaa0(data_02108760, 1, 0);
        }
        SubmitQuestionResponse(ControlWord_Yes, unknown_58_);
        dialogueOptions_90_.Release();
        unknown_19cd_ = 1;
        unknown_40_ = 0;
        break;
    case 1: // no
        if (!silentYesNo && !silentNo)
            func_0205eaa0(data_02108760, 1, 0);
        // fallthrough
    case -2: // cancel (no)
        unknown_954_ = 1;
        ReleaseMainMessageGlyphs();
        SubmitQuestionResponse(ControlWord_No, unknown_58_);
        dialogueOptions_90_.Release();
        unknown_19cd_ = 1;
        unknown_40_ = 0;
        break;
    default:
        break;
    }
}

void TextManager::ProcessUkeYameInput(int input)
{
    unsigned short control = glyphIndexList_4c_[unknown_78_];

    switch (input)
    {
    case 0: // yes (uke)
        ReleaseMainMessageGlyphs();
        func_0205eaa0(data_02108760, 1, 0);
        SubmitQuestionResponse(ControlWord_Uke, unknown_58_);
        dialogueOptions_90_.Release();
        unknown_19cd_ = 1;
        unknown_40_ = 0;
        break;
    case 1: // no (yame)
        func_0205eaa0(data_02108760, 1, 0);
        // fallthrough
    case -2: // cancel (no/yame)
        unknown_954_ = 1;
        ReleaseMainMessageGlyphs();
        SubmitQuestionResponse(ControlWord_Yame, unknown_58_);
        dialogueOptions_90_.Release();
        unknown_19cd_ = 1;
        unknown_40_ = 0;
        break;
    default:
        break;
    }
}

bool TextManager::IsNextCharacterInteractionControl(const void *input) const
{
    unsigned short word;
    memcpy(&word, input, 2);
    if (word >= 0xff01 && word <= 0xff0d)
        return true;
    return false;
}

bool TextManager::IsNextCharacterControlOrJump(const void* input) const
{
    unsigned short word;
    memcpy(&word, input, 2);
    if (word >= 0xff01 && word <= 0xff18)
        return true;
    if (IsControlWordJump(word))
        return true;
    return false;
}

void CapitalizeSection(char* str, int numChars)
{
    if (str == NULL)
        return;
    for (int i = 0; i < numChars; i++)
    {
        if (*str == 0)
            break;

        int c = *str;
        if (c >= 'a' && c <= 'z')
            c -= ('a' - 'A');
        *str = c;
        str++;
    }
}

CBool DoesTextStartWithVowel(const Noun* noun, const char* text, int fontType)
{
    if (noun != NULL && noun->startsWithVowel_)
        return true;

    if (text == NULL)
        return false;

    FontIndexFile::Glyph* glyph = GetNextFontGlyph(text, fontType);
    if (glyph != NULL)
        return glyph->isVowel;
    else
        return false;
}

CBool DoesTextStartWithFrenchVowel(const Noun* noun, const char* text, int fontType)
{
    if (noun != NULL && !noun->specialVowelBehavior_)
        return noun->startsWithVowel_;

    FontIndexFile::Glyph* glyph = GetNextFontGlyph(text, fontType);
    if (glyph != NULL)
    {
        if (glyph->isVowel)
            return true;
        char ch = *text;
        const char* nextText = text + glyph->tagLength;
        if (ch == 'h')
        {
            FontIndexFile::Glyph* nextGlyph = GetNextFontGlyph(nextText, fontType);
            if (nextGlyph != NULL && nextGlyph->isVowel)
            {
                return !DoesTextStartWithVowel(noun, nextText + nextGlyph->tagLength, fontType);
            }
        }
        else if (ch == 'y')
        {
            return !DoesTextStartWithVowel(noun, nextText, fontType);
        }
    }
    return false;
}

// Checks if it ends with upper or lower case s, x, z or eszett
CBool DoesGermanTextEndWithSLike(const char* text)
{
    if (func_0200fb08(GameState::GetInstance()) == 3)
    {
        char regularCharacters[7];
        COPY_ARRAY(regularCharacters, data_020e7e35);

        char eszett[5];
        COPY_ARRAY(eszett, data_020e7e30);

        int length = func_020d2ff0(text);
        if (length != 0)
        {
            char lastChar = text[length - 1];
            for (const char* regularChar = regularCharacters; *regularChar != 0; regularChar++)
            {
                char ch = *regularChar;
                if (ch == 0)
                    break;
                if (ch == lastChar)
                    return true;
            }

            if (length >= 4 && memcmp(text + (length - 4), eszett, 4) == 0)
                return true;
        }
    }
    return false;
}

void CapitalizeFirstGlyph(const char *input, char *output, int fontIndex)
{
    if (input == NULL)
        return;

    sprintf(output, input);
    if (*input == 0)
        return;
    FontIndexFile::Glyph* glyph = GetNextFontGlyph(input, fontIndex);
    if (glyph == NULL || !glyph->unk_5_bit_7)
        return;
    CapitalizeSection(output, glyph->tagLength);
}

int GetNextNumberInString(const char* str)
{
    while (*str != 0)
    {
        if (*str >= '0' && *str <= '9')
            return func_02005a94(str);
        str++;
    }
    return 0;
}

void ProcessVALTag(const char* input, char** ppOutput, TextManager* manager, int index)
{
    int lookupIndex = GetNextNumberInString(input);
    
    int value = manager->valueLookup[lookupIndex - 1];
    int unknownThing = manager->valueUnknownArray[lookupIndex - 1];
    int paddingMode = manager->maybeValuePaddingModes[lookupIndex - 1];

    if (unknownThing != 0)
    {
        int W = unknownThing * 8 - MeasureNumberTextWidth(0, value);
        switch (paddingMode)
        {
        case 0: // pad on the right?
            *ppOutput += sprintf(*ppOutput, data_020f08df, value, W);
            break;
        case 1: // pad on the left?
            *ppOutput += sprintf(*ppOutput, data_020f08e8, W, value);
            break;
        default:
            *ppOutput += sprintf(*ppOutput, data_020f08f1, value);
        }
    }
    else
    {
        *ppOutput += sprintf(*ppOutput, data_020f08f1, value);
    }
}

typedef void(*PFNProcessVALTag)(const char*, char**, TextManager*, int);
extern const TextProcessingCallback<PFNProcessVALTag> data_020e7e5c[];

void TextManager::SubstituteValueTags(const char *input, char *argOutput)
{
    if (input == NULL || argOutput == NULL)
        return;
    char* output = argOutput;

    while (true)
    {
        if (*input == 0)
            break;
        
        if (*input == '<')
        {
            const char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL)
            {
                bool processedTag = false;
                const TextProcessingCallback<PFNProcessVALTag>* candidate;
                const char* tagInterior = input + 1;
                
                for (candidate = data_020e7e5c; candidate->tag != NULL; candidate++)
                {
                    if (!DoesStringBeginWith(input + 1, candidate->tag))
                        continue;
                    if (candidate->callback != NULL)
                    {
                        int valueIndex = 0;
                        int tagPrefixLength = func_020d2ff0(candidate->tag);
                        if (tagInterior[tagPrefixLength] >= '1' && tagInterior[tagPrefixLength] <= '9')
                            valueIndex = func_02005a94(tagInterior + tagPrefixLength) - 1;
                        candidate->callback(tagInterior, &output, this, valueIndex);
                    }
                    input = tagEnd + 1;
                    processedTag = true;
                    break;
                }

                if (processedTag)
                    continue;
            }
        }

        *output = *input;
        input++;
        output++;
    }

    *output = *input;
}

void TextManager::SubstituteUnsupportedCharacters(char* input, char* output, int fontType)
{
    if (input == NULL || output == NULL)
        return;

    while (true)
    {
        char nextInputChar = *input;
        if (nextInputChar == 0)
            break;

        if (input[0] == '\\' && input[1] == 'n')
        {
            output[0] = input[0];
            output[1] = input[1];
            output += 2;
            input += 2;
            continue;
        }

        FontIndexFile::Glyph* glyph = GetNextFontGlyph(input, fontType);
        if (glyph != NULL)
        {
            int tagLength = glyph->tagLength;
            memcpy(output, input, tagLength);
            input += tagLength;
            output += tagLength;
            continue;
        }

        if (nextInputChar == '<')
        {
            const char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL)
            {
                memcpy(output, input, tagEnd - input + 1);
                CapitalizeSection(output, tagEnd - input + 1);
                int copyLength = tagEnd - input;
                input += copyLength + 1;
                output += copyLength + 1;
                continue;
            }
        }

        if (nextInputChar == ' ' || nextInputChar == '/' || nextInputChar == '\n')
        {
            *output++ = *input++;
            continue;
        }

        memcpy(output, data_020f08f4, 3);
        input++;
        output += 3;
    }

    *output = *input;
}

int TextConditional_SinglePlayerMode(GameState* state, TextManager*, int)
{
    const unsigned char* partyData = func_02010828(state);
    unsigned char totalSize = func_02011538(state);
    if (partyData[0xf7c] == totalSize) // the party accounts for all chars: single player
        return 0;
    return 1;
}

int TextConditional_HostMale(GameState* state, TextManager*, int)
{
    PartyMember* hero = state->GetPartyMemberByIndex(0);
    if (hero != NULL)
    {
        return hero->partyMemberStats_->gender_;
    }
    return 0;
}

int TextConditional_AddresseeMale(GameState* state, TextManager*, int)
{
    PartyMember* protag = state->GetCartridgeProtagonist();
    if (protag != NULL)
    {
        return protag->partyMemberStats_->gender_;
    }
    return 0;
}

int TextConditional_HeroMale(GameState* state, TextManager*, int)
{
    PartyMember* hero = state->GetPartyMemberByIndex(0);
    if (hero != NULL)
    {
        return hero->partyMemberStats_->gender_;
    }
    return 0;
}

int TextConditional_LeaderMale(GameState* state, TextManager*, int)
{
    PartyMember* leader = state->GetPartyLeader();
    if (leader != NULL)
    {
        return leader->partyMemberStats_->gender_;
    }
    return 0;
}

int TextConditional_FemaleParty(GameState* state, TextManager*, int)
{
    const unsigned char* partyData = func_02010828(state);
    for (int i = 0; i < partyData[0xf7c]; i++)
    {
        const unsigned char* memberList = partyData + 0xf78;
        PartyMember* member = state->GetPartyMemberByIndex(memberList[i]);
        if (member != NULL)
        {
            if (member->partyMemberStats_->gender_ == 0) // male
                return 1; // not all-female
        }
    }
    return 0;
}

int TextConditional_Solo(GameState* state, TextManager* mgr, int)
{
    char partySize = mgr->unknown_30_;
    if (partySize < 0)
        partySize = func_02010828(state)[0xf7c];
    if (partySize != 1)
        return 1;
    return 0;
}

int TextConditional_SoloHost(GameState* state, TextManager*, int)
{
    char hostPartySize = 0;
    for (int i = 1; i < 4; i++)
    {
        PartyMember* companion = state->GetCompanionByIndex(i);
        if (companion != NULL && func_020546a8(companion) == 0)
            hostPartySize++;
    }
    hostPartySize++;
    if (hostPartySize != 1)
        return 1;
    return 0;
}

int TextConditional_Value1Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[0] == 1) ? 0 : 1; }
int TextConditional_Value2Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[1] == 1) ? 0 : 1; }
int TextConditional_Value3Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[2] == 1) ? 0 : 1; }
int TextConditional_Value4Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[3] == 1) ? 0 : 1; }
int TextConditional_Value5Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[4] == 1) ? 0 : 1; }

int TextConditional_Item1PluraleTantum(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->items_18_[0];
    if (noun != NULL)
    {
        if (noun->pluraleTantum_)
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_Monster1PluraleTantum(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->monsters_20_[0];
    if (noun != NULL)
    {
        if (noun->pluraleTantum_)
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_Item1Gender(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->items_18_[0];
    if (noun != NULL)
        return noun->gender_;
    return 0;
}

int TextConditional_Monster1Gender(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->monsters_20_[0];
    if (noun != NULL)
        return noun->gender_;
    return 0;
}

int TextConditional_Action1Gender(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->actions_8_[0];
    if (noun != NULL)
        return noun->gender_;
    return 0;
}

int TextConditional_Actor1Gender_MF(GameState*, TextManager* mgr, int)
{
    // This is expected to be a binary predicate but in principle could be
    // called on a neuter noun and still return 2. This might break something?
    Noun* noun = mgr->actors_0_[0];
    if (noun != NULL)
        return noun->gender_;
    return 0;
}

int TextConditional_Actor1Gender(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->actors_0_[0];
    if (noun != NULL)
        return noun->gender_;
    return 0;
}

int TextConditional_Target1Gender(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->targets_10_[0];
    if (noun != NULL)
        return noun->gender_;
    return 0;
}

int TextConditional_TargetParty(GameState*, TextManager* mgr, int)
{
    if (mgr->targetIsParty_19d5_)
        return 0;
    return 1;
}

int TextConditional_ActorParty(GameState*, TextManager* mgr, int)
{
    if (mgr->actorIsParty_19d6_)
        return 0;
    return 1;
}

int TextConditional_ActorIsTarget(GameState*, TextManager* mgr, int)
{
    Noun* actor = mgr->actors_0_[0];
    Noun* target = mgr->targets_10_[0];
    if (actor != NULL && target != NULL)
    {
        if (actor == target)
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_ActorIsPerson(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->actors_0_[0];
    if (noun != NULL)
    {
        if (noun->isPerson_)
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_TargetIsPerson(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->targets_10_[0];
    if (noun != NULL)
    {
        if (noun->isPerson_)
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_TargetSingle(GameState*, TextManager* mgr, int)
{
    if (mgr->multipleTargets_19d7_)
        return 1;
    return 0;
}

int TextConditional_TargetMixed(GameState*, TextManager* mgr, int)
{
    if (mgr->maybeTargetingMultipleGroups_19d8_)
        return 0;
    return 1;
}

int TextConditional_HeroStartsWithFrenchVowel(GameState* state, TextManager*, int fontIndex)
{
    PartyMember* hero = state->GetPartyMemberByIndex(0);
    if (hero != NULL)
    {
        if (DoesTextStartWithFrenchVowel(NULL, hero->baseStats_->name, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_CartridgeProtagonistStartsWithFrenchVowel(GameState* state, TextManager*, int fontIndex)
{
    PartyMember* protag = state->GetCartridgeProtagonist();
    if (protag != NULL)
    {
        if (DoesTextStartWithFrenchVowel(NULL, protag->baseStats_->name, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_PartyLeaderStartsWithFrenchVowel(GameState* state, TextManager*, int fontIndex)
{
    PartyMember* leader = state->GetPartyLeader();
    if (leader != NULL)
    {
        if (DoesTextStartWithFrenchVowel(NULL, leader->baseStats_->name, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_Item1StartsWithFrenchVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->items_18_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithFrenchVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_Item2StartsWithFrenchVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->items_18_[1];
    if (noun != NULL)
    {
        if (DoesTextStartWithFrenchVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_MonsterStartsWithFrenchVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->monsters_20_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithFrenchVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_VocationStartsWithFrenchVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->vocation_28_;
    if (noun != NULL)
    {
        if (DoesTextStartWithFrenchVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_ActorStartsWithFrenchVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->actors_0_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithFrenchVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_TargetStartsWithFrenchVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->targets_10_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithFrenchVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_Value1SingleFrench(GameState*, TextManager* mgr, int)
{
    if (mgr->valueLookup[0] != 0 && mgr->valueLookup[0] != 1)
        return 1;
    return 0;
}

int TextConditional_Value2SingleFrench(GameState*, TextManager* mgr, int)
{
    if (mgr->valueLookup[1] != 0 && mgr->valueLookup[1] != 1)
        return 1;
    return 0;
}

int TextConditional_Value3SingleFrench(GameState*, TextManager* mgr, int)
{
    if (mgr->valueLookup[2] != 0 && mgr->valueLookup[2] != 1)
        return 1;
    return 0;
}

int TextConditional_Value4SingleFrench(GameState*, TextManager* mgr, int)
{
    if (mgr->valueLookup[3] != 0 && mgr->valueLookup[3] != 1)
        return 1;
    return 0;
}

int TextConditional_Value5SingleFrench(GameState*, TextManager* mgr, int)
{
    if (mgr->valueLookup[4] != 0 && mgr->valueLookup[4] != 1)
        return 1;
    return 0;
}

int TextConditional_HeroStartsWithVowel(GameState* state, TextManager*, int fontIndex)
{
    // is this a bug? I don't think this tag is ever used so it doesn't come up
    PartyMember* hero = state->GetCartridgeProtagonist();
    if (hero != NULL)
    {
        if (DoesTextStartWithVowel(NULL, hero->baseStats_->name, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_PartyLeaderStartsWithVowel(GameState* state, TextManager*, int fontIndex)
{
    PartyMember* leader = state->GetPartyLeader();
    if (leader != NULL)
    {
        if (DoesTextStartWithVowel(NULL, leader->baseStats_->name, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_Item1StartsWithVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->items_18_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_MonsterStartsWithVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->monsters_20_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_VocationStartsWithVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->vocation_28_;
    if (noun != NULL)
    {
        if (DoesTextStartWithVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_ActorStartsWithVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->actors_0_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_TargetStartsWithVowel(GameState*, TextManager* mgr, int fontIndex)
{
    Noun* noun = mgr->targets_10_[0];
    if (noun != NULL)
    {
        if (DoesTextStartWithVowel(noun, noun->singularName_, fontIndex))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_ItemEndsInSLikeGerman(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->items_18_[0];
    if (noun != NULL)
    {
        if (DoesGermanTextEndWithSLike(noun->singularName_))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_MonsterEndsInSLikeGerman(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->monsters_20_[0];
    if (noun != NULL)
    {
        if (DoesGermanTextEndWithSLike(noun->singularName_))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_ActorEndsInSLikeGerman(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->actors_0_[0];
    if (noun != NULL)
    {
        if (DoesGermanTextEndWithSLike(noun->singularName_))
            return 0;
        return 1;
    }
    return 0;
}

int TextConditional_TargetEndsInSLikeGerman(GameState*, TextManager* mgr, int)
{
    Noun* noun = mgr->targets_10_[0];
    if (noun != NULL)
    {
        if (DoesGermanTextEndWithSLike(noun->singularName_))
            return 0;
        return 1;
    }
    return 0;
}

// Takes a string of form
// .....[match]....[open]....[close]
// and cuts out [open]....[close] in place
// (no square brackets in the actual string)
// Returns a pointer past the end of [match].
// if open == close then only [open] will be cut out
char* RemoveTextBetweenTags(char* input, const char* match, const char* open, const char* close)
{
    char* searchStart = strstr(input, match);
    if (searchStart != NULL)
    {
        char* firstOpen = strstr(searchStart, open);
        if (firstOpen != NULL)
        {
            char* firstClose = strstr(firstOpen, close);
            if (firstClose != NULL)
            {
                input = searchStart + func_020d2ff0(match);
                char* afterClose = firstClose + func_020d2ff0(close);

                char* copyDest = input;
                char* copySource = input;
                while (true)
                {
                    if (copySource == firstOpen)
                        copySource = afterClose;
                    char ch = *copySource;
                    *copyDest = ch;
                    if (ch == 0)
                        break;
                    copyDest++;
                    copySource++;
                }
            }
        }
    }
    return input;
}

struct ConditionProcessor
{
    struct TagWordSet
    {
        char indices[4];
    };

    TagWordSet tagWordSets[4];
    int (*callback)(GameState*, TextManager*, int);
};
typedef char TagWord[12];

extern const TagWord data_020e80cc[];
extern const ConditionProcessor data_020e833c[];

void BuildConditionTag(const ConditionProcessor::TagWordSet& words, char* out)
{
    if (words.indices[0] < 0)
        return;

    char stringBuilder[48] = {0};
    const char* indexPtr = words.indices;
    for (int i = 0; i < 4; i++)
    {
        if (*indexPtr < 0)
            break;
        if (i != 0)
            strcat(stringBuilder, data_020f08f8);
        strcat(stringBuilder, data_020e80cc[*indexPtr]);
        indexPtr++;
    }
    sprintf(out, data_020f08fa, stringBuilder);
}

void TextManager::SubstituteConditionals(char *input, char *output, int fontType)
{
    GameState* gameState = GameState::GetInstance();
    while (true)
    {
        char nextChar = *input;
        if (nextChar == '\0')
            break;
        
        if (nextChar == '<' && CaseInsensitiveDoesStringBeginWith(input, data_020f08ff) && GetTextTagEnd(input) != NULL)
        {
            char* inputNoSkip = input;
            for (const ConditionProcessor* condition = data_020e833c; condition->callback != NULL; condition++)
            {
                char ifTag[48] = {0};
                BuildConditionTag(condition->tagWordSets[0], ifTag);
                if (CaseInsensitiveDoesStringBeginWith(input, ifTag))
                {
                    int evaluation = condition->callback(gameState, this, fontType);
                    char elseOrSecondTag[48] = {0};
                    char thirdTag[48] = {0};
                    char endifTag[48] = {0};
                    BuildConditionTag(condition->tagWordSets[1], elseOrSecondTag);
                    BuildConditionTag(condition->tagWordSets[2], thirdTag);
                    BuildConditionTag(condition->tagWordSets[3], endifTag);     
                    switch (evaluation)
                    {
                    case 0:
                        // turns <if>A<else>B<endif>blah into <if>Ablah within input,
                        // then input skips past the <if>
                        input = RemoveTextBetweenTags(input, ifTag, elseOrSecondTag, endifTag);
                        break;
                    case 1:
                        if (thirdTag[0] != '\0')
                        {
                            // <if>A<if2>B<if3>C<endif> becomes <if>A<if2>B
                            // and pointer skips to B
                            input = RemoveTextBetweenTags(input, elseOrSecondTag, thirdTag, endifTag);
                        }
                        else
                        {
                            // <if>A<else>B<endif> becomes <if>A<else>B
                            // and pointer skips to B
                            input = RemoveTextBetweenTags(input, elseOrSecondTag, endifTag, endifTag);
                        }
                        break;
                    case 2:
                        // <if1>A<if2>B<if3>C<endif> becomes <if1>A<if2>B<if3>C
                        // and pointer skips to C
                        input = RemoveTextBetweenTags(input, thirdTag, endifTag, endifTag);
                        break;
                    default: // assume <if> is the right one
                        input = RemoveTextBetweenTags(input, ifTag, elseOrSecondTag, endifTag);
                        break;
                    }   
                    break; // out of inner for loop
                }
            }
            if (input != inputNoSkip)
                continue;
        }

        *output = nextChar;
        output++;
        input++;
    }

    *output = '\0';
}

void ConvertHeroTag(const char* input, char** output, TextManager* mgr, int num)
{
    PartyMember* hero = GameState::GetInstance()->GetPartyMemberByIndex(0);
    if (hero == NULL)
        return;
    
    // silly matching hack
    BaseCombatStats* stats = *(BaseCombatStats* volatile*)&hero->baseStats_;
    *output += sprintf(*output, stats->name);
}

void ConvertLeaderTag(const char* input, char** output, TextManager* mgr, int num)
{
    PartyMember* leader = GameState::GetInstance()->GetPartyLeader();
    if (leader == NULL)
        return;
    
    // silly matching hack
    BaseCombatStats* stats = *(BaseCombatStats* volatile*)&leader->baseStats_;
    *output += sprintf(*output, stats->name);
}

void ConvertAddresseeTag(const char* input, char** output, TextManager* mgr, int num)
{
    PartyMember* addressee = GameState::GetInstance()->GetCartridgeProtagonist();
    if (addressee == NULL)
        return;
    
    // silly matching hack
    BaseCombatStats* stats = *(BaseCombatStats* volatile*)&addressee->baseStats_;
    *output += sprintf(*output, stats->name);
}

void ConvertTmapSecTag(const char* input, char** output, TextManager* mgr, int)
{
    int index = GetNextNumberInString(input) - 1;
    char* source = mgr->tmapSecStrings_2ec_[index];
    if (source == NULL)
        return;
    *output += sprintf(*output, source);
}

void ConvertStrTag(const char* input, char** output, TextManager* mgr, int)
{
    int index = GetNextNumberInString(input) - 1;
    char* source = mgr->customStrings_4ac_[index];
    if (source == NULL)
        return;
    *output += sprintf(*output, source);
}

struct NameTagProcessor
{
    const char* tag;
    void (*callback)(const char*, char**, TextManager*, int);
};

extern const NameTagProcessor data_020e7e8c[6];

struct GrammarTagProcessingData
{
    const char* tag;
    // 0 = definitiveness, 1 = article?, 2 = plural?, 3 = case, 
    // 4 = use item data, 5 = use monster data, 6 = use actor data
    // 7 = use target data, 8 = use action data, 9 = use reflex(?) data
    char infoType;
    // e.g. if infoType = 3, specifies which case to use
    char infoValue;
};

extern const GrammarTagProcessingData data_020e7f04[16];

void TextManager::SubstituteNamesAndArticles(char *input, char *argOutput, int fontType) 
{
    if (input == NULL || argOutput == NULL)
        return;
    char* output = argOutput;
    while (true)
    {
        char ch = *input;
        if (ch == 0)
            break;

        if (ch == '<')
        {
            char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL)
            {
                bool matchedTag = false;
                char* tagInterior = input + 1;
                for (const NameTagProcessor* candidate = data_020e7e8c; candidate->tag != NULL; candidate++)
                {
                    if (!DoesStringBeginWith(tagInterior, candidate->tag))
                        continue;
                    if (candidate->callback != NULL)
                    {
                        int numeric = 0;
                        int numberOffsetInTag = func_020d2ff0(candidate->tag);
                        if (tagInterior[numberOffsetInTag] >= '1' && tagInterior[numberOffsetInTag] <= '9')
                            numeric = func_02005a94(tagInterior + numberOffsetInTag) - 1;
                        candidate->callback(tagInterior, &output, this, numeric);
                    }
                    input = tagEnd + 1;
                    matchedTag = true;
                    break;
                }

                if (matchedTag)
                    continue;

                int indefinite = -1;
                int article = 0;
                int plural = -1;
                int gcase = -1;
                Noun* noun = NULL;

                GrammarTagProcessingData grammarTags[16];
                COPY_ARRAY(grammarTags, data_020e7f04);

                matchedTag = false;
                const GrammarTagProcessingData* grammarPart = grammarTags;
                for (;; grammarPart++)
                {
                    const char* candidateTag = grammarPart->tag;
                    if (candidateTag == NULL)
                        break;
                    int tagLength = func_020d2ff0(candidateTag);
                    if (memcmp(tagInterior, candidateTag, tagLength) != 0)
                        continue;
                    
                    matchedTag = true;
                    tagInterior += tagLength;
                    int nounIndex = 0;
                    
                    if (grammarPart->infoValue < 0 && *tagInterior >= '1' && *tagInterior <= '9')
                        nounIndex = func_02005a94(tagInterior) - 1;
                    switch (grammarPart->infoType)
                    {
                    case 0:
                        indefinite = grammarPart->infoValue;
                        break;
                    case 1:
                        article = grammarPart->infoValue;
                        break;
                    case 2:
                        plural = grammarPart->infoValue;
                        break;
                    case 3:
                        gcase = grammarPart->infoValue;
                        break;
                    case 4:
                        noun = items_18_[nounIndex];
                        break;
                    case 5:
                        noun = monsters_20_[nounIndex];
                        break;
                    case 6:
                        noun = actors_0_[nounIndex];
                        break;
                    case 7:
                        noun = targets_10_[nounIndex];
                        break;
                    case 8:
                        noun = actions_8_[nounIndex];
                        break;
                    case 9:
                        noun = reflex_2c_;
                        break;
                    }
                }

                if (noun != NULL)
                    output = AddArticleAndDeclineNoun(&output, noun, indefinite, article, plural, gcase);

                if (matchedTag)
                {
                    input = tagEnd + 1;
                    continue;
                }
            }
        }
        else if (ch == '[')
        {
            if (memcmp(input, data_020f0904, 4) == 0)
            {
                input += 4;
                PartyMember* member = (PartyMember*)GameState::GetInstance()->GetObject100(partyMemberIndex_1880_);
                if (member != NULL)
                {
                    intptr_t intptr_foo = (intptr_t)member->partyMemberStats_ + 0x294;
                    struct Foo { 
                        char unk_0[4];
                        const char* weaponName;
                        unsigned int thing_we_care_about : 4;
                        unsigned int other_shit : 28;
                        char unk_c[0xc];
                        short other_thing_we_care_about;
                    } *pFoo = (Foo*)intptr_foo;
                    if (pFoo != NULL && pFoo->other_thing_we_care_about > 0 && pFoo->thing_we_care_about == 0)
                    {
                        output += sprintf(output, pFoo->weaponName);
                    }
                }
            }
            else if (memcmp(input, data_020f0909, 6) == 0) // [ACTER= [sic]
            {
                input += 6;
                output += sprintf(output, tmapSecStrings_2ec_[3]);
            }
            else
            {
                input++;
                *output = ch;
                output++;
                continue;
            }
            
            while (*input != ']' && *input != 0)
                input++;

            if (*input == 0)
            {
                output += sprintf(output, data_020f0910);
                break;
            }
            else
            {
                input++;
                continue;
            }

        }

        *output = *input;
        output++;
        input++;
    }
    *output = 0;
}

void TextManager::SubstituteCaps(char *input, char *output, int fontType)
{
    while (true)
    {
        if (*input == 0)
            break;

        if (*input == '<')
        {
            const char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL && memcmp(input + 1, data_020f0916, 4) == 0)
            {
                input = (char*)tagEnd + 1;
                FontIndexFile::Glyph* glyph = GetNextFontGlyph(input, fontType);
                if (glyph != NULL && glyph->unk_5_bit_7)
                {
                    int tagLength = glyph->tagLength;
                    memcpy(output, input, tagLength);
                    CapitalizeSection(output, tagLength);
                    input += tagLength;
                    output += tagLength;
                }

                continue;
            }
        }

        *output = *input;
        output++;
        input++;
    }

    *output = 0;
}

int ReadNumbersInTag(const char* text, int* output, int readCount)
{
    for (int i = 0; i < readCount; i++)
    {
        // if this sees anything that isn't a number the loop just ends without
        // doing anything else
        if (*text != 0 && ((*text >= '0' && *text <= '9') || *text == '-'))
        {
            output[i] = func_02005a94(text);
            while (true)
            {
                if (*text == 0)
                    break;
                if (*text == ',' || *text == '>')
                {
                    text++;
                    break;
                }
                text++;
            }
        }
    }
    return 0;
}

int ConvertPageTTag(char** ppOutput, char* tagExtras)
{
    int duration;
    ReadNumbersInTag(tagExtras, &duration, 1);
    TextManager::GetInstance()->pageTValue_1960_ = duration;
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_PageT;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertPageTag(char** ppOutput, char* tageExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Page;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertAutoTag(char** ppOutput, char* tagExtras)
{
    int value;
    ReadNumbersInTag(tagExtras, &value, 1);
    TextManager::GetInstance()->autoValue_195e_ = value;
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Auto;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertAddTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Add;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertTimeTag(char** ppOutput, char* tagExtras)
{
    int duration;
    ReadNumbersInTag(tagExtras, &duration, 1);
    func_020457e8(TextManager::GetInstance(), duration);
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Time;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertPadWaitTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_PadWait;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertPadTTag(char** ppOutput, char* tagExtras)
{
    int duration;
    ReadNumbersInTag(tagExtras, &duration, 1);
    func_020457e8(TextManager::GetInstance(), duration);
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_PadT;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertPadWaitNoCursorTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_PadWaitNoCursor;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertWinOnTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_WinOn;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertWinOffTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_WinOff;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertCenOnTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_CenOn;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertCenOffTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_CenOff;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertAllRecoverTag(char** ppOutput, char* tagExtras)
{
    int params[3];
    ReadNumbersInTag(tagExtras, params, 3);
    TextManager* mgr = TextManager::GetInstance();
    unsigned short healAmount = params[2];
    bool firstBool = (bool)params[0];
    bool secondBool = (bool)params[1];
    mgr->recoveryParam_19c6_ = firstBool;
    mgr->recoveryParam_19c7_ = secondBool;
    mgr->recoveryAmount_187e_ = healAmount;
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_AllRecover;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertSTTag(char** ppOutput, char* tagExtras)
{
    int params[2];
    ReadNumbersInTag(tagExtras, params, 2);
    TextManager* mgr = TextManager::GetInstance();
    int secondArg = params[1];
    mgr->tagSTArrayIndex_1956_ %= 4;
    mgr->tagSTArray1_194e_[mgr->tagSTArrayIndex_1956_] = params[0];
    mgr->tagSTArray2_1952_[mgr->tagSTArrayIndex_1956_] = secondArg;
    mgr->tagSTArrayIndex_1956_++;

    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_ST;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertYesNoTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_YesNo;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertNoYesTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_NoYes;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertYesNoNotSeTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_YesNo_NotSe;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertYesNoNotSeIIETag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_YesNo_NotSe_IIE;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertUkeYameTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_UkeYame;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertLBTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_LBBase;
    controlWord += (unsigned short)(tagExtras[0] - 0x40);
    unsigned short copySource = controlWord;
    memcpy(dest, &copySource, 2);
    return 2;
}

int ConvertJPTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_JPBase;
    controlWord += (unsigned short)(tagExtras[0] - 0x40);
    unsigned short copySource = controlWord;
    memcpy(dest, &copySource, 2);
    return 2;
}

int ConvertTurnTag(char** ppOutput, char* tagExtras)
{
    float angles[1];

    for (int i = 0; i < 1; i++)
    {
        if (*tagExtras != 0 && *tagExtras >= '0' && *tagExtras <= '9')
        {
            angles[i] = (float)func_020055d4(tagExtras);

            while (true)
            {
                if (*tagExtras == 0)
                    break;
                if (*tagExtras == ',' || *tagExtras == '>')
                {
                    tagExtras++;
                    break;
                }
                tagExtras++;
            }
        }
    }

    TextManager* mgr = TextManager::GetInstance();
    mgr->turnAngle_1840_ = 4096.0f * angles[0];
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Turn;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertQuestSETag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_QuestSE;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertQuestTag(char** ppOutput, char* tagExtras)
{
    int internalQuestNumber;
    ReadNumbersInTag(tagExtras, &internalQuestNumber, 1);
    unsigned char converted = func_020965c0(func_02094d6c(), internalQuestNumber);
    TextManager::GetInstance()->questIndex_1948_ = converted;
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Quest;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertQuestHanTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_QuestHan;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertQuestFailedTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_QuestFailed;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertSlashQuestTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_SlashQuest;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertNTurnTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_NTurn;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertEndRTurnTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_EndRTurn;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertRTurnTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_RTurn;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertTurnPTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_TurnP;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertExclamationTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Exclamation;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertQuestionTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Question;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertYesTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Yes;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertNoTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_No;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertUkeTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Uke;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertYameTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Yame;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertEndTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_End;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertCloseTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Close;
    memcpy(dest, &controlWord, 2);
    return 2;
}

int ConvertShakeTag(char** ppOutput, char* tagExtras)
{
    char* dest = *ppOutput;
    unsigned short controlWord = ControlWord_Shake;
    memcpy(dest, &controlWord, 2);
    return 2;
}

struct ControlTagProc
{
    const char* tag;
    int (*callback)(char**, char*);
};
extern const ControlTagProc data_020e7f84[];

void TextManager::SubstituteControlTags(char *argInput, char *argOutput)
{
    if (argInput == NULL || argOutput == NULL)
        return;

    char* output = argOutput; // create local copy so can pass by reference
    char* input = HandleImmediateTags(argInput);
    unknown_19d4_ = true;
    unsigned short voiceVolumeIdx = 0;
    unsigned short finalControlWord;

    while (true)
    {
        if (*input == 0)
            break;

        if ((input[0] == '\\' && input[1] == 'n') || (input[0] == '\r' && input[1] == '\n'))
        {
            unsigned short controlWord = ControlWord_LineBreak;
            memcpy(output, &controlWord, 2);
            output += 2;
            input += 2;
            continue;
        }

        if (input[0] == '\n')
        {
            unsigned short controlWord = ControlWord_LineBreak;
            memcpy(output, &controlWord, 2);
            output += 2;
            input += 1;
            continue;
        }

        if (input[0] == '<')
        {
            char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL)
            {
                char* comparison = input + 1;
                bool matchedTag = false;
                for (const ControlTagProc* proc = data_020e7f84; proc->tag != NULL && proc->callback != NULL; proc++)
                {
                    if (!CaseInsensitiveDoesStringBeginWith(comparison, proc->tag))
                        continue;
                    char* tagParams = comparison + func_020d2ff0(proc->tag);
                    int writeLength = proc->callback(&output, tagParams);
                    output += writeLength;
                    input = tagEnd + 1;
                    matchedTag = true;
                    break;
                }

                if (matchedTag)
                    continue;

                const char* facilities[6]; // inn, church, bank, shop, renkin, ""
                COPY_ARRAY(facilities, data_020e7e74);
                for (const char** loopFacility = facilities; *loopFacility != NULL; loopFacility++)
                {
                    if (!DoesStringBeginWith(input, *loopFacility))
                        continue;
                    while (*input != '>')
                        input++;
                    input++;
                    break;
                }

                if (DoesStringBeginWith(input, data_020f091b))
                {
                    int tagLength = 14;
                    int voiceVolume = func_02005a94(input + 14);
                    
                    while (input[tagLength] != '>')
                    {
                        if (input[tagLength] == 0)
                            return;
                        tagLength++;
                    }
                    voiceVolumeLookup_194a_[voiceVolumeIdx] = voiceVolume;
                    unsigned short controlWord = ControlWord_VoiceVolumeBase + voiceVolumeIdx;
                    memcpy(output, &controlWord, 2);
                    input += tagLength + 1;
                    output += 2;
                    voiceVolumeIdx++;
                    voiceVolumeIdx &= 3;
                    continue;
                }

                if (DoesStringBeginWith(input, data_020f092a)) // <ME_
                {
                    int tagLength = 4;
                    int meParam = func_02005a94(input + 4);
                    while (input[tagLength] != '>')
                    {
                        if (input[tagLength] == 0)
                            return;
                        tagLength++;
                    }
                    unsigned short controlWord = ControlWord_MEBase + meParam;
                    memcpy(output, &controlWord, 2);
                    input += tagLength + 1;
                    output += 2;
                    continue;
                }

                if (DoesStringBeginWith(input, data_020f092f)) // <SE_
                {
                    int tagLength = 4;
                    int meParam = func_02005a94(input + 4);
                    while (input[tagLength] != '>')
                    {
                        if (input[tagLength] == 0)
                            return;
                        tagLength++;
                    }
                    unsigned short controlWord = ControlWord_SEBase;
                    short local_44[2];
                    unsigned short i = 0;
                    COPY_ARRAY(local_44, data_020e7e04.array_24);
                    
                    for (; local_44[i] >= 0; i++)
                    {
                        if (meParam == local_44[i])
                        {
                            controlWord += i;
                            break;
                        }
                    }
                    unsigned short localControlWord = controlWord;
                    memcpy(output, &localControlWord, 2);
                    input += tagLength + 1;
                    output += 2;
                    continue;
                }
            }
        }

        *output = *input;
        input++;
        output++;
    }

    finalControlWord = ControlWord_End;
    memcpy(output, &finalControlWord, 2);
    output += 2;
    *output = '\0';
}

char* TextManager::HandleImmediateTags(char* input)
{
    bool moreTagsToGo = true;
    maybeDoesNPCTurn_19b6_ = true;
    PartyMember* leader = GameState::GetInstance()->GetPartyLeader();
    void* maybeNPC = func_0203dce4(func_0203cf4c(), npcID_1838_);
    if (leader != NULL && maybeNPC != NULL)
    {
        Vector3fix playerPos = leader->position_;
        Vector3fix npcPos = func_020406f8(maybeNPC);
        
        npcAngleToUse_1844_ = fix32ReduceAngle0To2Pi(
            fix32_Atan2(playerPos.x - npcPos.x, playerPos.z - npcPos.z)
        );
        maybeDoesNPCTurn_19b6_ = true;
    }
    maybeSoundDuration_1959_ = 0;
    maybeDoesSoundPlay_19b7_ = false;
    while (moreTagsToGo)
    {
        if (DoesStringBeginWith(input, data_020f0934)) // <N_TURN>
        {
            npcAngleToUse_1844_ = npcPriorRotation_183c_;
            maybeDoesNPCTurn_19b6_ = false;
            input += 8;
            continue;
        }

        if (DoesStringBeginWith(input, data_020f093d)) // <EXC>
        {
            maybeSoundDuration_1959_ = 60;
            maybeDoesSoundPlay_19b7_ = true;
            unknown_195c_ = 0;
            input += 5;
            func_0205eaa0(data_02108760, 6, 0);
            continue;
        }

        if (DoesStringBeginWith(input, data_020f0943)) // <QES>
        {
            maybeSoundDuration_1959_ = 60;
            maybeDoesSoundPlay_19b7_ = true;
            unknown_195c_ = 1;
            input += 5;
            func_0205eaa0(data_02108760, 28, 0);
            continue;
        }

        if (CaseInsensitiveDoesStringBeginWith(input, data_020f0949)) // <RECT=
        {
            bool outOfTagParameters = false;
            
            int argReadOffset = 6;
            int totalTagLength = 6;
            
            while (input[totalTagLength] != '>')
            {
                if (input[totalTagLength] == 0)
                    return input + totalTagLength;
                totalTagLength++;
            }

            int rectX = func_02005a94(input + argReadOffset);
            while (input[argReadOffset] != ',')
            {
                if (input[argReadOffset] == '>')
                {
                    input += argReadOffset + 1;
                    outOfTagParameters = true;
                    break;
                }
                argReadOffset++;
            }
            if (outOfTagParameters)
                return input;

            argReadOffset++; // skip over the comma
            int rectY = func_02005a94(input + argReadOffset);
            while (input[argReadOffset] != ',')
            {
                if (input[argReadOffset] == '>')
                {
                    input += argReadOffset + 1;
                    outOfTagParameters = true;
                    break;
                }
                argReadOffset++;
            }
            if (outOfTagParameters)
                return input;

            argReadOffset++; // skip over the comma
            int rectWidth = func_02005a94(input + argReadOffset);
            while (input[argReadOffset] != ',')
            {
                if (input[argReadOffset] == '>')
                {
                    input += argReadOffset + 1;
                    outOfTagParameters = true;
                    break;
                }
                argReadOffset++;
            }
            if (outOfTagParameters)
                return input;

            argReadOffset++; // skip over the comma
            int rectHeight = func_02005a94(input + argReadOffset);

            if (rectX < 0)
                rectX = 0;
            if (rectX > 256)
                rectX = 256;
            if (rectY < 0)
                rectY = 0;
            if (rectY > 192)
                rectY = 192;
            
            if (rectWidth < 0)
                rectWidth = 256 - rectX;
            if (rectWidth > 256)
                rectWidth = 256 - rectX;
            if (rectHeight < 0)
                rectHeight = 192 - rectY;
            if (rectHeight > 192)
                rectHeight = 192 - rectY;

            func_02042b98(this, rectX, rectY, rectWidth, rectHeight);
            input += totalTagLength + 1;
            continue;
        }

        if (CaseInsensitiveDoesStringBeginWith(input, data_020f0950)) // <WIN>
        {
            showWindow_19b1_ = 0;
            input += 5;
            continue;
        }

        if (CaseInsensitiveDoesStringBeginWith(input, data_020f0956)) // <CEN>
        {
            cenTag_19b8_ = 1;
            input += 5;
            continue;
        }

        if (CaseInsensitiveDoesStringBeginWith(input, data_020f095c)) // <GYOU=
        {
            int tagLength = 6;
            int argument = func_02005a94(input + 6);
            while (input[tagLength] != '>')
            {
                if (input[tagLength] == 0)
                    return input + tagLength;
                tagLength++;
            }
            if (argument < 1)
                argument = 1;
            gyouParameter_9a4_ = argument;
            input += tagLength + 1;
            continue;
        }

        if (CaseInsensitiveDoesStringBeginWith(input, data_020f0963)) // <MOJI=
        {
            int argReadOffset = 6;
            int totalTagLength = 6;
            while (input[totalTagLength] != '>')
            {
                if (input[totalTagLength] == 0)
                    return input + totalTagLength;
                totalTagLength++;
            }

            int arg1 = func_02005a94(input + argReadOffset);
            if (arg1 <= 0)
                arg1 = 12;
            while (input[argReadOffset] != ',')
            {
                if (input[argReadOffset] == '>')
                {
                    input += argReadOffset + 1;
                    break;
                }
                argReadOffset++;
            }

            argReadOffset++;
            int arg2 = func_02005a94(input + argReadOffset);
            if (arg2 <= 0)
                arg2 = 16;

            mojiArg1_1860_ = arg1;
            mojiArg2_1864_ = arg2;
            input += totalTagLength + 1;
            continue;
        }
        
        if (CaseInsensitiveDoesStringBeginWith(input, data_020f096a)) // <COLOR=
        {
            int argReadOffset = 7;
            int totalTagLength = 7;
            while (input[totalTagLength] != '>')
            {
                if (input[totalTagLength] == 0)
                    return input + totalTagLength;
                totalTagLength++;
            }

            bool outOfTagParameters = false;
            int argRed = func_02005a94(input + argReadOffset);
            if (argRed < 0)
                argRed = 255;
            if (argRed > 255)
                argRed = 255;

            while (input[argReadOffset] != ',')
            {
                if (input[argReadOffset] == '>')
                {
                    input += argReadOffset + 1;
                    outOfTagParameters = true;
                    break;
                }
                argReadOffset++;
            }

            if (outOfTagParameters)
                continue;
            argReadOffset++;
            int argGreen = func_02005a94(input + argReadOffset);
            if (argGreen < 0)
                argGreen = 255;
            if (argGreen > 255)
                argGreen = 255;

            while (input[argReadOffset] != ',')
            {
                if (input[argReadOffset] == '>')
                {
                    input += argReadOffset + 1;
                    outOfTagParameters = true;
                    break;
                }
                argReadOffset++;
            }

            if (outOfTagParameters)
                continue;
            argReadOffset++;
            int argBlue = func_02005a94(input + argReadOffset);
            if (argBlue < 0)
                argBlue = 255;
            if (argBlue > 255)
                argBlue = 255;
            color_187c_ = (argRed >> 3) | ((argGreen >> 3) << 5) | ((argBlue >> 3) << 10);
            input += totalTagLength + 1;
            continue;
        }

        if (CaseInsensitiveDoesStringBeginWith(input, data_020f0972)) // <SKIP>
        {
            skipTag_19bb_ = false;
            input += 6;
            continue;
        }

        moreTagsToGo = false;
    }
    return input;
}

void TextManager::LoadFirstPage(const char* inFullMsg, int boxWidth)
{
    if (inFullMsg == NULL)
        return;
    char* copyDest = fullMessage_48_;
    length_68_ = func_020d2ff0(inFullMsg);
    memcpy(copyDest, inFullMsg, length_68_);
    unknown_54_ = unknown_58_ = fullMessage_48_;
    memset(glyphIndexList_4c_, 0, 0x280);
    memset(currentPageMessage_50_, 0, 0x280);
    CopyPage();
    CalculateNewlines(true, currentPageMessage_50_, pageScratchBuffer_60_, boxWidth, 0, true, 1);
    int finalCopyLength = func_020d2ff0(pageScratchBuffer_60_);
    memcpy(currentPageMessage_50_, pageScratchBuffer_60_, finalCopyLength);
}

void TextManager::CopyPage()
{
    unknown_7c_ = 0;
    glyphIndexListLength_6c_ = 0;
    unknown_70_ = 0;
    unknown_74_ = 0;
    unknown_78_ = 0;
    char* source = unknown_54_;
    char* dest = currentPageMessage_50_;
    bool foundNormalGlyph = false;
    while (true)
    {
        if (*source == 0)
            break;
        int controlWordByteCount = 0;
        if (IsNextCharacterControl(source))
        {
            controlWordByteCount = (GetControlWordNumExtraWords(source) + 1) * 2;
            memcpy(dest, source, controlWordByteCount);
            dest += controlWordByteCount;
            unknown_70_ += controlWordByteCount;
        }
        if (length_68_ < (source - fullMessage_48_))
        {
            unknown_998_ = 0;
            break;
        }
        if (IsNextCharacterControl(source))
        {
            if (IsNextCharacterInteractionControl(source))
            {
                if (foundNormalGlyph)
                {
                    unknown_58_ = source;
                    break;
                }
                unsigned char old = unknown_19cc_;
                func_02043204(this);
                func_02043124(this);
                unknown_19cc_ = old;
                break;
            }
            unsigned short control;
            memcpy(&control, source, 2);
            source += controlWordByteCount;
            if (IsControlWordJump(control))
            {
                control = GetJumpControlIndex(control) + 0xffd0;
                source = FindControlCharacter(control, fullMessage_48_);
                if (source == NULL)
                {
                    unsigned char old = unknown_19cc_;
                    func_02043204(this);
                    func_02043124(this);
                    unknown_19cc_ = old;
                    break;
                }
            }
        }
        else 
        {
            foundNormalGlyph = true;
            int copyLength = 1;
            FontIndexFile::Glyph* glyph = GetNextFontGlyph(source, fontIndex_19dc_);
            if (glyph != NULL)
                copyLength = glyph->tagLength;
            memcpy(dest, source, copyLength);
            dest += copyLength;
            unknown_70_++;
            source += copyLength;
        }
    }
    ComputeSpeaker();
}

int TextManager::GetControlWordNumExtraWords(const char *input) const
{
    int numExtra = 0;
    unsigned short controlWord;
    memcpy(&controlWord, input, 2);
    if (!IsNextCharacterControlAlt(&controlWord))
        return 0;

    switch (controlWord)
    {
    case ControlWord_Unknown_ff20:
    case ControlWord_Unknown_ff21:
    case ControlWord_Unknown_ff23:
    case ControlWord_Unknown_ff24:
        numExtra = 1;
        break;
    case ControlWord_Unknown_ff22:
    case ControlWord_Unknown_ff25:
        numExtra = 2;
        break;
    }
    return numExtra;
}

bool TextManager::IsNextCharacterControl(const char* input) const
{
    unsigned short word;
    memcpy(&word, input, 2);
    if ((word & 0xff00) == 0xff00)
        return true;
    return false;
}

char* TextManager::FindControlCharacter(unsigned short control, char* input) const
{
    if (input == NULL)
        return NULL;
    if (!IsNextCharacterControlAlt(&control))
        return NULL;

    unsigned int bytesSearched = 0;
    while (true)
    {
        if (*(unsigned char*)input == 0)
            break;

        if (IsNextCharacterControl(input))
        {
            if (memcmp(&control, input, 2) == 0)
                return input;
            int skipLength = 2 * GetControlWordNumExtraWords(input) + 2;
            input += skipLength;
            bytesSearched += skipLength;
            continue;
        }

        int glyphMarkupLength = 1;
        const FontIndexFile::Glyph* glyph = GetNextFontGlyph(input, fontIndex_19dc_);
        if (glyph != NULL)
            glyphMarkupLength = glyph->tagLength;
        
        bytesSearched += glyphMarkupLength;
        input += glyphMarkupLength;
        
        if (length_68_ < bytesSearched)
            break;
    }

    return NULL;
}

void TextManager::ComputeSpeaker()
{
    int speakerLength = 0;
    mainMessageStartOffset_84_ = 0;
    glyphIndexListMainMessageStart_88_ = 0;
    speakerNameWidth_1944_ = 0;
    unknown_1942_ = 0;
    unknown_1946_ = 0;

    unsigned char backupNewSpeaker = speakerDifference_19c2_;
    CBool newSpeaker = true;

    int numCharsAtStart = 0;
    char speakerBuffer[64] = {0};

    char* speakerPtr;
    char* textPtr = currentPageMessage_50_;
    char* speakerStart = strstr(textPtr, data_020f08dc);
    char* noSpeakerStart = strstr(textPtr, data_020f0979);
    char* altNoSpeakerStart = strstr(textPtr, data_020f097c);

    unsigned char spaceSize = GetFontSpaceSize(fontIndex_19dc_);
    if (speakerStart != NULL)
    {
        while (true)
        {
            if (textPtr == speakerStart)
                break;
            unsigned short control;
            memcpy(&control, textPtr, 2);
            numCharsAtStart++;
            if ((control & 0xff00) == 0xff00)
                textPtr++;
            textPtr++;
        }

        char* speaker = speakerStart + 2;
        char* speakerEnd = strstr(speaker, data_020f08dc);
        if (speakerEnd != NULL)
        {
            speakerLength = speakerEnd - speaker;
            memcpy(speakerBuffer, speaker, speakerLength);
            newSpeaker = false;
            speakerDifference_19c2_ = 0;
            speakerPtr = speakerBuffer;
            while (true)
            {
                if (*speakerPtr == 0)
                    break;
                FontIndexFile::Glyph* glyph = GetNextFontGlyph(speakerPtr, fontIndex_19dc_);
                if (glyph != NULL)
                {
                    unknown_1942_++;
                    int thisWidth = glyph->width;
                    int thisLength = glyph->tagLength;
                    speakerNameWidth_1944_ += thisWidth + 1;
                    speakerPtr += thisLength;
                }
                else
                {
                    unknown_1942_++;
                    speakerPtr++;
                    speakerNameWidth_1944_ += spaceSize + 1;
                }
            }
            unknown_1946_ = speakerEnd - speakerStart;
            unknown_1946_ += 3;
            unknown_1946_ += numCharsAtStart - 2;
            unknown_912_ = 4;
            short boxWidthRounding = 8 - (speakerNameWidth_1944_ & 7);
            speakerTextBox_930_.width = speakerNameWidth_1944_ + 18 + boxWidthRounding;
            speakerTextBox_930_.height = 22;
        }
        else
        {
            speakerDifference_19c2_ = backupNewSpeaker;
        }
    }
    else if (noSpeakerStart != NULL || altNoSpeakerStart != NULL)
    {
        unknown_912_ = 3;
    }
    else
    {
        unknown_1946_ = -1;
    }

    if (!speakerDifference_19c2_ && speakerLength != 0)
    {
        newSpeaker = strcmp(buffer_1902_, speakerBuffer);
    }

    if (newSpeaker)
    {
        for (int i = 0; i < unknown_1942_; i++)
            ReleaseGlyphByIndex(indexBuffer_1882_[i]);
        // bug/oversight: memset writes bytes so will just fill with 0x01
        memset(indexBuffer_1882_, 0xff01, sizeof(indexBuffer_1882_));
        memset(buffer_1902_, 0, sizeof(buffer_1902_));
        memcpy(buffer_1902_, speakerBuffer, speakerLength);
        unknown_19c3_ = 0;
        if (unknown_1942_ != 0)
        {
            mainMessageStartOffset_84_ = unknown_1946_;
            glyphIndexListMainMessageStart_88_ = unknown_1942_ + numCharsAtStart + 3;
        }
    }
    else if (unknown_1942_ != 0)
    {
        mainMessageStartOffset_84_ = unknown_1946_;
        glyphIndexListMainMessageStart_88_ = unknown_1942_ + numCharsAtStart + 3;
        unknown_74_ = unknown_1942_ + numCharsAtStart + 3;
        unknown_78_ = unknown_1942_ + numCharsAtStart + 3;
        unknown_1942_++;
        return;
    }
    unknown_1942_ = 0;
}

void TextManager::ReleaseMainMessageGlyphs()
{
    if (unknown_54_ == NULL || unknown_58_ == NULL)
        return;
    if (glyphIndexListLength_6c_ == 0)
        return;
    unsigned short* pIndex;
    for (unsigned int i = glyphIndexListMainMessageStart_88_; i < glyphIndexListLength_6c_; i++)
    {
        pIndex = &glyphIndexList_4c_[i];
        if (IsNextCharacterControlAlt(pIndex))
            i += GetControlWordNumExtraWords(pIndex);
        else
            ReleaseGlyphByIndex(*pIndex);
    }
    glyphIndexListLength_6c_ = 0;
    unknown_70_ = 0;
    unknown_74_ = 0;
    unknown_78_ = 0;
}

void TextManager::ReleaseGlyphByIndex(unsigned short ch)
{
    if (ch >= 0x80)
        return;
    if (IsNextCharacterControlAlt(&ch))
        return;
    
    RenderGlyph* glyph = &glyphs_[ch];
    if (glyph->unknown_8 < 0)
        return;
    if (glyphRefcounts_17b8_[ch] == 0)
        return;
    int newRefcount = glyphRefcounts_17b8_[ch] - 1;
    glyphRefcounts_17b8_[ch] = newRefcount;
    if ((newRefcount & 0xff) == 0)
    {
        glyph->Reset2();
        numGlyphsInUse_9b4_ = (numGlyphsInUse_9b4_ - 1) & 0x7f;
    }
}

void TextManager::LoadNextPage()
{
    ReleaseMainMessageGlyphs();
    memset(glyphIndexList_4c_, 0, 0x280);
    memset(currentPageMessage_50_, 0, 0x280);

    int skip = 1;
    unsigned char controlTest = unknown_58_[1];
    if (controlTest == 0xff)
    {
        skip = 2;
    }

    unknown_54_ = unknown_58_ + skip;
    unknown_58_ = unknown_54_;

    CopyPage();
    CalculateNewlines(true, currentPageMessage_50_, pageScratchBuffer_60_, 227, 0, true, 1);
    int len = func_020d2ff0(pageScratchBuffer_60_);
    memcpy(currentPageMessage_50_, pageScratchBuffer_60_, len);
}

void TextManager::SubmitQuestionResponse(unsigned short response, char* searchStart)
{
    char* newPos = FindControlCharacter(response, searchStart);
    if (newPos == NULL)
    {
        func_02043204(this);
        func_02043124(this);
        return;
    }

    ReleaseMainMessageGlyphs();
    memset(glyphIndexList_4c_, 0, 0x280);
    memset(currentPageMessage_50_, 0, 0x280);
    unknown_54_ = unknown_58_ = newPos;
    pauseType_9a0_ = 5;
    CopyPage();
    CalculateNewlines(true, currentPageMessage_50_, pageScratchBuffer_60_, 227, 0, true, 1);
    int len = func_020d2ff0(pageScratchBuffer_60_);
    memcpy(currentPageMessage_50_, pageScratchBuffer_60_, len);
}

int TextManager::MeasureLine(const char* text) const
{
    if (text == NULL)
        return 0;
    unsigned char defaultSpacing = GetFontSpaceSize(fontIndex_19dc_);
    int currentGlyphIndex = 0xff;
    
    unsigned char lastGlyphIndex = currentGlyphIndex;
    int tagLength;
    
    int totalWidth = 0;
    while (!IsNextCharacterControlOrJump(text))
    {
        unsigned short control;
        memcpy(&control, text, 2);
        if (IsNextCharacterControlAlt(&control))
        {
            int numExtraWords = GetControlWordNumExtraWords(&control);
            short extraWord;
            switch (control)
            {
            case ControlWord_Unknown_ff19:
                lastGlyphIndex = 0xff;
                totalWidth += GetFontGlyphKerning(lastGlyphIndex, currentGlyphIndex, fontIndex_19dc_);
                break;
            case ControlWord_Unknown_ff20:
                lastGlyphIndex = 0xff;
                memcpy(&extraWord, text + 2, 2);
                if (totalWidth < extraWord)
                    totalWidth = extraWord;
                break;
            case ControlWord_Unknown_ff23:
                lastGlyphIndex = 0xff;
                memcpy(&extraWord, text + 2, 2);
                totalWidth += extraWord;
                break;
            }
            text = text + 2 + (2 * numExtraWords);
        }
        else
        {
            tagLength = 1;
            int glyphWidth = defaultSpacing;
            FontIndexFile::Glyph* glyph = GetNextFontGlyph(text, fontIndex_19dc_);
            if (glyph != NULL)
            {
                currentGlyphIndex = (unsigned char)GetNextFontGlyphIndex(text, fontIndex_19dc_);
                totalWidth += GetFontGlyphKerning(lastGlyphIndex, currentGlyphIndex, fontIndex_19dc_);
                lastGlyphIndex = currentGlyphIndex;
                currentGlyphIndex = 0xff;
                tagLength = glyph->tagLength;
                glyphWidth = glyph->width;
            }
            totalWidth += glyphWidth + 1;
            text += tagLength;
        }
    }
    return totalWidth;
}

void TextManager::GetLineCenteringOffsets(int* outXs, int* outYs) const
{
    int middle = centerX_990_;
    int outIndex;
    const char* textPtr = currentPageMessage_50_ + mainMessageStartOffset_84_;
    
    outXs[0] = middle - (MeasureLine(textPtr) >> 1);
    outYs[0] = 0;

    outIndex = 1;
    while (true)
    {
        char nextChar = *textPtr;
        if (nextChar == 0)
            return;

        if (IsNextCharacterControl(textPtr))
        {
            if (IsNextCharacterInteractionControl(textPtr))
                return;
            if (IsNextCharacterControlOrJump(textPtr))
            {
                textPtr += 2;
                outXs[outIndex] = middle - (MeasureLine(textPtr) >> 1);
                outYs[outIndex] = 0;
                outIndex++;
            }
            else
                textPtr = textPtr + 2 + GetControlWordNumExtraWords(textPtr) * 2;
        }
        else
        {
            int tagLength = 1;
            FontIndexFile::Glyph* glyph = GetNextFontGlyph(textPtr, fontIndex_19dc_);
            if (glyph != NULL)
                tagLength = glyph->tagLength;
            textPtr += tagLength;
        }
    }
}

void TextManager::AdvanceLine()
{
    bool moreChars = true;
    int wordSkip = 0;
    unsigned short* wordPtr = glyphIndexList_4c_ + unknown_74_;
    while (moreChars)
    {
        if (IsNextCharacterControlOrJump(wordPtr))
            moreChars = false;
        if (IsNextCharacterControlAlt(wordPtr))
        {
            int numExtraWords = GetControlWordNumExtraWords(wordPtr);
            wordPtr += numExtraWords;
            wordSkip += numExtraWords;
        }
        else
            ReleaseGlyphByIndex(*wordPtr);
        wordPtr++;
        wordSkip++;
    }

    unknown_74_ += wordSkip;
    unknown_78_++;
}

void TextManager::PositionOptionDialogue()
{
    short mainX = mainTextBox_914_.x;
    short mainW = mainTextBox_914_.width;
    short optionW = dialogueOptions_90_.textBox_b4_.width;
    short optionH = dialogueOptions_90_.textBox_b4_.height;
    short mainY = mainTextBox_914_.y;

    short x = mainX + mainW - optionW;
    if (x < 0)
        x = 0;
       
    short y = mainY - optionH;
    if (y < 0)
        y = 0;
    dialogueOptions_90_.textBox_b4_.x = x;
    dialogueOptions_90_.textBox_b4_.y = y;
}

void TextManager::GetBoxShakeOffset(int *outX, int *outY)
{
    *outX = *outY = 0;
    if (!maybeBoxShaking_19c0_)
        return;
    unknown_19cd_ = 1;
    unknown_40_ = 0;
    int index = unknown_195d_ & 3;
    *outX = data_020e7e04.shakeXValues[index];
    *outY = data_020e7e04.shakeYValues[index];
}

struct NewlineCalculationTagProcessor
{
    char* tag;
    int (*proc)(char* input, int* pX, int fontIndex, int leftMargin);
};

extern const NewlineCalculationTagProcessor data_020e7ec4[];

int NewlineCalculation_X(char* input, int* pX, int fontIndex, int leftMargin)
{
    int value;
    ReadNumbersInTag(input, &value, 1);
    *pX = value - (GetFontSpaceSize(fontIndex) + 1);
    return 0;
}

int NewlineCalculation_XY(char* input, int* pX, int fontIndex, int leftMargin)
{
    int value;
    ReadNumbersInTag(input, &value, 1);
    *pX = value - (GetFontSpaceSize(fontIndex) + 1);
    return 0;
}

int NewlineCalculation_W(char* input, int* pX, int fontIndex, int leftMargin)
{
    int value;
    ReadNumbersInTag(input, &value, 1);
    *pX += value;
    return 0;
}

int NewlineCalculation_WH(char* input, int* pX, int fontIndex, int leftMargin)
{
    int value;
    ReadNumbersInTag(input, &value, 1);
    *pX += value;
    return 0;
}

int NewlineCalculation_CloseTitle(char* input, int* pX, int fontIndex, int leftMargin)
{
    *pX = 0;
    return 0;
}

int NewlineCalculation_OpenXR(char* input, int* pX, int fontIndex, int leftMargin)
{
    int value;
    ReadNumbersInTag(input, &value, 1);
    *pX = value;
    return 1;
}

int NewlineCalculation_CloseXR(char* input, int* pX, int fontIndex, int leftMargin)
{
    return 2;
}

void TextManager::CalculateNewlines(bool arg2, char *input, char *output,
    int boxWidth, int leftMargin, bool useNewlineControl, unsigned char fontType)
{
    int currentX;
    char* inputLastWordStart = NULL;
    char* outputLastWordStart = NULL;
    int lastWordStartX = 0;
    int extraWidthFromSpeaker = 0;
    
    char* noSpeakerStart = strstr(input, data_020f0979);
    char* altNoSpeakerStart = strstr(input, data_020f097c);

    if (noSpeakerStart != NULL || altNoSpeakerStart != NULL)
        extraWidthFromSpeaker = 2;
    
    currentX = leftMargin;
    unsigned char lastGlyphIndex = 0xff;
    bool leftAligned = true;
    bool shouldWriteHyphen = false;
    
    unsigned char defaultSpacing = GetFontSpaceSize(fontType);

    while (true)
    {
        char nextChar = *input;
        if (nextChar == 0)
            break;
        unsigned short control;
        memcpy(&control, input, 2);
        if (IsNextCharacterControlAlt(&control) && control != ControlWord_Unknown_ff19)
        {
            int numControlWordBytes = (GetControlWordNumExtraWords(&control) + 1) * 2;
            memcpy(output, input, numControlWordBytes);
            input += numControlWordBytes;
            output += numControlWordBytes;
            if (control == ControlWord_LineBreak)
            {
                extraWidthFromSpeaker = 0;
                currentX = leftMargin;
                inputLastWordStart = input;
                outputLastWordStart = output;
                lastWordStartX = currentX;
                lastGlyphIndex = 0xff;
                shouldWriteHyphen = false;
            }
            continue;
        }

        if (nextChar == '\\' && input[1] == 'n')
        {
            extraWidthFromSpeaker = 0;
            currentX = leftMargin;
            lastGlyphIndex = 0xff;
            
            memcpy(output, input, 2);
            input += 2;
            output += 2;
            inputLastWordStart = input;
            outputLastWordStart = output;
            lastWordStartX = currentX;
            shouldWriteHyphen = false;
            continue;
        }

        if (nextChar == '\n')
        {
            extraWidthFromSpeaker = 0;
            currentX = leftMargin;
            lastGlyphIndex = 0xff;
            memcpy(output, input, 1);
            input += 1;
            output += 1;
            inputLastWordStart = input;
            outputLastWordStart = output;
            lastWordStartX = currentX;
            shouldWriteHyphen = false;
            continue;
        }

        if (memcmp(input, data_020f0981, 6) == 0)
        {
            extraWidthFromSpeaker = 0;
            currentX = leftMargin;
            lastGlyphIndex = 0xff;
            memcpy(output, input, 6);
            input += 6;
            output += 6;
            inputLastWordStart = input;
            outputLastWordStart = output;
            lastWordStartX = currentX;
            shouldWriteHyphen = false;
            continue;
        }

        if (memcmp(input, data_020f08dc, 2) == 0)
        {
            char* speakerEnd = strstr(input, data_020f0988);
            if (speakerEnd != NULL)
            {
                currentX = NULL;
                int speakerLength = speakerEnd - input;
                memcpy(output, input, speakerLength + 3);
                input += speakerLength + 3;
                output += speakerLength + 3;
                continue;
            }
        }

        int nextGlyphIndex = GetNextFontGlyphIndex(input, fontType);
        if (control == ControlWord_Unknown_ff19)
            nextGlyphIndex = -1;
        if (nextChar == '<' && nextGlyphIndex < 0)
        {
            char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL)
            {
                char* tagInner = input + 1;
                for (const NewlineCalculationTagProcessor* candidate = data_020e7ec4; 
                    candidate->tag != NULL && candidate->proc != NULL; candidate++)
                {
                    if (!CaseInsensitiveDoesStringBeginWith(tagInner, candidate->tag))
                        continue;

                    int procRet = candidate->proc(tagInner + func_020d2ff0(candidate->tag), &currentX, fontType, leftMargin);
                    if (procRet != 1)
                    {
                        if (procRet == 2)
                        {
                            leftAligned = true;
                        }
                        break;
                    }
                    else
                    {
                        leftAligned = false;
                        break;
                    }
                }
                int tagInnerLength = (tagEnd - input);
                memcpy(output, input, tagInnerLength + 1);
                input += tagInnerLength + 1;
                output += tagInnerLength + 1;
                continue;
            }
        }

        bool foundHyphen = false;
        int glyphSpacing = defaultSpacing + 1;
        int ioCopyLength = 1;
        if (control == ControlWord_Unknown_ff19)
        {
            shouldWriteHyphen = true;
            ioCopyLength = 2;
        }
        if (nextChar == ' ')
        {
            shouldWriteHyphen = false;
        }
        if (memcmp(input, data_020f098c, 3) == 0)
        {
            nextGlyphIndex = -1;
            input += 3;
            glyphSpacing = 0;
            ioCopyLength = 0;
            shouldWriteHyphen = true;
        }
        if (nextGlyphIndex >= 0)
        {
            FontIndexFile::Glyph* glyph = GetFontGlyphByIndex(nextGlyphIndex, fontType);
            ioCopyLength = glyph->tagLength;
            unsigned char nextGlyphUC = nextGlyphIndex;
            int kerning = GetFontGlyphKerning(lastGlyphIndex, nextGlyphUC, fontType);
            lastGlyphIndex = nextGlyphUC;
            glyphSpacing = kerning + glyph->width;
            
            foundHyphen = (glyph->tag[0] == '-');
            if (foundHyphen)
                shouldWriteHyphen = false;

            if (inputLastWordStart != NULL && outputLastWordStart != NULL && leftAligned && 
                (boxWidth + extraWidthFromSpeaker < currentX + glyphSpacing))
            {
                currentX -= lastWordStartX;
                currentX += leftMargin;
                extraWidthFromSpeaker = 0;
                int copyLength = input - inputLastWordStart;
                int clearLength = output - outputLastWordStart;
                output = outputLastWordStart;
                memset(outputLastWordStart, 0, clearLength);
                if (shouldWriteHyphen)
                {
                    memcpy(output, data_020f0990, 1);
                    output += 1;
                }
                if (useNewlineControl)
                {
                    memcpy(output, &data_020e7e04.newlineWord, 2);
                    output += 2;
                }
                else
                {
                    memcpy(output, data_020f0992, 1);
                    output += 1;
                }
                memcpy(output, inputLastWordStart, copyLength);
                output += copyLength;
                shouldWriteHyphen = false;
                if (arg2)
                {
                    unknown_70_++;
                }
            }
            glyphSpacing++;
        }
        if (leftAligned)
            currentX += glyphSpacing;

        memcpy(output, input, ioCopyLength);
        input += ioCopyLength;
        output += ioCopyLength;
        if (nextGlyphIndex < 0 || foundHyphen)
        {
            inputLastWordStart = input;
            outputLastWordStart = output;
            lastWordStartX = currentX;
        }
    }

    *output = *input;
}