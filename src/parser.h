#include "tokens.h"

struct Variable{
    Token_Type dataType; //only 1050-1099 allowed
    String name;
    msi id;
    msi level;
};

struct Function{
    Token_Type returnType;
    String name;
    Variable* arguments;
};

Heap_Allocator* p_heap;

s64 p_currentIndex;
Token* p_currentToken;
Token* p_tokens;
b8 p_parsing;

Variable* p_variables;
msi variableCount = 0;
msi p_currentScope;

Function* p_functions;

void increaseScope(){
    p_currentScope++;
}

void decreaseScope(){
    while(ARR_LAST(p_variables)->level==p_currentScope)
        ARR_POP(p_variables);
    p_currentScope--;
}

void nextToken(){
    p_currentIndex++;
    p_currentToken = &p_tokens[p_currentIndex];
    if (p_currentToken->type == TOKEN_EOF)
        p_parsing = false;
}

void previousToken(){
    p_currentIndex--;
    p_currentToken = &p_tokens[p_currentIndex];
    p_parsing = true;
}

Token getPreviousToken(msi howFarBack){
    if (p_currentIndex-howFarBack >= 0)
        return p_tokens[p_currentIndex-howFarBack];
    return Token{};
}

b8 accept(msi tokenType){
    if (p_parsing == 1 && p_currentToken->type == tokenType) {
        nextToken();
        return true;
    }
    return false;
}

b8 expect(msi tokenType){
    if (accept(tokenType))
        return true;
    printf("Error at %llu:%llu, expected: %.*s got: %.*s\n",
           p_currentToken->line,
           p_parsing ? p_currentToken->column : p_currentToken->column + p_currentToken->text.length,
           type_to_str(tokenType),
           type_to_str(p_currentToken->type).length, type_to_str(p_currentToken->type).data);
    exit(EXIT_FAILURE);
}

//this method does identifier checking
//TOKEN_UNKNOWN->not a declaration; TOKEN_S64, TOKEN_F64, TOKEN_B8
b8 inspectId(Token_Type declarationType, Token t){
    if (!declarationType){
        for (msi i=0;i<ARR_LEN(p_variables);i++){
            if (cmp_string(t.text,p_variables[i].name)){
                return true;
            }
        }
        printf("Error, undeclared identifier: %.*s at %u:%u\n", t.text,
               t.line,
               p_parsing ? t.column : t.column + t.text.length);
        exit(EXIT_FAILURE);
    }
    else {
        for (msi i=0;i<ARR_LEN(p_variables);i++){
            if (cmp_string(t.text,p_variables[i].name) && p_currentScope == p_variables[i].level){
                printf("Error, redeclared identifier: %.*s at %u:%u\n", t.text,
                       t.line,
                       p_parsing ? t.column : t.column + t.text.length);
                exit(EXIT_FAILURE);
            }
        }
        Variable v{};
        v.level = p_currentScope;
        v.dataType = declarationType;
        v.id = variableCount;
        v.name = t.text;
        ARR_PUSH(p_variables,v);
        variableCount++;
        return true;
    }
}

b8 acceptId(Token_Type declarationType){
    if (accept(TOKEN_ID)){
        return inspectId(declarationType, getPreviousToken(1));
    }
    return false;
}

b8 expectId(Token_Type declarationType){
    Token t = *p_currentToken;
    if (acceptId(declarationType))
        return true;
    printf("Error, expected: %u at %llu:%llu\n", declarationType,
           t.line,
           p_parsing ? t.column : t.column + t.text.length);
    exit(EXIT_FAILURE);
}

Token_Type expectType(){
    if (accept(TOKEN_S64))
        return TOKEN_S64;
    if (accept(TOKEN_F64))
        return TOKEN_F64;
    if (accept(TOKEN_C8))
        return TOKEN_C8;
    printf("Error, expected type at %llu:%llu\n",
           p_currentToken->line,
           p_parsing ? p_currentToken->column : p_currentToken->column + p_currentToken->text.length);
    exit(EXIT_FAILURE);
}

