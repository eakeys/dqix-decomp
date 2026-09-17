#include "Util/Grammar.h"
#include "GameState/GameState.h"

#include <globaldefs.h>
#include "GameState/GameState.h"
#include "Filesystem/BackgroundLoader.h"
#include "Memory/SafeAllocator.h"
#include "World/Object3D.h"
#include "Resource/GameResources.h"
#include "Combat/Main/BattleList.h"
#include "Util/Random.h"

extern "C"
{
    int func_0200fb08(GameState*);
    int func_020d2ff0(const char*);

    // get grammar unique string e.g. article
    const char* func_020e51e4(short);
}

typedef char DeclensionRuleName[11];
typedef char DeclensionRule[8]; // index into this by [2 * case + indefinite]
typedef const char Article[6];
typedef const char DeclensionAppendage[3]; 

extern DeclensionAppendage data_020ee9d8[]; // "", "'", "en", "es", "s", "n", "r"
extern char* (*data_020ee9f0[])(char*, Noun*);
extern Article data_020eea08[];

extern struct ArticleRule
{
    char articleIndexByCase[4];
    const char* tag;
    char tagLength;
} data_020eea50[];

extern DeclensionRule data_020eeab0[];
extern DeclensionRuleName data_020f2d2c[];
extern char data_020f2dc8[];
extern char data_020f2dcb[];
extern char data_020f2dcf[]; // " "

char* CombineArticleAndNoun(char* output, const char* article, const char* name, Noun* unused_noun)
{
    if (article != NULL && *article != 0)
    {
        output += sprintf(output, data_020f2dc8, article);
        int needsSpace = 1;
        if (article != NULL)
        {
            int length = func_020d2ff0(article);
            if (length <= 3)
                goto definitely;
            else
            {
                needsSpace = memcmp(article + (length - 3), data_020f2dcb, 3);
                goto maybe;
            }
        }
        definitely:
            needsSpace = 1;
        maybe:

        if (needsSpace)
            output += sprintf(output, data_020f2dcf);
    }

    output += sprintf(output, name);
    return output;
}

void Noun::Reset()
{
    memset(this, 0, sizeof(Noun));
}

char* CreateNounString_SingularNoArticle(char* output, Noun* noun)
{
    if (noun->singularName_ != NULL)
        output += sprintf(output, noun->singularName_);
    return output;
}

char* CreateNounString_SingularIndefinite(char* output, Noun* noun)
{
    if (noun->singularName_ != NULL)
    {
        const char* article = func_020e51e4(noun->indefiniteSingularOffset_ + 100);
        output = CombineArticleAndNoun(output, article, noun->singularName_, NULL);
    }
    return output;
}

char* CreateNounString_SingularDefinite(char* output, Noun* noun)
{
    if (noun->singularName_ != NULL)
    {
        const char* article = func_020e51e4(noun->definiteSingularOffset_);
        output = CombineArticleAndNoun(output, article, noun->singularName_, NULL);
    }
    return output;
}

char* CreateNounString_PluralNoArticle(char* output, Noun* noun)
{
    if (noun->pluralName_ != NULL)
        output += sprintf(output, noun->pluralName_);
    return output;
}

char* CreateNounString_PluralIndefinite(char* output, Noun* noun)
{
    if (noun->pluralName_ != NULL)
    {
        const char* article = func_020e51e4(noun->indefinitePluralOffset_ + 300);
        output = CombineArticleAndNoun(output, article, noun->pluralName_, noun);
    }
    return output;
}

char* CreateNounString_PluralDefinite(char* output, Noun* noun)
{
    if (noun->pluralName_ != NULL)
    {
        const char* article = func_020e51e4(noun->definitePluralOffset_ + 200);
        output = CombineArticleAndNoun(output, article, noun->pluralName_, noun);
    }
    return output;
}

void ProcessGermanDeclension(const char* input, char* output, int indefinite, int article, int plural, int grammarCase)
{
    while (true)
    {
        if (*input == 0)
            break;

        if (*input == '[')
        {
            bool matchedRule = false;
            int i = 0;
            while (true)
            {
                if (data_020f2d2c[i][0] == 0)
                    break;
                int ruleLength = func_020d2ff0(data_020f2d2c[i]);
                if (memcmp(input + 1, data_020f2d2c[i], ruleLength) == 0)
                {
                    if (indefinite >= 0 && grammarCase >= 0)
                    {
                        int appendageIndex = data_020eeab0[i][2 * grammarCase + indefinite];
                        int appendageLength = func_020d2ff0(data_020ee9d8[appendageIndex]);
                        memcpy(output, data_020ee9d8[appendageIndex], appendageLength);
                        output += appendageLength;
                    }
                    matchedRule = true;
                    input += ruleLength + 1;
                    break;
                }

                i++;
            }

            if (matchedRule)
                continue;
        }
        
        *output = *input;
        output++;
        input++;
    }
    *output = *input;
}

