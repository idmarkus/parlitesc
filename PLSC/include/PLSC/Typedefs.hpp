#pragma once
#include "PLSC/DBG/Profile.hpp"

#include <cstdint>

typedef bool   b;
typedef float  f32;
typedef double f64;

typedef int                i32;
typedef long long          i64;
typedef unsigned int       u32;
typedef unsigned long long u64;

typedef int8_t  i8_t;
typedef int16_t i16_t;
typedef int32_t i32_t;
typedef int64_t i64_t;

typedef uint8_t  u8_t;
typedef uint16_t u16_t;
typedef uint32_t u32_t;
typedef uint64_t u64_t;

namespace PLSC
{
    // Arithmetic type used for user settings
    using Number = long double;
} // namespace PLSC