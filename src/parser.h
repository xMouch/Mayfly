#include "tokens.h"
#include "nodes.h"

Heap_Allocator* p_heap;

s64 p_currentIndex;
Token* p_currentToken;
Token* p_tokens;
c8 p_parsing;

Node* p_nodes_bl;
Variable* p_variables_bl;
Variable** p_variablesInScope;
msi p_currentScope;

msi global_cnt = 0;

Function* p_functions_bl;

Token getPreviousToken(msi howFarBack);

DataType tokenTypeToDatatype(Token_Type t){
    switch (t) {
        case TOKEN_F64:
            return F64;
        case TOKEN_S64:
            return S64;
        case TOKEN_C8:
            return C8;
        default:
            IR_INVALID_CASE;
            return S64;
    }
}

NodeType dataTypeToConversionOp(DataType d){
    switch (d) {
        case F64:
            return N_TO_F64;
        case S64:
            return N_TO_S64;
        case C8:
            return N_TO_C8;
        default:
            IR_INVALID_CASE;
            return N_EMPTY;
    }
}

Node* makeNode(Node newNode, s64 textOffset = 0){
    newNode.line_text = getPreviousToken(-textOffset).line_text;
    BL_PUSH(p_nodes_bl, newNode);
    return BL_LAST(p_nodes_bl);
}

void operandError(Token_Type operation, Node node){
    printf("error: invalid operand(s) for operation %.*s\n", type_to_str(operation));
    printf(" %.*s", node.line_text);
    exit(1);
}

void incompatibleDataError(Type left, Type right, String lineText){
    printf("Error incompatible types: %.*s%.*s and %.*s%.*s\n",
           left.pointerLvl, "***********", data_type_to_str(left.dataType),
           right.pointerLvl, "***********", data_type_to_str(right.dataType));
    printf(" %.*s",lineText);
    exit(1);
}

Node* derefChain(Node* child, msi count){
    if(count > 0){
        Node* children = derefChain(child, count-1);
        Type type = {.dataType=children->dataType.dataType, .pointerLvl=children->dataType.pointerLvl-1};
        if (((s64)children->dataType.pointerLvl)-1<0)
            operandError((Token_Type)'*', *child);
        return makeNode({.type=N_DEREF, .dataType=type, .left=children});
    }else
        return child;
}

Type convertNodes(Node** left, Node** right, bool assignment = false){
    if((assignment && (*right)->dataType.pointerLvl != 0 && (*left)->dataType.pointerLvl == 0 && (*left)->dataType.dataType != S64) ||
        (!assignment && ((*right)->dataType.pointerLvl == 0 ^ (*left)->dataType.pointerLvl == 0)))
        incompatibleDataError((*left)->dataType, (*right)->dataType, (*left)->line_text);
    if((*left)->dataType.dataType != (*right)->dataType.dataType){
        if((*left)->dataType.pointerLvl != 0)
            return (*left)->dataType;
        DataType resDataType = assignment?(*left)->dataType.dataType:(DataType)s64_max((*left)->dataType.dataType, (*right)->dataType.dataType);
        if((*left)->dataType.dataType == F64 || (*right)->dataType.dataType == F64){
            if ((*left)->dataType.dataType != resDataType){
                *left = makeNode({.type=dataTypeToConversionOp(resDataType),.dataType={.dataType=resDataType,.pointerLvl=0},.left=*left});
            }
            else{
                *right = makeNode({.type=dataTypeToConversionOp(resDataType),.dataType={.dataType=resDataType,.pointerLvl=0},.left=*right});
            }
        }
        return {.dataType=resDataType, .pointerLvl=0};
    }
    return (*left)->dataType;
}

