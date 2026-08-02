#pragma once

#include "std_library_functions.h"

// Fixed-point number with 12 bits after the point
typedef int32_t fix32_t;
// Fixed-point number with 12 bits after the point
typedef int16_t fix16_t;

#define FIX32_MULTIPLY(a, b) (fix32_t)((((int64_t)(a) * (int64_t)(b)) + (int64_t)0x800) >> (int64_t)12)

#define FIX32_MULTIPLY_v2(a, b) ((((int64_t)(unsigned int)(b) * (int64_t)(a))) >> (int64_t)12) // no cast to fix32_t
#define FIX32_MULTIPLY_v3(a, b) ((((int64_t)(b) * (int64_t)(a))) >> (int64_t)12)

#define FIX32_MULTIPLY_v4(a, b) ((((int64_t)(unsigned)(b) * (int64_t)(signed)(a))) >> (int64_t)12) // no cast to fix32_t

struct Vector3i
{
    int x;
    int y;
    int z;
};

typedef Vector3i Vector3fix;