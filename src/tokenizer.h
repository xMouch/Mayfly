#ifndef TOKENIZER_H
#define TOKENIZER_H

struct Tokenizer
{
    String file;
    String line_text;
    Token* tokens;
    c8  n[2];
    msi cur;
    msi cur_line;
    msi cur_column;
};

static
Tokenizer create_tokenizer(String file, Heap_Allocator* heap)
{
    Tokenizer t = {};
    t.file = file;
    ARR_INIT(t.tokens, 64, heap);
    
    t.cur_line = 1;
    t.cur_column = 1;
    
    if(file.length >= 1)
    {
        t.n[0]=t.file.data[0];   
    }
    if(file.length >= 2)
    {
        t.n[1]=t.file.data[1];
    }
    
    return t;
    
}

static 
void adv_chars(Tokenizer* t, msi count)
{
    for(msi i = 0; i < count; ++i)
    {
        if(t->cur >= t->file.length)
        {
            t->n[0] = 0;
            t->n[1] = 0;
            break;
        }
        
        ++t->cur_column;
        if(is_end_of_line(t->file.data[t->cur]))
        {
            ++t->cur_line;
            t->cur_column = 1;
            
            msi begin = 1;
            for(;is_whitespace(t->file.data[t->cur+begin]);++begin)
            {}
            
            t->line_text.data = &t->file.data[t->cur+begin];
            
            msi i = 0;
            for(; !is_end_of_line(t->line_text.data[i]); ++i)
            {}
            
            t->line_text.length = i+1;
            
        }
        ++t->cur;
        
        t->n[0] = t->file.data[t->cur];
        
        if(t->cur+1 < t->file.length)
            t->n[1] = t->file.data[t->cur+1];
        else
            t->n[1] = 0;
    }
}

static 
void skip_whitespace_and_comments(Tokenizer* t)
{
    for(;;)
    {
        if(is_whitespace(t->n[0]) || is_end_of_line(t->n[0]))
        {
            adv_chars(t, 1);
            continue;
        }
        //Skip CPP style comment
        if(t->n[0] == '/' && t->n[1] == '/')
        {
            while(!is_end_of_line(t->n[0]))
            {
                adv_chars(t, 1);
            }
            continue;
        }
  
        
        //Skip C style comment
        if(t->n[0] == '/' && t->n[1] == '*')
        {
            while(!(t->n[0] == '*' && t->n[1] == '/'))
            {
                adv_chars(t, 1);
            }  
            adv_chars(t, 2);
            continue;
        }
        
        break;
    }
}