Type convertNode(Type left, Node** right){
    if(left.pointerLvl == 0 ^ (*right)->dataType.pointerLvl == 0)
        incompatibleDataError(left, (*right)->dataType, (*right)->line_text);
    else if(left.dataType != (*right)->dataType.dataType)
    {
        if (left.pointerLvl != 0)
            return left;
        if (left.dataType == F64 || (*right)->dataType.dataType == F64){
            *right = makeNode({.type=dataTypeToConversionOp(left.dataType),.dataType=left,.left=*right});
        }
    }
    return left;
}

void increaseScope(){
    p_currentScope++;
}

void decreaseScope(){
    while((*ARR_LAST(p_variablesInScope))->level==p_currentScope)
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

c8 accept(msi tokenType){
    if (p_parsing == 1 && p_currentToken->type == tokenType) {
        nextToken();
        return true;
    }
    
    
    return false;
}

c8 expect(msi tokenType){
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
msi inspectId(Token_Type declarationType, msi pointerLvl, Token t, b8 isGlobal = false){
    if (!declarationType){
        for (s64 i=ARR_LEN(p_variablesInScope)-1;i>=0;i--){
            if (cmp_string(t.text,p_variablesInScope[i]->name)){
                return p_variablesInScope[i]->id;
            }
        }
        printf("Error, undeclared identifier: %.*s at %u:%u\n", t.text,
               t.line,
               p_parsing ? t.column : t.column + t.text.length);
        exit(EXIT_FAILURE);
    }
    else {
        for (s64 i=ARR_LEN(p_variablesInScope)-1;i>=0;i--){
            if (cmp_string(t.text,p_variablesInScope[i]->name) && p_currentScope == p_variablesInScope[i]->level){
                printf("Error, redeclared identifier: %.*s at %u:%u\n", t.text,
                       t.line,
                       p_parsing ? t.column : t.column + t.text.length);
                exit(EXIT_FAILURE);
            }
        }
        Variable v{};
        v.level = p_currentScope;
        v.type = {.dataType=tokenTypeToDatatype(declarationType), .pointerLvl=pointerLvl};
        v.id = BL_LEN(p_variables_bl);
        v.name = t.text;
        if(isGlobal)
        {
            v.global_loc = global_cnt++;
        }
        else
        {
            v.global_loc = (msi)-1;
        }
        BL_PUSH(p_variables_bl,v);
        ARR_PUSH(p_variablesInScope, BL_LAST(p_variables_bl));
        return v.id;
    }
}


c8 acceptId(Token_Type declarationType, msi pointerLvl, b8 isGlobal = false){
    if (accept(TOKEN_ID)){
        inspectId(declarationType, pointerLvl, getPreviousToken(1), isGlobal);
        return true;
    }
    return false;
}

msi expectId(Token_Type declarationType, msi pointerLvl, b8 isGlobal = false){
    Token t = *p_currentToken;
    if (accept(TOKEN_ID))
        return inspectId(declarationType, pointerLvl, t, isGlobal);
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

Node* assignmentOrExpression(Token_Type varType, c8 emptyAssignAllowed){
    Node* n;
    if(varType != TOKEN_UNKOWN){
        msi pointerLvl = 0;
        while (accept('*'))
            pointerLvl++;
        msi index = expectId(varType, pointerLvl);

        if((emptyAssignAllowed && accept('=')) || (!emptyAssignAllowed && expect('='))){
            Node* left = makeNode({.type=N_VAR,.var=BL_GET(p_variables_bl, index),
                             .dataType=BL_GET(p_variables_bl, index)->type});
            Node* right = expression();
            Type dataType = convertNodes(&left, &right, true);
            n = makeNode({.type=N_ASSIGN,.dataType=dataType,.left=left,.right=right});
        }
        else
            n = makeNode({.type=N_EMPTY});
    }else{
        msi pointerLvl = 0;
        while (accept('*'))
            pointerLvl++;

        if (accept(TOKEN_ID)){
            if (!accept('(')){
                msi index = inspectId(TOKEN_UNKOWN, pointerLvl, getPreviousToken(1));
                n = makeNode({.type=N_VAR,.var=BL_GET(p_variables_bl, index),
                                 .dataType=BL_GET(p_variables_bl, index)->type});
                Token* curTok = p_currentToken;
                while (accept('[')){
                    if (n->dataType.pointerLvl <= 0)
                        operandError((Token_Type)'[', *n);
                    Node* right = expression();
                    expect(']');
                    n = makeNode({.type=N_DEREF,.dataType={.dataType=n->dataType.dataType, .pointerLvl=n->dataType.pointerLvl-1},.left=n,.right=right});
                }
                if(accept('=')){
                    Node* right = expression();
                    n = derefChain(n, pointerLvl);
                    Type dataType = convertNodes(&n, &right, true);
                    if (n->dataType.dataType == F64 && pointerLvl>0)
                        operandError((Token_Type)'*',*n);
                    n = makeNode({.type=N_ASSIGN,.dataType=dataType,.left=n,.right=right});
                }else{
                    p_currentToken = curTok;
                    previousToken();
                    n = expression();
                    n = derefChain(n, pointerLvl);
                }
            }else{
                previousToken();
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

Node* function(c8 declaration){
    if (declaration){
        Token_Type t;
        Function f{};
        msi line = p_currentToken->line;
        msi column = p_currentToken->column;
        expect(TOKEN_FN);
        expect(TOKEN_ID);
        f.name = getPreviousToken(1).text;
        increaseScope();
        ARR_INIT(f.arguments,4,p_heap);
        expect('(');
        if ((t=acceptType()) != TOKEN_UNKOWN){
            msi pointerLvl = 0;
            while (accept('*'))
                pointerLvl++;
            expectId(t, pointerLvl);
            Variable* v = *ARR_LAST(p_variablesInScope);
            ARR_PUSH(f.arguments, v);
            while (accept(',')){
                t = expectType();
                pointerLvl = 0;
                while (accept('*'))
                    pointerLvl++;
                expectId(t, pointerLvl);
                v = *ARR_LAST(p_variablesInScope);
                ARR_PUSH(f.arguments, v);
            }
        }
        expect(')');
        expect(':');
        msi returnPointerLvl = 0;
        while(accept('*'))
            returnPointerLvl++;
        f.returnType = {.dataType=tokenTypeToDatatype(expectType()), .pointerLvl=returnPointerLvl}; //no support for void return
        for (msi i = 0; i<BL_LEN(p_functions_bl); i++){
            if (cmp_string(BL_GET(p_functions_bl, i)->name, f.name)){
                printf("Error, redeclared function at %llu:%llu\n", line, column);
                exit(1);
            }
        }
        BL_PUSH(p_functions_bl,f);
        expect('{');
        Node* n = makeNode({.type=N_FUNC,.func=BL_LAST(p_functions_bl), .dataType=f.returnType, .left=statements()});
        decreaseScope();
        expect('}');
        return n;
    } else {
        expect(TOKEN_ID);
        Function* f = nullptr;
        for (msi i = 0; i<BL_LEN(p_functions_bl); i++){
            if (cmp_string(BL_GET(p_functions_bl, i)->name, getPreviousToken(1).text)){
                f = BL_GET(p_functions_bl, i);
            }
        }
        if (f == nullptr){
            printf("Error, unknown function %.*s\n", getPreviousToken(1).text);
            exit(1);
        }
        Node* n = makeNode({.type=N_FUNC_CALL,.func=f, .dataType=f->returnType});
        msi currentParam = 0;
        expect('(');
        if(!accept(')')){
            Node* argument = expression();
            if (ARR_LEN(f->arguments)<=currentParam){
                printf("Error, unexpected arguments for function call at %llu:%llu\n",
                       getPreviousToken(1).line,
                       getPreviousToken(1).column);
                exit(1);
            }
            if (argument->dataType.dataType != f->arguments[currentParam]->type.dataType ||
                argument->dataType.pointerLvl != f->arguments[currentParam]->type.pointerLvl){
                convertNode(f->arguments[currentParam]->type, &argument);
            }
            n->left = makeNode({.type=N_FUNC_ARG, .dataType=argument->dataType, .left=argument});
            argument = n->left;
            currentParam++;

            while (accept(',')){
                Node* argument2 = expression();
                if (ARR_LEN(f->arguments)<=currentParam){
                    printf("Error, unexpected arguments for function call at %llu:%llu\n",
                           getPreviousToken(1).line,
                           getPreviousToken(1).column);
                    exit(1);
                }
                if (argument2->dataType.dataType != f->arguments[currentParam]->type.dataType ||
                    argument2->dataType.pointerLvl != f->arguments[currentParam]->type.pointerLvl){
                    convertNode(f->arguments[currentParam]->type, &argument2);
                }
                argument->right = makeNode({.type=N_FUNC_ARG, .dataType=argument2->dataType, .left=argument2});
                argument = argument->right;
                currentParam++;
            }
            expect(')');
        }
        if (currentParam < ARR_LEN(f->arguments)){
            printf("Error, missing arguments for function call at %llu:%llu\n",
                   getPreviousToken(1).line,
                   getPreviousToken(1).column);
            exit(1);
        }
        return n;
    }
}

Node* statement(){
    Token_Type type;
    Node* n;
    if (accept('{')){
        increaseScope();
        n = statements();
        decreaseScope();
        expect('}');
        return n;
    }

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
        s64 ifIndex = p_currentIndex;
        expect('(');
        Node* e = expression();
        expect(')');
        expect('{');
        increaseScope();
        Node* stm1 = statements();
        decreaseScope();
        expect('}');
        if (accept(TOKEN_ELSE)){
            s64 elseIndex = p_currentIndex;
            expect('{');
            increaseScope();
            n = makeNode({.type=N_IF, .left=e, .right =
                          makeNode({.type=N_ELSE, .left=stm1, .right=statements()}, elseIndex - p_currentIndex)},
                         ifIndex - p_currentIndex);
            decreaseScope();
            expect('}');
        }else{
            n = makeNode({.type=N_IF, .left=e, .right = stm1}, ifIndex - p_currentIndex);
        }
        return n;
    }

    if (accept(TOKEN_FOR)){
        s64 forIndex = p_currentIndex;
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
                      makeNode({.type=N_FOR, .left=n1, .right=n2}, forIndex - p_currentIndex), .right=n3},
                               forIndex - p_currentIndex), .right=statements()},forIndex - p_currentIndex);
        decreaseScope();
        expect('}');
        return n;
    }

    if (accept(TOKEN_WHILE)){
        s64 whileIndex = p_currentIndex;
        expect('(');
        Node* n1 = assignmentOrExpression(TOKEN_UNKOWN,true);
        expect(')');
        expect('{');
        increaseScope();
        n = makeNode({.type=N_WHILE, .left=n1, .right=statements()}, whileIndex - p_currentIndex);
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
        Node* left = expression();
        Type returnType = convertNode(BL_LAST(p_functions_bl)->returnType, &left);
        n = makeNode({.type=N_RETURN, .dataType=returnType, .left=left});
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
    return makeNode({.type=N_EMPTY});
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
            return makeNode({.type=N_VAR, .var=BL_GET(p_variables_bl, index),
                                .dataType=BL_GET(p_variables_bl, index)->type});
        }
    }
    else if (accept(TOKEN_NUM)){
        if(search_string_first_occurrence(getPreviousToken(1).text,'.').length>0){
            f64 val = strtod(getPreviousToken(1).text.data, &p_currentToken->text.data);
            return makeNode({.type=N_FLOAT, .fValue=val, .dataType={.dataType=F64, .pointerLvl=0}});
        }else{
            s64 val = strtol(getPreviousToken(1).text.data, &p_currentToken->text.data,10);
            return makeNode({.type=N_NUM, .sValue=val, .dataType={.dataType=S64, .pointerLvl=0}});
        }
    }else if(accept(TOKEN_STR_LIT)){
        return makeNode({.type=N_STR, .str=getPreviousToken(1).text, .dataType={.dataType=C8, .pointerLvl=1}});
    }else{
        expect('(');
        Node* n = expression();
        expect(')');
        return n;
    }
}

Node* exp1(){
    Node* n = term();
    while (accept('[')){
        if (n->dataType.pointerLvl <= 0)
            operandError((Token_Type)'[', *n);
        Node* right = expression();
        expect(']');
        n = makeNode({.type=N_DEREF,.dataType={.dataType=n->dataType.dataType, .pointerLvl=n->dataType.pointerLvl-1},.left=n,.right=right});
    }
    return n;
}

//these are named after precedence level of treated operators
//https://en.cppreference.com/w/c/language/operator_precedence
Node* exp2(){ //TODO: operanderror for pointer types on most operations
    if (accept('-')){
        Node* n = exp1();
        if(n->dataType.pointerLvl>0)
            operandError((Token_Type)'-', *n);
        return makeNode({.type=N_NEG,.dataType=n->dataType,.left=n});
    } else if (accept('*')){
        Node* n = exp1();
        if (n->dataType.pointerLvl == 0)
            operandError((Token_Type)'*',*n);
        return makeNode({.type=N_DEREF,.dataType={.dataType=n->dataType.dataType, .pointerLvl=n->dataType.pointerLvl-1},.left=n});
    } else if (accept('&')){
        Node* n = exp1();
        return makeNode({.type=N_IADDR,.dataType={.dataType=n->dataType.dataType, .pointerLvl=n->dataType.pointerLvl + 1},.left=n});
    } else if (accept('!')){
        Node* n = exp1();
        if(n->dataType.pointerLvl>0)
            operandError((Token_Type)'-', *n);
        return makeNode({.type=N_NOT,.dataType={.dataType=S64, .pointerLvl=0},.left=n});
    } else {
        accept('+');
        return exp1();
    }
}

Node* exp3(){
    Node* returnVal = exp2();
    while (accept('*') || accept('/') || accept('%')){
        if (getPreviousToken(1).type=='%'){
            Node* right=exp2();
            if (right->dataType.dataType == F64 || returnVal->dataType.dataType == F64)
                operandError((Token_Type)'%',*returnVal);

            Type resType = convertNodes(&returnVal, &right);
            returnVal = makeNode({.type=N_MOD,.dataType=resType,
                                  .left=returnVal,.right=right});
        }
        else
        {
            NodeType nodeType = (getPreviousToken(1).type=='*')?N_MUL:N_DIV;
            Node* right=exp2();
            Type resType = convertNodes(&returnVal, &right);
            returnVal = makeNode({.type=nodeType,.dataType=resType,
                                  .left=returnVal,.right=right});
        }
    }
    return returnVal;
}

Node* exp4(){
    Node* returnVal = exp3();
    while (accept('+') || accept('-')){
        NodeType nodeType = (getPreviousToken(1).type=='+')?N_ADD:N_SUB;
        Node* right=exp3();
        Type resType = convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=nodeType,.dataType=resType,
                              .left=returnVal,.right=right});
    }
    return returnVal;
}

