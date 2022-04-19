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


String read_entire_file(c8* file_name, Memory_Arena* arena)
{
    FILE* file;
    file = fopen(file_name, "rb+");
    
    u64 file_size = 0;
    
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    String result;
    result.length = file_size;
    result.data = (u8*)push_size(file_size, arena);
    
    fread(result.data, result.length, 1, file);
    
    fclose(file);
    
    return result;
}


Token* tokenize(String file, Heap_Allocator* heap)
{
    Token* tokens = nullptr;
    ARR_INIT(tokens, 64, heap);
    
    
    c8* cur = file.data;
    msi line = 1;
    msi column = 0;
    while(*cur)
    {
        column++;
        if(is_end_of_line(*cur))
        {
            line++;
            column = 0;
            cur++;
            continue;
        }
        if(is_whitespace(*cur))
        {
            cur++;
            continue;
        }
        
        if(is_alpha(*cur) || *cur == '_')
        {
            Token token;
            token.type = TOKEN_ID;
            token.text.data = cur;
            
            u32 length = 1;
            cur++;
            
            while( is_alpha(*cur) || is_number(*cur) || *cur == '_')
            {
                cur++;
                length++;
            }
            
            token.text.length = length;
            token.line = line;
            token.column = column;
            
            if(cmp_string(IR_CONSTZ("return"), token.text))
            {
                token.type = TOKEN_RETURN;   
            }
            
            ARR_PUSH(tokens, token);
        }
        
        if(is_number(*cur))
        {
            Token num_token;
            num_token.type = TOKEN_NUM;
            num_token.text.data = cur;
            u32 num_length = 0;
            while(is_number(*cur)) 
            {
                num_length++;
                cur++;
            }
            num_token.text.length = num_length;
            num_token.line = line;
            num_token.column = column;
            ARR_PUSH(tokens, num_token);
            continue;
        }
        cur++;
    }
    
    return tokens;
}

int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(256), (u8*)malloc(IR_MEGABYTES(256)));
    
    String file = read_entire_file("testcode/test.mf", &arena);
    
    Heap_Allocator heap = create_heap(&arena, IR_MEGABYTES(64), 0);
    
    Token* tokens = tokenize(file, &heap);
    
    
    for(msi i = 0; i < ARR_LEN(tokens); ++i)
    {
        fprintf(stdout, "Type: %u, (%.*s)\n", tokens[i].type, tokens[i].text);
    }
    
    
    
    return 0;
}
