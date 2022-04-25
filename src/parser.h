#include "tokens.h"
#include "nodes.h"

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

Node* p_nodes;
Variable* p_variables;
Variable* p_variablesInScope;
msi p_currentScope;

Function* p_functions;

Node* makeNode(Node newNode){
    ARR_PUSH(p_nodes, newNode);
    return ARR_LAST(p_nodes);
}

void increaseScope(){
    p_currentScope++;
}

void decreaseScope(){
    while(ARR_LAST(p_variablesInScope)->level==p_currentScope)
        ARR_POP(p_variablesInScope);
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
msi inspectId(Token_Type declarationType, Token t){
    if (!declarationType){
        for (msi i=0;i<ARR_LEN(p_variablesInScope);i++){
            if (cmp_string(t.text,p_variablesInScope[i].name)){
                return i;
            }
        }
        printf("Error, undeclared identifier: %.*s at %u:%u\n", t.text,
               t.line,
               p_parsing ? t.column : t.column + t.text.length);
        exit(EXIT_FAILURE);
    }
    else {
        for (msi i=0;i<ARR_LEN(p_variablesInScope);i++){
            if (cmp_string(t.text,p_variablesInScope[i].name) && p_currentScope == p_variablesInScope[i].level){
                printf("Error, redeclared identifier: %.*s at %u:%u\n", t.text,
                       t.line,
                       p_parsing ? t.column : t.column + t.text.length);
                exit(EXIT_FAILURE);
            }
        }
        Variable v{};
        v.level = p_currentScope;
        v.dataType = declarationType;
        v.id = ARR_LEN(p_variables);
        v.name = t.text;
        ARR_PUSH(p_variables,v);
        ARR_PUSH(p_variablesInScope,v);
        return v.id;
    }
}

b8 acceptId(Token_Type declarationType){
    if (accept(TOKEN_ID)){
        inspectId(declarationType, getPreviousToken(1));
        return true;
    }
    return false;
}

msi expectId(Token_Type declarationType){
    Token t = *p_currentToken;
    if (accept(TOKEN_ID))
        return inspectId(declarationType, t);
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

Node* expression();
Node* statements();

Node* function(b8 declaration){
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
            Variable* v = ARR_LAST(p_variablesInScope);
            ARR_PUSH(f.arguments, *v);
            while (accept(',')){
                t = expectType();
                accept('*');
                expectId(t);
                v = ARR_LAST(p_variablesInScope);
                ARR_PUSH(f.arguments, *v);
            }
        }
        expect(')');
        expect(':');
        f.returnType = expectType(); //no support for void return
        ARR_PUSH(p_functions,f);
        expect('{');
        increaseScope();
        Node* n = makeNode({.type=N_FUNC,.sValue=ARR_LEN_S(p_functions)-1, .left=statements()});
        decreaseScope();
        expect('}');
        return n;
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
        return nullptr;
    }
}

Node* statement(){
    Token_Type type;
    Node* n = nullptr;
    if ((type = acceptType()) != TOKEN_UNKOWN){
        accept('*');
        msi index = expectId(type);
        if(accept('=')){
            n = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.sValue=(s64)index}),.right=expression()});
        }
        else
            n = makeNode({.type=N_EMPTY});
        expect(';');
    }else if (accept(TOKEN_ID)){
        if (accept('(')){
            previousToken();
            previousToken();
            n = function(false);
            expect(';');
        }else if (accept('=')){
            msi index = inspectId(TOKEN_UNKOWN, getPreviousToken(2));
            n = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.sValue=(s64)index}),.right=expression()});
            expect(';');
        }else{
            previousToken();
            n = expression();
            expect(';');
        }
    }else if (accept(TOKEN_IF)){
        expect('(');
        Node* e = expression();
        expect(')');
        expect('{');
        increaseScope();
        Node* stm1 = statements();
        decreaseScope();
        expect('}');
        if (accept(TOKEN_ELSE)){
            expect('{');
            increaseScope();
            n = makeNode({.type=N_IF, .left=e, .right = makeNode({.type=N_ELSE, .left=stm1, .right=statements()})});
            decreaseScope();
            expect('}');
        }else{
            n = makeNode({.type=N_IF, .left=e, .right = stm1});
        }
    }else if (accept(TOKEN_FOR)){
        expect('(');
        increaseScope();
        Node* n1;
        if((type = acceptType()) != TOKEN_UNKOWN){
            msi index = expectId(type);
            expect('=');
            n1 = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.sValue=(s64)index}),.right=expression()});
        }else if (acceptId(TOKEN_UNKOWN)){
            msi index = inspectId(TOKEN_UNKOWN, getPreviousToken(1));
            expect('=');
            n1 = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.sValue=(s64)index}),.right=expression()});
        }else{
            n1 = expression();
        }
        expect(';');
        Node* n2 = expression();
        expect(';');
        expectId(TOKEN_UNKOWN);
        expect('=');
        Node* n3 = expression();
        expect(')');
        expect('{');
        n = makeNode({.type=N_FOR, .left=
                      makeNode({.type=N_FOR, .left=
                      makeNode({.type=N_FOR, .left=n1, .right=n2}), .right=n3}), .right=statements()});
        decreaseScope();
        expect('}');
    }else if (accept(TOKEN_WHILE)){
        expect('(');
        Node* n1 = expression();
        expect(')');
        expect('{');
        increaseScope();
        n = makeNode({.type=N_WHILE, .left=n1, .right=statements()});
        decreaseScope();
        expect('}');
    }else if (accept(TOKEN_BREAK)){
        n = makeNode({.type=N_BREAK});
        expect(';');
    }
    else if(accept(TOKEN_CONT)){
        n = makeNode({.type=N_CONTINUE});
        expect(';');
    }else if (accept(TOKEN_RETURN)){
        n = makeNode({.type=N_RETURN, .left=expression()});
        expect(';');
    }
    else if (accept('}')){
        previousToken();
        return nullptr;
    }else{
        n = expression(); //messes with the error msgs
        expect(';');
    }
    return n;
}