Token_Type acceptType(){
    if (accept(TOKEN_S64))
        return TOKEN_S64;
    if (accept(TOKEN_F64))
        return TOKEN_F64;
    if (accept(TOKEN_C8))
        return TOKEN_C8;
    return TOKEN_UNKOWN;
}

void expression();
void statements();

void function(b8 declaration){
    if (declaration){
        Token_Type t;
        Function f{};
        expect(TOKEN_FN);
        expect(TOKEN_ID);
        f.name = getPreviousToken(1).text;
        ARR_INIT(f.arguments,4,p_heap);
        expect('(');
        if ((t=acceptType()) != TOKEN_UNKOWN){
            accept('*');
            expectId(t);
            Variable* v = ARR_LAST(p_variables);
            ARR_PUSH(f.arguments, *v);
            while (accept(',')){
                t = expectType();
                accept('*');
                expectId(t);
                v = ARR_LAST(p_variables);
                ARR_PUSH(f.arguments, *v);
            }
        }
        expect(')');
        expect(':');
        f.returnType = expectType(); //no support for void return
        ARR_PUSH(p_functions,f);
        expect('{');
        increaseScope();
        statements();
        decreaseScope();
        expect('}');
    } else {
        //TODO: function signature checking requires knowledge of expression type
        expect(TOKEN_ID);
        expect('(');
        if(!accept(')'))
            expression();
        while (accept(',')){
            expression();
        }
        expect(')');
    }
}

b8 statement(){
    Token_Type type;
    if ((type = acceptType()) != TOKEN_UNKOWN){
        accept('*');
        expectId(type);
        if(accept('='))
            expression();
        expect(';');
    }else if (accept(TOKEN_ID)){
        if (accept('(')){
            previousToken();
            previousToken();
            function(false);
            expect(';');
        }else if (accept('=')){
            inspectId(TOKEN_UNKOWN, getPreviousToken(2));
            expression();
            expect(';');
        }else{
            previousToken();
            expression();
            expect(';');
        }
    }else if (accept(TOKEN_IF)){
        expect('(');
        expression();
        expect(')');
        expect('{');
        increaseScope();
        statements();
        decreaseScope();
        expect('}');
        if (accept(TOKEN_ELSE)){
            expect('{');
            increaseScope();
            statements();
            decreaseScope();
            expect('}');
        }
    }else if (accept(TOKEN_FOR)){
        expect('(');
        increaseScope();
        if((type = acceptType()) != TOKEN_UNKOWN){
            expectId(type);
            expect('=');
        }else if (acceptId(TOKEN_UNKOWN))
            expect('=');
        expression();
        expect(';');
        expression();
        expect(';');
        expectId(TOKEN_UNKOWN);
        expect('=');
        expression();
        expect(')');
        expect('{');
        statements();
        decreaseScope();
        expect('}');
    }else if (accept(TOKEN_WHILE)){
        expect('(');
        expression();
        expect(')');
        expect('{');
        increaseScope();
        statements();
        decreaseScope();
        expect('}');
    }else if (accept(TOKEN_BREAK) || accept(TOKEN_CONT))
        expect(';');
    else if (accept(TOKEN_RETURN)){
        expression();
        expect(';');
    }
    else if (accept('}')){
        previousToken();
        return false;
    }else{
        expression(); //messes with the error msgs
        expect(';');
    }
    return true;
}

void statements(){
    while (statement()){}
}

void term(){
    if (accept(TOKEN_ID)){
        if (accept('(')){
            previousToken();
            previousToken();
            function(false);
        }
        else
            inspectId(TOKEN_UNKOWN, getPreviousToken(1));
    }
    else if (!accept(TOKEN_NUM) && !accept(TOKEN_STR_LIT)){
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
    Token_Type t;
    if ((t = acceptType()) != TOKEN_UNKOWN){
        expectId(t);
        if (accept('='))
            expression();
        expect(';');
    }
    else{
        function(true);
    }
}

void parse(Token* tokens, Heap_Allocator* heap){
    p_tokens = tokens;
    p_currentIndex = -1;
    p_parsing = true;
    ARR_INIT(p_variables,64, heap);
    ARR_INIT(p_functions,8, heap);
    p_heap = heap;
    nextToken();
    while(p_parsing){
        block();
    }
}