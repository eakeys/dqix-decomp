#pragma once

#include "../GameState/PartyMember.h"
#include "../GameState/CombatEnemy.h"
#include "std_library_functions.h"

enum GrammarCase
{
    GrammarCase_Nominative = 0,
    GrammarCase_Genitive = 1,
    GrammarCase_Dative = 2,
    GrammarCase_Accusative = 3
};

// sizeof == 0xc == 12.
// Represents some in-game thing or group of things that is referred
// to in words, such as players, monsters or items
class Noun
{
public:
    char* singularName_;
    char* pluralName_;
    // add 100 to this to get the index of the singular indefinite article, e.g.
    // to distinguish *a jar of* nectar vs *a lump of* densinium
    unsigned int indefiniteSingularOffset_ : 6;
    unsigned int indefinitePluralOffset_ : 6;
    unsigned int definiteSingularOffset_ : 6;
    unsigned int definitePluralOffset_ : 6;
    unsigned int gender_ : 2; // 0 = m, 1 = f, 2 = n
    unsigned int bit_26 : 1;
    unsigned int bit_27 : 1;
    unsigned int startsWithVowel_ : 1;
    unsigned int bit_29 : 1;
    // if set, you can't be certain about startsWithVowel being correct
    // for french, need to look at the glyphs and do h/y processing?
    unsigned int specialVowelBehavior_ : 1;
    unsigned int bit_31 : 1;

    void Reset();

    void Populate(char* singularName, char* pluralName, unsigned char indefSingOffset,
        unsigned char indefPluralOffset, unsigned char defSingOffset, unsigned char defPluralOffset,
        unsigned char gender, unsigned char arg8, unsigned char arg9, unsigned char arg10, unsigned char arg11);

    void PopulateFromPartyMember(int index);
    void PopulateFromPartyMember(PartyMember* member);
    void PopulateFromEnemy(CombatEnemy* enemy, int unknown);

    // func_020e4e38: PopulateFromAction() or PopulateFromAbility()
    // fires when using a skill in or out of combat

    // flag bit 0: copy names (bytes are copied but not allocated)
    // flag bit 1: copy article info
    // gender & 1-bit data (except bits 29, 30) are always copied
    void CopyTo(Noun* output, int flags);
};

char* AddArticleAndDeclineNoun(char** ppOutput, Noun* noun, int indefinite, int hasArticle, int plural, int grammarCase);