#pragma once

#ifndef IR_CUSTOM_TYPES
#define IR_CUSTOM_TYPES

/**
 * @NOTE(Michael): 1 KB = 1024 B, 1 MB = 1024 KB ...
 */
#define IR_KILOBYTES(AMOUNT)    ((AMOUNT) * 1024LL)
#define IR_MEGABYTES(AMOUNT)    (IR_KILOBYTES((AMOUNT)) * 1024LL)
#define IR_GIGABYTES(AMOUNT)    (IR_MEGABYTES((AMOUNT)) * 1024LL)
#define IR_TERABYTES(AMOUNT)    (IR_GIGABYTES((AMOUNT)) * 1024LL )

typedef char                u8;
typedef unsigned short      u16;
typedef unsigned int        u32;
typedef unsigned long long  u64;

typedef signed char         s8;
typedef signed short        s16;
typedef signed int          s32;
typedef signed long long    s64;

typedef float               f32;
typedef double              f64;
typedef long double         f128;

typedef bool                b8;

typedef u8				c8;

//Memory sized integer aka. size_t
#ifdef IR_32bit
typedef u32              msi;
#else
typedef u64              msi;
#endif


struct Buffer
{
    msi length;
    u8* data;
};


typedef Buffer String;

// Use this to write string literals. usage: string str = CONSTZ("myliteral");
#define IR_CONSTZ(STRINGLITERAL) {sizeof((STRINGLITERAL))-1, (u8*)(STRINGLITERAL)}

#ifndef IR_WRAP_INTO_BUFFER
#define IR_WRAP_INTO_BUFFER(ptr, length) (Buffer{(msi) length, (u8*)ptr})
#endif


#define IR_CONSTZ(STRINGLITERAL) {sizeof((STRINGLITERAL))-1, (u8*)(STRINGLITERAL)}

#endif
