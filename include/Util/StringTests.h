#pragma once

// these might be sdk functions?

// usa: func_020d8550
char MakeCharUpperCase(int c);

// usa: func_020d857c
// Tests if the second string appears as an initial segment of the first
bool DoesStringBeginWith(const char* str, const char* initial);

// usa: func_020d85dc
// Tests if the second string appears as an initial segment of the first
bool CaseInsensitiveDoesStringBeginWith(const char* str, const char* initial);