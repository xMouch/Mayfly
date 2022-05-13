#include "tokens.h"
#include "nodes.h"

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
    newNode.line_text = p_currentToken->line_text;
    ARR_PUSH(p_nodes, newNode);
    return ARR_LAST(p_nodes);
}

Node* derefChain(Node* child, msi count){
    if(count > 0)
        return makeNode({.type=N_DEREF, .left=derefChain(child, count-1)});
    else
        return child;
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
msi inspectId(Token_Type declarationType, msi pointerLvl, Token t){
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
        v.pointerLvl = pointerLvl;
        ARR_PUSH(p_variables,v);
        ARR_PUSH(p_variablesInScope,v);
        return v.id;
    }
}

b8 acceptId(Token_Type declarationType, msi pointerLvl){
    if (accept(TOKEN_ID)){
        inspectId(declarationType, pointerLvl, getPreviousToken(1));
        return true;
    }
    return false;
}

msi expectId(Token_Type declarationType, msi pointerLvl){
    Token t = *p_currentToken;
    if (accept(TOKEN_ID))
        return inspectId(declarationType, pointerLvl, t);
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

Node* assignmentOrExpression(Token_Type varType, b8 emptyAssignAllowed){
    Node* n;
    if(varType != TOKEN_UNKOWN){
        msi pointerLvl = 0;
        while (accept('*'))
            pointerLvl++;
        msi index = expectId(varType, pointerLvl);

        if((emptyAssignAllowed && accept('=')) || (!emptyAssignAllowed && expect('='))){
            n = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.var=&p_variables[index]}),.right=expression()});
        }
        else
            n = makeNode({.type=N_EMPTY});
    }else{
        msi pointerLvl = 0;
        while (accept('*'))
            pointerLvl++;
        if (acceptId(TOKEN_UNKOWN, pointerLvl)){
            msi index = inspectId(TOKEN_UNKOWN, pointerLvl, getPreviousToken(1));
            if(accept('='))
                n = makeNode({.type=N_ASSIGN,.left=derefChain(makeNode({.type=N_VAR,.var=&p_variables[index]}),pointerLvl),.right=expression()});
            else{
                previousToken();
                n = expression();
                n = derefChain(n, pointerLvl);
            }
        }else{
            n = expression();
            n = derefChain(n, pointerLvl);
        }
    }
    return n;
}

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
            msi pointerLvl = 0;
            while (accept('*'))
                pointerLvl++;
            expectId(t, pointerLvl);
            Variable* v = ARR_LAST(p_variablesInScope);
            ARR_PUSH(f.arguments, *v);
            while (accept(',')){
                t = expectType();
                pointerLvl = 0;
                while (accept('*'))
                    pointerLvl++;
                expectId(t, pointerLvl);
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
        Node* n = makeNode({.type=N_FUNC,.func=ARR_LAST(p_functions), .left=statements()});
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
    if (accept('}')){
        previousToken();
        return nullptr;
    }

    if ((type = acceptType()) != TOKEN_UNKOWN){
        n = assignmentOrExpression(type, true);
        expect(';');
        return n;
    }

    if (accept(TOKEN_IF)){
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
        return n;
    }

    if (accept(TOKEN_FOR)){
        expect('(');
        increaseScope();
        Node *n1, *n2, *n3;
        if (!accept(';')){
            type = acceptType();
            n1 = assignmentOrExpression(type, true);
            expect(';');
        }else{
            n1 = makeNode({N_EMPTY});
        }
        if (!accept(';')){
            n2 = assignmentOrExpression(TOKEN_UNKOWN,true);
            expect(';');
        }
        else{
            n2 = makeNode({N_EMPTY});
        }
        if (!accept(')')){
            n3 = assignmentOrExpression(TOKEN_UNKOWN,true);
            expect(')');
        }else
            n3 = makeNode({N_EMPTY});

        expect('{');
        n = makeNode({.type=N_FOR, .left=
                      makeNode({.type=N_FOR, .left=
                      makeNode({.type=N_FOR, .left=n1, .right=n2}), .right=n3}), .right=statements()});
        decreaseScope();
        expect('}');
        return n;
    }

    if (accept(TOKEN_WHILE)){
        expect('(');
        Node* n1 = assignmentOrExpression(TOKEN_UNKOWN,true);
        expect(')');
        expect('{');
        increaseScope();
        n = makeNode({.type=N_WHILE, .left=n1, .right=statements()});
        decreaseScope();
        expect('}');
        return n;
    }

    if (accept(TOKEN_BREAK)){
        n = makeNode({.type=N_BREAK});
        expect(';');
        return n;
    }
    if(accept(TOKEN_CONT)){
        n = makeNode({.type=N_CONTINUE});
        expect(';');
        return n;
    }
    if (accept(TOKEN_RETURN)){
        n = makeNode({.type=N_RETURN, .left=expression()});
        expect(';');
        return n;
    }

    n = assignmentOrExpression(TOKEN_UNKOWN,true);
    expect(';');
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
            msi index = inspectId(TOKEN_UNKOWN, 0, getPreviousToken(1));
                return makeNode({.type=N_VAR, .var=&p_variables[index]});
        }
    }
    else if (accept(TOKEN_NUM)){
        if(search_string_first_occurrence(getPreviousToken(1).text,'.').length>0){
            f64 val = strtod(getPreviousToken(1).text.data, &p_currentToken->text.data);
            return makeNode({.type=N_FLOAT, .fValue=val});
        }else{
            s64 val = strtol(getPreviousToken(1).text.data, &p_currentToken->text.data,10);
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
        return makeNode({.type=N_DEREF,.left=term()});
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

Node* exp6(){
    Node* returnVal = exp4();
    while (accept(TOKEN_LEQ) || accept(TOKEN_GEQ) || accept('<') || accept('>')){
        NodeType n;
        switch (getPreviousToken(1).type) {
            case '<':
                n = N_CMP_LT;
                break;
            case '>':
                n = N_CMP_GT;
                break;
            case TOKEN_LEQ:
                n = N_CMP_LEQ;
                break;
            case TOKEN_GEQ:
                n = N_CMP_GEQ;
                break;
            default:
                n = N_EMPTY;
                IR_INVALID_CASE
        }
        returnVal = makeNode({.type=n,.left=returnVal,.right=exp4()});
    }
    return returnVal;
}

Node* exp7(){
    Node* returnVal = exp6();
    while (accept(TOKEN_D_EQ) || accept(TOKEN_NOTEQ))
        returnVal = makeNode({.type=(getPreviousToken(1).type==TOKEN_D_EQ)?N_CMP_EQ:N_CMP_NEQ,
                              .left=returnVal,.right=exp6()});
    return returnVal;
}

Node* exp8(){
    Node* returnVal = exp7();
    while (accept('&'))
        returnVal = makeNode({.type=N_AND,.left=returnVal,.right=exp7()});
    return returnVal;
}

Node* exp10(){
    Node* returnVal = exp8();
    while (accept('|'))
        returnVal = makeNode({.type=N_OR,.left=returnVal,.right=exp8()});
    return returnVal;
}

Node* exp11(){
    Node* returnVal = exp10();
    while (accept(TOKEN_AND))
        returnVal = makeNode({.type=N_CMP_AND,.left=returnVal,.right=exp10()});
    return returnVal;
}

Node* expression(){
    Node* returnVal = exp11();
    while (accept(TOKEN_OR))
        returnVal = makeNode({.type=N_CMP_OR,.left=returnVal,.right=exp11()});
    returnVal = makeNode({.type=N_EXPR,.left=returnVal,.right=nullptr});
    return returnVal;
}

Node* block(){
    Token_Type t;
    if ((t = acceptType()) != TOKEN_UNKOWN){
        Node* node = nullptr;
        msi pointerLvl = 0;
        while (accept('*'))
            pointerLvl++;
        msi index = expectId(t, pointerLvl);
        if (accept('=')){
                node = makeNode({.type=N_ASSIGN,.left=makeNode({.type=N_VAR,.var=&p_variables[index]}),.right=expression()});
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
void printVar(Variable v){
    printf("%.*s ", type_to_str(v.dataType));
    printf("%.*s", v.pointerLvl, "****************");
    printf("%.*s ", v.name);
}

void printTree(Node* n, s64 offset){
    if (n != nullptr){
        printf("%.*s", offset * 2, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | ");
        if(n->type==N_VAR){
            printVar(*n->var);
            printf("\n");
        }else if(n->type==N_FUNC){
            printf("%.*s %.*s (", type_to_str(n->type),n->func->name);
            for (msi i=0;i< ARR_LEN(n->func->arguments);i++)
                printVar(n->func->arguments[i]);
            printf(")\n");
        }else
            printf("%.*s %u\n", type_to_str(n->type), n->sValue);
        printTree(n->left, offset+1);
        printTree(n->right, offset+1);
    }
}

struct Parser_Result
{
    Node* ast;
    msi reg_max;
    
};

Parser_Result parse(Token* tokens, Heap_Allocator* heap){
    p_tokens = tokens;
    p_currentIndex = -1;
    p_parsing = true;
    ARR_INIT(p_nodes,64, heap);
    ARR_INIT(p_variables,64, heap);
    ARR_INIT(p_variablesInScope,64, heap);
    ARR_INIT(p_functions,8, heap);
    p_heap = heap;
    nextToken();
    Node* ast = program();
    printTree(ast, 0);
    
    Parser_Result result;
    result.ast = ast;
    result.reg_max = ARR_LEN(p_variables)+10;
    
    for(msi i = 0; i < ARR_LEN(p_variables); ++i)
    {
        p_variables[i].id +=10; 
    }
    
    return result;
}