Node* exp6(){
    Node* returnVal = exp4();
    while (accept(TOKEN_LEQ) || accept(TOKEN_GEQ) || accept('<') || accept('>')){
        NodeType nodeType;
        switch (getPreviousToken(1).type) {
            case '<':
                nodeType = N_CMP_LT;
                break;
            case '>':
                nodeType = N_CMP_GT;
                break;
            case TOKEN_LEQ:
                nodeType = N_CMP_LEQ;
                break;
            case TOKEN_GEQ:
                nodeType = N_CMP_GEQ;
                break;
            default:
                nodeType = N_EMPTY;
                IR_INVALID_CASE
        }
        Node* right=exp4();
        convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=nodeType,.dataType={.dataType=C8, .pointerLvl=0},.left=returnVal,.right=right}); //for cmp ops datatype of children is of importance
    }
    return returnVal;
}

Node* exp7(){
    Node* returnVal = exp6();
    while (accept(TOKEN_D_EQ) || accept(TOKEN_NOTEQ)){
        NodeType nodeType = (getPreviousToken(1).type==TOKEN_D_EQ)?N_CMP_EQ:N_CMP_NEQ;
        Node* right=exp6();
        convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=nodeType,.dataType={.dataType=S64, .pointerLvl=0},
                              .left=returnVal,.right=right});
    }
    return returnVal;
}

