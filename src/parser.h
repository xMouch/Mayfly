#include "tokens.h"

s64 _currentIndex;
Token* _currentToken;
Token* _tokens;
msi _parsing;

void nextToken(){
    if (_currentIndex >= (s64)ARR_LEN(_tokens)-1){
        _parsing = 0;
    } else {
        _currentIndex++;
        _currentToken = &_tokens[_currentIndex];
    }
}

b8 accept(msi tokenType){
    if (_parsing == 1 && _currentToken->type == tokenType) {
        nextToken();
        return 1;
    }
    return 0;
}

b8 expect(msi tokenType){
    if (accept(tokenType))
        return 1;
    printf("Error, expected: %u at %u:%u", tokenType,
               _currentToken->line,
               _parsing?_currentToken->column:_currentToken->column+_currentToken->text.length);
    exit(EXIT_FAILURE);
    return 0;
}

b8 expectType(){
    if (accept(TOKEN_S64) || accept(TOKEN_F64) || accept(TOKEN_C8))
        return 1;
    printf("Error, expected type at %u:%u",
           _currentToken->line,
           _parsing?_currentToken->column:_currentToken->column+_currentToken->text.length);
    exit(EXIT_FAILURE);
    return 0;
}

b8 acceptType(){
    if (accept(TOKEN_S64) || accept(TOKEN_F64) || accept(TOKEN_C8))
        return 1;
    return 0;
}

void expression();
void statements();

b8 statement(){
    if (acceptType()){
        expect(TOKEN_ID);
        expect('=');
        expression();
        expect(';');
    }else if (accept(TOKEN_ID)){ //TODO: add logic to save previous identifiers
        if (accept('(')){
            //function call TODO: count necessary arguments beforehand
            expression();
            while (accept(',')){
                expression();
            }
            expect(')');
            expect(';');
        }else{
            expect('=');
            expression();
            expect(';');
        }
    }else if (accept(TOKEN_IF)){
        expect('(');
        expression();
        expect(')');
        expect('{');
        statements();
        expect('}');
        if (accept(TOKEN_ELSE)){
            expect('{');
            statements();
            expect('}');
        }
    }else if (accept(TOKEN_FOR)){
        expect('(');
        if(acceptType()){
            expect(TOKEN_ID);
            expect('=');
        }else if (accept(TOKEN_ID))
            expect('=');
        expression();
        expect(';');
        expression();
        expect(';');
        expect(TOKEN_ID);
        expect('=');
        expression();
        expect(')');
        expect('{');
        statements();
        expect('}');
    }else if (accept(TOKEN_WHILE)){
        expect('(');
        expression();
        expect(')');
        expect('{');
        statements();
        expect('}');
    }else if (accept(TOKEN_BREAK))
        expect(';');
    else if (accept(TOKEN_CONT))
        expect(';');
    else if (accept(TOKEN_RETURN))
        expect(';');
    else
        return 0;
    return 1;
}

void statements(){
    while (statement()){}
}

void term(){
    if (!accept(TOKEN_ID)&&!accept(TOKEN_NUM)&&!accept(TOKEN_STR_LIT)){
        expect('(');
        expression();
        expect(')');
    }
}

//these are named after precedence level of treated operators
//https://en.cppreference.com/w/c/language/operator_precedence
void exp2(){
    if (accept('+') || accept('-') || accept('*') || accept('!')){}
    term();
}

void exp3(){
    exp2();
    while (accept('*') || accept('/') || accept('%'))
        exp2();
}

void exp4(){
    exp3();
    while (accept('+') || accept('-'))
        exp3();
}

void exp7(){
    exp4();
    while (accept(TOKEN_D_EQ) || accept(TOKEN_NOTEQ))
        exp4();
}

void exp11(){
    exp7();
    while (accept(TOKEN_AND))
        exp7();
}

void expression(){
    exp11();
    while (accept(TOKEN_OR))
        exp11();
}

void block(){
    if (acceptType()){
        expect(TOKEN_ID);
        if (accept('='))
            expression();
        expect(';');
    }
    else{
        expect(TOKEN_FN);
        expect(TOKEN_ID);
        expect('(');
        if (acceptType()){
            expect(TOKEN_ID);
            while (accept(',')){
                expectType();
                expect(TOKEN_ID);
            }
        }
        expect(')');
        expect(':');
        expectType(); //no support for void return
        expect('{');
        statements();
        expect('}');
    }
}

void parse(Token* tokens){
    _tokens = tokens;
    _currentIndex = -1;
    _parsing = 1;
    nextToken();
    while(_parsing){
        block();
    }
}