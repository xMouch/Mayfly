#ifndef MAYFLY_MAIN_H
#define MAYFLY_MAIN_H

#include "stdio.h"
#include "stdlib.h"

#include "ir_types.h"
#include "ir_memory.h"
#include "ir_string.h"
#include "ir_ds.h"

enum Token_Type
        {
    TOKEN_UNKOWN = 0,
    TOKEN_ID,
    TOKEN_NUM,
    TOKEN_STR_LIT,

    TOKEN_FN        = 1000,
    TOKEN_RETURN,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_BREAK,
    TOKEN_CONT,
    TOKEN_STRUCT,

    TOKEN_S64       = 1050,
    TOKEN_F64,
    TOKEN_C8,

    TOKEN_D_EQ      = 1100, // ==
    TOKEN_AND, // &&
    TOKEN_OR, // ||
    TOKEN_NOTEQ, // !=

    /*
       All single charactars are the same value as their ASCII value so A == 'A' == 65

    */

        };

struct Token
        {
    String text;
    Token_Type type;
    msi line;
    msi column;
        };

#endif //MAYFLY_MAIN_H
