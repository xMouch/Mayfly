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
    TOKEN_EOF,
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
    TOKEN_LEQ, // <= TODO
    TOKEN_GEQ, // >=
    
    /*
       All single charactars are the same value as their ASCII value so A == 'A' == 65

    */
    
};


//TODO(Michael) ADD whole line to token for better error msg
struct Token
{
    String text;
    Token_Type type;
    msi line;
    msi column;
};

static
String type_to_str(Token_Type t)
{
    switch(t)
    {
        case TOKEN_UNKOWN: return IR_CONSTZ("TOKEN_UNKOWN");
        case TOKEN_EOF:    return IR_CONSTZ("TOKEN_EOF");
        case TOKEN_ID:     return IR_CONSTZ("TOKEN_ID");
        case TOKEN_NUM:    return IR_CONSTZ("TOKEN_NUM");
        case TOKEN_STR_LIT:return IR_CONSTZ("TOKEN_STR_LIT");
        case TOKEN_FN:     return IR_CONSTZ("TOKEN_FN");
        case TOKEN_RETURN: return IR_CONSTZ("TOKEN_RETURN");
        case TOKEN_IF:     return IR_CONSTZ("TOKEN_IF");
        case TOKEN_ELSE:   return IR_CONSTZ("TOKEN_ELSE");
        case TOKEN_FOR:    return IR_CONSTZ("TOKEN_FOR");
        case TOKEN_WHILE:  return IR_CONSTZ("TOKEN_WHILE");
        case TOKEN_BREAK:  return IR_CONSTZ("TOKEN_BREAK");
        case TOKEN_CONT:   return IR_CONSTZ("TOKEN_CONT");
        case TOKEN_STRUCT: return IR_CONSTZ("TOKEN_STRUCT");
        case TOKEN_S64:    return IR_CONSTZ("TOKEN_S64");
        case TOKEN_F64:    return IR_CONSTZ("TOKEN_F64");
        case TOKEN_C8:     return IR_CONSTZ("TOKEN_C8");
        case TOKEN_D_EQ:   return IR_CONSTZ("TOKEN_D_EQ");
        case TOKEN_AND:    return IR_CONSTZ("TOKEN_AND");
        case TOKEN_OR:     return IR_CONSTZ("TOKEN_OR");
        case TOKEN_NOTEQ:  return IR_CONSTZ("TOKEN_NOTEQ");
        case '(': return IR_CONSTZ("'('");
        case ')': return IR_CONSTZ("')'");
        case '[': return IR_CONSTZ("'['");
        case ']': return IR_CONSTZ("']'");
        case '{': return IR_CONSTZ("'{'");
        case '}': return IR_CONSTZ("'}'");
        case '<': return IR_CONSTZ("'<'");
        case '>': return IR_CONSTZ("'>'");
        case '%': return IR_CONSTZ("'%'");
        case '+': return IR_CONSTZ("'+'");
        case '-': return IR_CONSTZ("'-'");
        case '*': return IR_CONSTZ("'*'");
        case '/': return IR_CONSTZ("'/'");
        case '!': return IR_CONSTZ("'!'");
        case '=': return IR_CONSTZ("'='");
        case ':': return IR_CONSTZ("':'");
        case ';': return IR_CONSTZ("';'");
        case ',': return IR_CONSTZ("','");
        case '.': return IR_CONSTZ("'.'");
        case '#': return IR_CONSTZ("'#'");
        case '&': return IR_CONSTZ("'&'");
        case '|': return IR_CONSTZ("'|'");
        default: return IR_CONSTZ("TOKEN TYPE PRINT NOT IMPLEMENTED");
    }
}

static
String type_to_str(msi t)
{
    return type_to_str((Token_Type)t);
}


#endif //MAYFLY_MAIN_H
