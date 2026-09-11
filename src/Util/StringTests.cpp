#include "Util/StringTests.h"
#include <globaldefs.h>

char MakeCharLowerCase(int c)
{
    unsigned char ch = c;
    bool isUpper = false;
    if (ch >= 'A' && ch <= 'Z')
        isUpper = true;
    if (isUpper)
        c += 0x20;
    return c;
}

bool DoesStringBeginWith(const char *str, const char *initial)
{
    if (str == NULL)
        return false;
    if (initial == NULL)
        return false;
    if (str == initial)
        return true;

    while (*str != 0 && *initial != 0 && *str == *initial)
    {
        str++;
        initial++;
    }
    if (*initial == 0)
        return true;

    return false;
}

bool CaseInsensitiveDoesStringBeginWith(const char* str, const char* initial)
{
    if (str == NULL)
        return false;
    if (initial == NULL)
        return false;
    if (str == initial)
        return true;

    while (*str != 0 && *initial != 0 && MakeCharLowerCase(*str) == MakeCharLowerCase(*initial))
    {
        str++;
        initial++;
    }
    if (*initial == 0)
        return true;
    return false;
}