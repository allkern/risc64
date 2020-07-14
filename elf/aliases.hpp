#pragma once

#include <cstdint>

namespace elf {
    typedef uint_least8_t  u8 ;
    typedef uint_least16_t u16;
    typedef uint_least32_t u32;
    typedef uint_least64_t u64;
    typedef  int_least8_t  s8 ;
    typedef  int_least16_t s16;
    typedef  int_least32_t s32;
    typedef  int_least64_t s64;

    enum target { bits32, bits64 };
}