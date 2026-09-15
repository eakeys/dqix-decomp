#include "Graphics/Text.h"
#include "std_library_functions.h"
#include "Util/StringTests.h"
#include "GameState/GameState.h"
#include <globaldefs.h>

extern "C"
{
    // probably atoi
    int func_02005a94(const char*);
    // probably atof
    double func_020055d4(const char*);

    void* func_0203dce4(void*, int);
    void* func_0203cf4c();
    Vector3fix func_020406f8(void*);

    // set textbox dimensions?
    void func_02042b98(TextManager*, int, int, int, int);
    bool func_02044494(const TextManager*, const void*);
    void func_020457e8(TextManager*, int);

    // play sound effect?
    void func_0205eaa0(void*, int effect, int);

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
    char unk_0[0x24];
    short array_24[2];
} extern const data_020e7e04;

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

void TextManager::SubstituteValueTags(char *input, char *argOutput)
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
            char* tagEnd = GetTextTagEnd(input);
            if (tagEnd != NULL)
            {
                bool processedTag = false;
                const TextProcessingCallback<PFNProcessVALTag>* candidate;
                char* tagInterior = input + 1;
                
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

int TextConditional_Value1Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[0] == 1) ? 0 : 1; }
int TextConditional_Value2Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[1] == 1) ? 0 : 1; }
int TextConditional_Value3Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[2] == 1) ? 0 : 1; }
int TextConditional_Value4Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[3] == 1) ? 0 : 1; }
int TextConditional_Value5Single(GameState*, TextManager* mgr, int) { return (mgr->valueLookup[4] == 1) ? 0 : 1; }

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
    GameObject* leader = GameState::GetInstance()->GetPartyLeader();
    void* maybeNPC = func_0203dce4(func_0203cf4c(), npcID_1838_);
    if (leader != NULL && maybeNPC != NULL)
    {
        Vector3fix playerPos = leader->obj3D_.position_;
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
            winTag_19b1_ = 0;
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

int TextManager::GetControlWordNumExtraWords(const char *input) const
{
    int numExtra = 0;
    unsigned short controlWord;
    memcpy(&controlWord, input, 2);
    if (!func_02044494(this, &controlWord))
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
    if (!func_02044494(this, &control))
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