Node* exp8(){
    Node* returnVal = exp7();
    while (accept('&')){
        Node* right=exp7();
        if (right->dataType.dataType == F64 || returnVal->dataType.dataType == F64)
            operandError((Token_Type)'&',*returnVal);
        Type resType = convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=N_AND,.dataType=resType,.left=returnVal,.right=right});
    }
    return returnVal;
}

Node* exp9(){
    Node* returnVal = exp8();
    while (accept('^')){
        Node* right=exp8();
        if (right->dataType.dataType == F64 || returnVal->dataType.dataType == F64)
            operandError((Token_Type)'^',*returnVal);
        Type resType = convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=N_XOR,.dataType=resType,.left=returnVal,.right=right});
    }
    return returnVal;
}

Node* exp10(){
    Node* returnVal = exp9();
    while (accept('|')){
        Node* right=exp9();
        if (right->dataType.dataType == F64 || returnVal->dataType.dataType == F64)
            operandError((Token_Type)'|',*returnVal);
        Type resType = convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=N_OR,.dataType=resType,.left=returnVal,.right=right});
    }
    return returnVal;
}

Node* exp11(){
    Node* returnVal = exp10();
    while (accept(TOKEN_AND)){
        Node* right=exp10();
        convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=N_CMP_AND,.dataType={.dataType=S64, .pointerLvl=0},.left=returnVal,.right=right});
    }
    return returnVal;
}

