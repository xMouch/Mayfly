#ifndef MAYFLY_NODES_H
#define MAYFLY_NODES_H

enum NodeType{
    N_EMPTY,
    N_ASSIGN,
    N_FUNC,
    N_OR,
    N_AND,
    N_EQ,
    N_NOT_EQ,
    N_ADD,
    N_SUB,
    N_MUL,
    N_DIV,
    N_MOD,
    N_NEG,
    N_DEREF,
    N_NOT,
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

struct Node{
    NodeType type;
    union{
        s64 sValue;
        f64 fValue;
        b8 bValue;
        String str;
    };
    Node* left;
    Node* right;
};

static
String type_to_str(NodeType t)
{
    switch(t)
    {
        case N_EMPTY:  return IR_CONSTZ("N_EMPTY");
        case N_ASSIGN:  return IR_CONSTZ("N_ASSIGN");
        case N_FUNC:  return IR_CONSTZ("N_FUNC");
        case N_OR:  return IR_CONSTZ("N_OR");
        case N_AND:  return IR_CONSTZ("N_AND");
        case N_EQ:  return IR_CONSTZ("N_EQ");
        case N_NOT_EQ:  return IR_CONSTZ("N_NOT_EQ");
        case N_ADD:  return IR_CONSTZ("N_ADD");
        case N_SUB:  return IR_CONSTZ("N_SUB");
        case N_MUL:  return IR_CONSTZ("N_MUL");
        case N_DIV:  return IR_CONSTZ("N_DIV");
        case N_MOD:  return IR_CONSTZ("N_MOD");
        case N_NEG:  return IR_CONSTZ("N_NEG");
        case N_DEREF:  return IR_CONSTZ("N_DEREF");
        case N_NOT:  return IR_CONSTZ("N_NOT");
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