char* AddArticleAndDeclineNoun(char** ppOutput, Noun* noun, int indefinite, int hasArticle, int plural, int grammarCase)
{
    char buffer1[256] = {0};
    int definiteIndex = 1 - indefinite;
    unsigned int adjustedPlural = plural;
    if (indefinite == -1)
        definiteIndex = 0;
    if (adjustedPlural == -1)
        adjustedPlural = 0;
    adjustedPlural *= 3;
    // at this point, the array order is
    // singular no article, singular indefinite, singular definite,
    // plural no article, plural indefinite, plural definite 
    data_020ee9f0[definiteIndex + hasArticle + adjustedPlural](buffer1, noun);
    definiteIndex = 1 - definiteIndex;
    char buffer2[256] = {0};

    if (grammarCase == -1 && func_0200fb08(GameState::GetInstance()) == 3) // german
        grammarCase = GrammarCase_Nominative;

    char* ptrA;
    char* ptrB = buffer2;
    ptrA = buffer1;

    while (true)
    {
        if (*ptrA == 0)
            break;

        if (*ptrA == '[')
        {
            bool matchedRule = false;
            for (const ArticleRule* rule = data_020eea50; rule->tagLength != 0; rule++)
            {
                int tagLength = rule->tagLength;
                if (memcmp(ptrA + 1, rule->tag, tagLength) != 0)
                    continue;
                
                if (grammarCase != -1)
                {
                    int articleIndex = rule->articleIndexByCase[grammarCase];
                    int articleLength = sprintf(ptrB, data_020eea08[articleIndex]);
                    ptrB += articleLength;
                    
                    ptrA += rule->tagLength + 1;
                    if (*ptrA != ' ')
                        ptrB += sprintf(ptrB, data_020f2dcf);
                }
                else
                {
                    ptrA += tagLength + 1;
                    if (*ptrA == ' ')
                        ptrA++;
                }
                matchedRule = true;
                break;
            }

            if (matchedRule)
                continue;
        }

        *ptrB = *ptrA;
        ptrA++;
        ptrB++;
    }

    *ptrB = *ptrA;
    memset(buffer1, 0, sizeof(buffer1));
    ProcessGermanDeclension(buffer2, buffer1, definiteIndex, hasArticle, adjustedPlural, grammarCase);
    int finalLength = func_020d2ff0(buffer1);
    memcpy(*ppOutput, buffer1, finalLength);
    *ppOutput += finalLength;
    return *ppOutput;
}

void Noun::Populate(char *singularName, char *pluralName, unsigned char indefSingOffset,
    unsigned char indefPluralOffset, unsigned char defSingOffset, unsigned char defPluralOffset,
    unsigned char gender, unsigned char pluraleTantum, unsigned char isPerson,
    unsigned char startsWithVowel, unsigned char specialVowelBehavior)
{
    memset(this, 0, sizeof(Noun));
    singularName_ = singularName;
    pluralName_ = pluralName;

    indefiniteSingularOffset_ = indefSingOffset;
    indefinitePluralOffset_ = indefPluralOffset;
    definiteSingularOffset_ = defSingOffset;
    definitePluralOffset_ = defPluralOffset;
    gender_ = gender;
    pluraleTantum_ = pluraleTantum;
    isPerson_ = isPerson;
    startsWithVowel = startsWithVowel;
    specialVowelBehavior_ = specialVowelBehavior;
}

void Noun::PopulateFromPartyMember(int index)
{
    Reset();
    PartyMember* member = GameState::GetInstance()->GetPartyMemberByIndex(index);
    if (member == NULL)
        return;
    Populate(member->baseStats_->name, member->baseStats_->name,
        0, 0, 0, 0,
        member->partyMemberStats_->gender_,
        false, // not plurale tantum
        true,  // is a person
        false, // doesn't (automatically!) start with a vowel
        true); // special vowel behavior, need to actually read the name
}

void Noun::PopulateFromPartyMember(PartyMember* member)
{
    Reset();
    if (member == NULL)
        return;
    Populate(member->baseStats_->name, member->baseStats_->name,
        0, 0, 0, 0,
        member->partyMemberStats_->gender_,
        false, // not plurale tantum
        true,  // is a person
        false, // doesn't (automatically!) start with a vowel
        true); // special vowel behavior, need to actually read the name
}

void Noun::PopulateFromEnemy(CombatEnemy *enemy, int unknown)
{
    Reset();
    if (enemy == NULL)
        return;
    EnemyNamingData* namingData = enemy->namingData_;
    char* individual = enemy->individualName_;
    Populate(individual, namingData->pluralName, 
        namingData->indefiniteSingularOffset_, namingData->indefinitePluralOffset_,
        namingData->definiteSingularOffset_, namingData->definitePluralOffset_,
        namingData->gender_, namingData->pluraleTantum_, namingData->isPerson_,
        namingData->startsWithVowel_, false); // no special vowel behavior

    int individualNameLength = func_020d2ff0(individual);
    char monsterIndex;
    if (individualNameLength != 0)
    {
        monsterIndex = individual[individualNameLength - 1];
        if (monsterIndex >= 'A' && monsterIndex <= 'Z')
            goto got_index;
    }
    monsterIndex = 0;
    got_index:
    if (monsterIndex != 0 && unknown)
    {
        int language = func_0200fb08(GameState::GetInstance());
        if (language == 1) // english
        {
            definiteSingularOffset_ = 0; // "slime A" instead of "the slime A"
            indefiniteSingularOffset_ = 0; // "slime A" instead of "a slime A"
        }
        bit_31 = true;
    }
}

void Noun::CopyTo(Noun* output, int flags)
{
    if (this == NULL || output == NULL)
        return;
    if (flags & 1)
    {
        char* destIndividual = output->singularName_;
        char* destPlural = output->pluralName_;
        sprintf(destIndividual, singularName_);
        sprintf(destPlural, pluralName_);
    }

    if (flags & 2)
    {
        output->indefiniteSingularOffset_ = indefiniteSingularOffset_;
        output->indefinitePluralOffset_ = indefinitePluralOffset_;
        output->definiteSingularOffset_ = definiteSingularOffset_;
        output->definitePluralOffset_ = definitePluralOffset_;        
    }

    output->gender_ = gender_;
    output->pluraleTantum_ = pluraleTantum_;
    output->isPerson_ = isPerson_;
    output->startsWithVowel_ = startsWithVowel_;
    output->bit_31 = bit_31;
}