Node* expression(){
    Node* returnVal = exp11();
    while (accept(TOKEN_OR)){
        Node* right=exp11();
        convertNodes(&returnVal, &right);
        returnVal = makeNode({.type=N_CMP_OR,.dataType={.dataType=S64, .pointerLvl=0},.left=returnVal,.right=right});
    }
    returnVal = makeNode({.type=N_EXPR,.dataType=returnVal->dataType,.left=returnVal,.right=nullptr});
    return returnVal;
}

Node* block(){
    Token_Type t;
    if ((t = acceptType()) != TOKEN_UNKOWN){
        Node* node = nullptr;
        msi pointerLvl = 0;
        while (accept('*'))
            pointerLvl++;
        msi index = expectId(t, pointerLvl, true);
        if (accept('=')){
            Node* left = makeNode({.type=N_VAR,.var=BL_GET(p_variables_bl, index),
                                      .dataType=BL_GET(p_variables_bl, index)->type});
            Node* right = expression();
            Type type = convertNodes(&left, &right, true);
            node = makeNode({.type=N_ASSIGN, .dataType=type, .left=left,.right=right});
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
    printf("%.*s ", data_type_to_str(v.type.dataType));
    printf("%.*s", v.type.pointerLvl, "****************");
    printf("%.*s ", v.name);
}

void printTree(Node* n, s64 offset){
    if (n != nullptr){
        printf("%.*s", offset * 2, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | ");
        if(n->type==N_VAR){
            printVar(*n->var);
        }else if(n->type==N_FUNC||n->type==N_FUNC_CALL){
            printf("%.*s %.*s (", type_to_str(n->type),n->func->name);
            for (msi i=0;i< ARR_LEN(n->func->arguments);i++)
                printVar(*n->func->arguments[i]);
            printf(")");
        }else if(n->type==N_FLOAT)
            printf("%.*s %f", type_to_str(n->type), n->fValue);
        else
            printf("%.*s %u", type_to_str(n->type), n->sValue);
        printf(" %.*s%.*s", n->dataType.pointerLvl, "********************",data_type_to_str(n->dataType.dataType));
        printf("\n");
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
    BL_INIT(p_nodes_bl, 64, heap);
    BL_INIT(p_variables_bl,64, heap);
    BL_INIT(p_functions_bl,64, heap);
    ARR_INIT(p_variablesInScope,64, heap);

    Function builtIn = {};
    builtIn.name = IR_CONSTZ("realloc");
    builtIn.returnType = {.dataType=S64, .pointerLvl=1};
    builtIn.jmp_loc = (s64)-1;
    ARR_INIT(builtIn.arguments, 2, heap);
    Variable v = {};
    v.type = {.dataType=S64, .pointerLvl=1};
    v.name = IR_CONSTZ("ptr");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    v.type = {.dataType=S64, .pointerLvl=0};
    v.name = IR_CONSTZ("new_size");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    BL_PUSH(p_functions_bl, builtIn);

    builtIn.name = IR_CONSTZ("printS64");
    builtIn.returnType = {.dataType=S64, .pointerLvl=0};
    builtIn.jmp_loc = (s64)-2;
    ARR_INIT(builtIn.arguments, 1, heap);
    v.type = {.dataType=S64, .pointerLvl=0};
    v.name = IR_CONSTZ("value");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    BL_PUSH(p_functions_bl, builtIn);

    builtIn.name = IR_CONSTZ("printF64");
    builtIn.returnType = {.dataType=S64, .pointerLvl=0};
    builtIn.jmp_loc = (s64)-3;
    ARR_INIT(builtIn.arguments, 1, heap);
    v.type = {.dataType=F64, .pointerLvl=0};
    v.name = IR_CONSTZ("value");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    BL_PUSH(p_functions_bl, builtIn);

    builtIn.name = IR_CONSTZ("print");
    builtIn.returnType = {.dataType=S64, .pointerLvl=0};
    builtIn.jmp_loc = (s64)-4;
    ARR_INIT(builtIn.arguments, 1, heap);
    v.type = {.dataType=C8, .pointerLvl=1};
    v.name = IR_CONSTZ("string");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    BL_PUSH(p_functions_bl, builtIn);

    builtIn.name = IR_CONSTZ("stbi_write_bmp");
    builtIn.returnType = {.dataType=S64, .pointerLvl=0};
    builtIn.jmp_loc = (s64)-5;
    ARR_INIT(builtIn.arguments, 5, heap);
    v.type = {.dataType=C8, .pointerLvl=1};
    v.name = IR_CONSTZ("filename");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    v.type = {.dataType=S64, .pointerLvl=0};
    v.name = IR_CONSTZ("w");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    v.type = {.dataType=S64, .pointerLvl=0};
    v.name = IR_CONSTZ("h");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    v.type = {.dataType=S64, .pointerLvl=0};
    v.name = IR_CONSTZ("comp");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    v.type = {.dataType=C8, .pointerLvl=1};
    v.name = IR_CONSTZ("data");
    v.global_loc = -1;
    ARR_PUSH(builtIn.arguments, BL_PUSH(p_variables_bl, v));
    BL_PUSH(p_functions_bl, builtIn);

    p_heap = heap;
    nextToken();
    Node* ast = program();
    //printTree(ast, 0);
    
    Parser_Result result;
    result.ast = ast;
    result.reg_max = BL_LEN(p_variables_bl);
   
    return result;
}