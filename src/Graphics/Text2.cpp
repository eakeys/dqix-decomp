#include "Graphics/Text.h"
#include "std_library_functions.h"
#include "Util/StringTests.h"
#include "Combat/Main/BattleList.h"
#include <globaldefs.h>

extern "C"
{
    // probably atoi
    int func_02005a94(const char*);

    // advance to end of tag
    const char* func_020424ac(const char*);
    int func_02042658(int, int);

    // make sequence of chars uppercase
    void func_02067f5c(char*, int);

    // alternative strlen implementation
    int func_020d2ff0(const char*);
}

template<class FuncPtr>
struct TextProcessingCallback
{
    const char* tag;
    FuncPtr callback;
};

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
        int W = unknownThing * 8 - func_02042658(0, value);
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
            const char* tagEnd = func_020424ac(input);
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

void TextManager::SubstituteUnsupportedCharacters(const char* input, char* output, int fontType)
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

        FontIndexFile::CharacterEntry* charEntry = GetNextFontCharacterEntry(input, fontType);
        if (charEntry != NULL)
        {
            int tagLength = charEntry->tagLength;
            memcpy(output, input, tagLength);
            input += tagLength;
            output += tagLength;
            continue;
        }

        if (nextInputChar == '<')
        {
            const char* tagEnd = func_020424ac(input);
            if (tagEnd != NULL)
            {
                memcpy(output, input, tagEnd - input + 1);
                func_02067f5c(output, tagEnd - input + 1);
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

int TextConditional_Value1Single(BattleStruct*, TextManager* mgr, int) { return (mgr->valueLookup[0] == 1) ? 0 : 1; }
int TextConditional_Value2Single(BattleStruct*, TextManager* mgr, int) { return (mgr->valueLookup[1] == 1) ? 0 : 1; }
int TextConditional_Value3Single(BattleStruct*, TextManager* mgr, int) { return (mgr->valueLookup[2] == 1) ? 0 : 1; }
int TextConditional_Value4Single(BattleStruct*, TextManager* mgr, int) { return (mgr->valueLookup[3] == 1) ? 0 : 1; }
int TextConditional_Value5Single(BattleStruct*, TextManager* mgr, int) { return (mgr->valueLookup[4] == 1) ? 0 : 1; }

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
    int (*callback)(BattleStruct*, TextManager*, int);
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
    BattleStruct* battle = GetBattleStruct();
    while (true)
    {
        char nextChar = *input;
        if (nextChar == '\0')
            break;
        
        if (nextChar == '<' && CaseInsensitiveDoesStringBeginWith(input, data_020f08ff) && func_020424ac(input) != NULL)
        {
            char* inputNoSkip = input;
            for (const ConditionProcessor* condition = data_020e833c; condition->callback != NULL; condition++)
            {
                char ifTag[48] = {0};
                BuildConditionTag(condition->tagWordSets[0], ifTag);
                if (CaseInsensitiveDoesStringBeginWith(input, ifTag))
                {
                    int evaluation = condition->callback(battle, this, fontType);
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