static
Token* tokenize(String file, Heap_Allocator* heap)
{
    IR_NOT_NULL(heap);
    Tokenizer t = create_tokenizer(file, heap);
    
    while(t.n[0] != 0)
    {
        skip_whitespace_and_comments(&t);
        
        Token token = {};
        token.text.data = &t.file.data[t.cur];
        token.text.length = 1;
        token.line = t.cur_line;
        token.column = t.cur_column;
        token.line_text = t.line_text;
        
        
        if(t.n[0] == '=' && t.n[1] == '=')
        {
            token.type = TOKEN_D_EQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }
        
        if(t.n[0] == '&' && t.n[1] == '&')
        {
            token.type = TOKEN_AND;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }
        
        if(t.n[0] == '|' && t.n[1] == '|')
        {
            token.type = TOKEN_OR;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }
        
        if(t.n[0] == '!' && t.n[1] == '=')
        {
            token.type = TOKEN_OR;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;     
        }

        if(t.n[0] == '<' && t.n[1] == '=')
        {
            token.type = TOKEN_LEQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }

        if(t.n[0] == '>' && t.n[1] == '=')
        {
            token.type = TOKEN_GEQ;
            adv_chars(&t, 2);
            token.text.length = 2;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        switch(t.n[0])
        {
            case '(':
            case ')':
            case '[':
            case ']':
            case '{':
            case '}':
            case '<':
            case '>':
            case '%':
            case '+':
            case '-':
            case '*':
            case '/':
            case '!':
            case '=':
            case ':':
            case ';':
            case ',':
            case '.':
            case '#':
            case '&':
            case '|':
            case '^':
            {
                token.type = (Token_Type)t.n[0];
                adv_chars(&t, 1);
                
                ARR_PUSH(t.tokens, token);
                continue;
            }
        }
        
        if(t.n[0] == '"')
        {
            token.type = TOKEN_STR_LIT;
            adv_chars(&t, 1);
            
            token.text.length = 0;
            token.text.data = &t.file.data[t.cur];
            token.line = t.cur_line;
            token.column = t.cur_column;
            
            while(t.n[0] != '"' && t.n[0] != 0)
            {
                
                if(t.n[0] == '\\' && t.n[1] == '"')
                {
                    adv_chars(&t, 2);
                    token.text.length+=2;
                }
                else
                {
                    adv_chars(&t, 1);
                    ++token.text.length;
                }
            }
            
            if(t.n[0] == '"')
                adv_chars(&t, 1);
            
            
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        if(is_alpha(t.n[0]) || t.n[0] == '_')
        {
            token.type = TOKEN_ID;
            
            u32 length = 1;
            adv_chars(&t, 1);
            while( is_alpha(t.n[0]) || is_number(t.n[0]) || t.n[0] == '_')
            {
                adv_chars(&t,1);
                length++;
            }
            
            token.text.length = length;
            
            if(cmp_string(IR_CONSTZ("fn"), token.text))
            {
                token.type = TOKEN_FN;   
            }else if(cmp_string(IR_CONSTZ("return"), token.text))
            {
                token.type = TOKEN_RETURN;   
            }else if(cmp_string(IR_CONSTZ("if"), token.text))
            {
                token.type = TOKEN_IF;   
            }else if(cmp_string(IR_CONSTZ("else"), token.text))
            {
                token.type = TOKEN_ELSE;   
            }else if(cmp_string(IR_CONSTZ("for"), token.text))
            {
                token.type = TOKEN_FOR;   
            }else if(cmp_string(IR_CONSTZ("while"), token.text))
            {
                token.type = TOKEN_WHILE;   
            }else if(cmp_string(IR_CONSTZ("break"), token.text))
            {
                token.type = TOKEN_BREAK;   
            }else if(cmp_string(IR_CONSTZ("continue"), token.text))
            {
                token.type = TOKEN_CONT;   
            }else if(cmp_string(IR_CONSTZ("struct"), token.text))
            {
                token.type = TOKEN_STRUCT;   
            }else if(cmp_string(IR_CONSTZ("s64"), token.text))
            {
                token.type = TOKEN_S64;   
            }else if(cmp_string(IR_CONSTZ("f64"), token.text))
            {
                token.type = TOKEN_F64;   
            }else if(cmp_string(IR_CONSTZ("c8"), token.text))
            {
                token.type = TOKEN_C8;   
            }
            
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        
        if(is_number(t.n[0]))
        {
            token.type = TOKEN_NUM;
            u32 num_length = 0;
            while(is_number(t.n[0])) 
            {
                num_length++;
                adv_chars(&t, 1);
            }
            token.text.length = num_length;
            ARR_PUSH(t.tokens, token);
            continue;
        }
        
        
        
        if(token.type == TOKEN_UNKOWN)
        {
            adv_chars(&t, 1);
            
            //ARR_PUSH(t.tokens, token);
            continue;
        }
    }
    
    Token token = {};
    token.text.data = &t.file.data[t.cur];
    token.text.length = 0;
    token.type = TOKEN_EOF;
    token.line = t.cur_line;
    token.column = t.cur_column;
    ARR_PUSH(t.tokens, token);
    
    return t.tokens;
}


#endif //TOKENIZER_H
