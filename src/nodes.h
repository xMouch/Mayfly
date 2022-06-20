#ifndef MAYFLY_NODES_H
#define MAYFLY_NODES_H

enum NodeType{
    N_EMPTY,
    N_ASSIGN,
    N_EXPR,
    N_FUNC,
    N_FUNC_CALL,
    N_FUNC_ARG,
    // OPERATORS
    N_OR,
    N_AND,
    N_XOR,
    N_ADD,
    N_SUB,
    N_MUL,
    N_DIV,
    N_MOD,
    N_NEG,
    N_DEREF,
    N_NOT,
    N_CMP_EQ,
    N_CMP_NEQ,
    N_CMP_AND,
    N_CMP_OR,
    N_CMP_LT,
    N_CMP_GT,
    N_CMP_LEQ,
    N_CMP_GEQ,
    //
    N_TO_F64,
    N_TO_S64,
    N_TO_C8,
    //
    N_IF,
    N_ELSE,
    N_FOR,
    N_WHILE,
    N_BREAK,
    N_CONTINUE,
    N_RETURN,
    N_BLOCK,
    N_STMNT,
    N_NUM,
    N_STR,
    N_FLOAT,
    N_VAR
};

enum DataType{
    C8, S64, F64
};

struct Variable{
    DataType dataType;
    msi pointerLvl; //0 -> not a pointer
    String name;
    msi id;
    msi level;
    msi global_loc; // is -1 when not global
    msi reg;
};

struct Function{
    Token_Type returnType;
    String name;
    Variable** arguments;
    
    msi jmp_loc;
    msi context_num;
};

struct Node{
    NodeType type;
    String line_text;
    union{
        s64 sValue;
        f64 fValue;
        c8 cValue;
        String str;
        Variable* var;
        Function* func;
    };
    DataType dataType;
    Node* left;
    Node* right;
};

static
String data_type_to_str(DataType dt)
{
    switch(dt)
    {
        case C8:  return IR_CONSTZ("C8");
        case S64: return IR_CONSTZ("S64");
        case F64:  return IR_CONSTZ("F64");
        default: return IR_CONSTZ("DATA TYPE PRINT NOT IMPLEMENTED");
    }
}

static
String type_to_str(NodeType t)
{
    switch(t)
    {
        case N_EMPTY:  return IR_CONSTZ("N_EMPTY");
        case N_EXPR: return IR_CONSTZ("N_EXPR");
        case N_ASSIGN:  return IR_CONSTZ("N_ASSIGN");
        case N_FUNC:  return IR_CONSTZ("N_FUNC");
        case N_FUNC_CALL:  return IR_CONSTZ("N_FUNC_CALL");
        case N_FUNC_ARG:  return IR_CONSTZ("N_FUNC_ARG");
        case N_OR:  return IR_CONSTZ("N_OR");
        case N_AND:  return IR_CONSTZ("N_AND");
        case N_XOR:  return IR_CONSTZ("N_XOR");
        case N_ADD:  return IR_CONSTZ("N_ADD");
        case N_SUB:  return IR_CONSTZ("N_SUB");
        case N_MUL:  return IR_CONSTZ("N_MUL");
        case N_DIV:  return IR_CONSTZ("N_DIV");
        case N_MOD:  return IR_CONSTZ("N_MOD");
        case N_NEG:  return IR_CONSTZ("N_NEG");
        case N_DEREF:  return IR_CONSTZ("N_DEREF");
        case N_NOT:  return IR_CONSTZ("N_NOT");
        case N_CMP_EQ:  return IR_CONSTZ("N_CMP_EQ");
        case N_CMP_NEQ:  return IR_CONSTZ("N_CMP_NEQ");
        case N_CMP_AND:  return IR_CONSTZ("N_CMP_AND");
        case N_CMP_OR:  return IR_CONSTZ("N_CMP_OR");
        case N_CMP_LT:  return IR_CONSTZ("N_CMP_LT");
        case N_CMP_GT:  return IR_CONSTZ("N_CMP_GT");
        case N_CMP_LEQ:  return IR_CONSTZ("N_CMP_LEQ");
        case N_CMP_GEQ:  return IR_CONSTZ("N_CMP_GEQ");
        case N_TO_F64:  return IR_CONSTZ("N_TO_F64");
        case N_TO_S64:  return IR_CONSTZ("N_TO_S64");
        case N_TO_C8:  return IR_CONSTZ("N_TO_C8");
        case N_IF:  return IR_CONSTZ("N_IF");
        case N_ELSE:  return IR_CONSTZ("N_ELSE");
        case N_FOR:  return IR_CONSTZ("N_FOR");
        case N_WHILE:  return IR_CONSTZ("N_WHILE");
        case N_BREAK:  return IR_CONSTZ("N_BREAK");
        case N_CONTINUE:  return IR_CONSTZ("N_CONTINUE");
        case N_RETURN:  return IR_CONSTZ("N_RETURN");
        case N_BLOCK:  return IR_CONSTZ("N_BLOCK");
        case N_STMNT:  return IR_CONSTZ("N_STMNT");
        case N_NUM:  return IR_CONSTZ("N_NUM");
        case N_STR:  return IR_CONSTZ("N_STR");
        case N_FLOAT:  return IR_CONSTZ("N_FLOAT");
        case N_VAR:  return IR_CONSTZ("N_VAR");
        default: return IR_CONSTZ("NODE TYPE PRINT NOT IMPLEMENTED");
    }
}

#endif //MAYFLY_NODES_H