Node* statements(){
    Node* n1;
    if ((n1=statement()) != nullptr){
        return makeNode({.type=N_STMNT, .left=n1, .right=statements()});
    }
    return nullptr;
}

Node* term(){
    if (accept(TOKEN_ID)){
        if (accept('(')){
            previousToken();
            previousToken();
            return function(false);
        }
        else{
            msi index = inspectId(TOKEN_UNKOWN, getPreviousToken(1));
            return makeNode({.type=N_VAR, .sValue=(s64)index});
        }
    }
    else if (accept(TOKEN_NUM)){
        if(search_string_first_occurrence(getPreviousToken(1).text,'.').length>0){
            f64 val = strtod(getPreviousToken(1).text.data, &p_currentToken->text.data);
            return makeNode({.type=N_FLOAT, .fValue=val});
        }else{
            s64 val = strtol(getPreviousToken(1).text.data, &p_currentToken->text.data,10);
            if (val == 0 || val == 1)
                return makeNode({.type=N_BOOL, .bValue=(b8)val});
            else
                return makeNode({.type=N_NUM, .sValue=val});
        }
    }else if(accept(TOKEN_STR_LIT)){
        return makeNode({.type=N_STR, .str=getPreviousToken(1).text});
    }else{
        expect('(');
        Node* n = expression();
        expect(')');
        return n;
    }
}

//these are named after precedence level of treated operators
//https://en.cppreference.com/w/c/language/operator_precedence
Node* exp2(){
    if (accept('-')){
        return makeNode({.type=N_NEG,.left=term()});
    } else if (accept('*')){
        return makeNode({.type=N_POINTER,.left=term()});
    } else if (accept('!')){
        return makeNode({.type=N_NOT,.left=term()});
    } else {
        accept('+');
        return term();
    }
}

Node* exp3(){
    Node* returnVal = exp2();
    while (accept('*') || accept('/') || accept('%'))
        returnVal = makeNode({.type=(getPreviousToken(1).type=='*')?N_MUL:
                              (getPreviousToken(1).type=='/')?N_DIV:N_MOD,
                              .left=returnVal,.right=exp2()});
    return returnVal;
}

Node* exp4(){
    Node* returnVal = exp3();
    while (accept('+') || accept('-'))
        returnVal = makeNode({.type=(getPreviousToken(1).type=='+')?N_ADD:N_SUB,
                              .left=returnVal,.right=exp3()});
    return returnVal;
}

Node* exp7(){
    Node* returnVal = exp4();
    while (accept(TOKEN_D_EQ) || accept(TOKEN_NOTEQ))
        returnVal = makeNode({.type=(getPreviousToken(1).type==TOKEN_D_EQ)?N_EQ:N_NOT_EQ,
                              .left=returnVal,.right=exp4()});
    return returnVal;
}

Node* exp11(){
    Node* returnVal = exp7();
    while (accept(TOKEN_AND))
        returnVal = makeNode({.type=N_AND,.left=returnVal,.right=exp7()});
    return returnVal;
}

Node* expression(){
    Node* returnVal = exp11();
    while (accept(TOKEN_OR))
        returnVal = makeNode({.type=N_OR,.left=returnVal,.right=exp11()});
    return returnVal;
}

Node* block(){
    Token_Type t;
    if ((t = acceptType()) != TOKEN_UNKOWN){
        Node* node = nullptr;
        msi index = expectId(t);
        if (accept('=')){
            node = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.sValue=(s64)index}),.right=expression()});
        }
        expect(';');
        return node;
    }
    else{
        return function(true);
    }
}

Node* program(){
    if (p_parsing){
        return makeNode({.type=N_BLOCK, .left=block(), .right=program()});
    }
    return nullptr;
}

void printTree(Node* n, s64 offset){
    if (n != nullptr){
        printf("%.*s", offset * 2, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | ");
        printf("%.*s %u\n", type_to_str(n->type), n->sValue);
        printTree(n->left, offset+1);
        printTree(n->right, offset+1);
    }
}

void parse(Token* tokens, Heap_Allocator* heap){
    p_tokens = tokens;
    p_currentIndex = -1;
    p_parsing = true;
    ARR_INIT(p_nodes,64, heap);
    ARR_INIT(p_variables,64, heap);
    ARR_INIT(p_variablesInScope,64, heap);
    ARR_INIT(p_functions,8, heap);
    p_heap = heap;
    nextToken();
    Node* n = program();
    printTree(n, 0);
}