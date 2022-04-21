#ifndef TOKENIZER_H
#define TOKENIZER_H

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


#endif //TOKENIZER_H
