#ifndef GENERATOR_H
#define GENERATOR_H

#include "nodes.h"

#pragma pack(push, 1)



enum Opcodes
{
    //R_TYPES
    //INTEGER
    OP_ADD=0,
    OP_SUB,
    OP_MUL,
    OP_DIV,
    OP_MOD,
    OP_AND,
    OP_OR,
    OP_SHIFT_R,
    OP_SHIFT_L,
    
    
    //COMPARE
    OP_CMP_AND,
    OP_CMP_OR,
    OP_CMP_EQ,
    OP_CMP_LT,
    OP_CMP_GT,
    
    
    //I_TYPES
    //INTEGER
    OP_IADD = 32,
    OP_ISUB,
    OP_IMUL,
    OP_IDIV,
    OP_IMOD,
    OP_IAND,
    OP_IOR,
    OP_ISHIFT_R,
    OP_ISHIFT_L,
    
    //COMPARE
    OP_ICMP_AND,
    OP_ICMP_OR,
    OP_ICMP_EQ,
    OP_ICMP_LT,
    OP_ICMP_GT,
    
    //LOAD/STORE
    OP_LOAD64,
    OP_LOAD8,
    OP_STORE64,
    OP_STORE8,

    
    //Branch
    OP_BEQ,
    OP_BNE,
    
    //J_Types
    OP_JMP = 64,
    
    /*
    //STACK
    OP_PUSH, //
    OP_POP,
    */
};

union Instr
{
    struct
    {
        u8 opcode;
        u8 dest;
        u8 op1;
        u8 op2;
        u8 shift;
        u32 __ : 24;
    }R;
    struct
    {
        u8 opcode;
        u8 dest;
        u8 op;
        s32 imm;
        u8 shift;
    }I;
    struct
    {
        u8 opcode;
        u64 jmp : 54;
    }J;
    u64 code;
};

#pragma pack(pop)




struct Meta_Function
{
    Function* func;
    msi jmp_loc;
};

struct Meta_Variable
{
    Variable* var;
    msi index;
    //b8 on_stack;
};


struct Metadata
{
    Meta_Function* funcs;
    Meta_Variable* var;
    
    Meta_Function* cur_func;
    msi cur_line;
    msi treg_cnt;
};

static
String opcode_to_str(Opcodes opcode)
{
    switch(opcode)
    {
        case OP_ADD:  return IR_CONSTZ("OP_ADD");
        case OP_SUB:  return IR_CONSTZ("OP_SUB");
        case OP_MUL:  return IR_CONSTZ("OP_MUL");
        case OP_DIV:  return IR_CONSTZ("OP_DIV");
        case OP_MOD:  return IR_CONSTZ("OP_MOD");
        case OP_AND:  return IR_CONSTZ("OP_AND");
        case OP_OR:  return IR_CONSTZ("OP_OR");
        case OP_SHIFT_R:  return IR_CONSTZ("OP_SHIFT_R");
        case OP_SHIFT_L:  return IR_CONSTZ("OP_SHIFT_L");
        case OP_CMP_AND:  return IR_CONSTZ("OP_CMP_AND");
        case OP_CMP_OR:  return IR_CONSTZ("OP_CMP_OR");
        case OP_CMP_EQ:  return IR_CONSTZ("OP_CMP_EQ");
        case OP_CMP_LT:  return IR_CONSTZ("OP_CMP_LT");
        case OP_CMP_GT:  return IR_CONSTZ("OP_CMP_GT");
        case OP_IADD:  return IR_CONSTZ("OP_IADD");
        case OP_ISUB:  return IR_CONSTZ("OP_ISUB");
        case OP_IMUL:  return IR_CONSTZ("OP_IMUL");
        case OP_IDIV:  return IR_CONSTZ("OP_IDIV");
        case OP_IMOD:  return IR_CONSTZ("OP_IMOD");
        case OP_IAND:  return IR_CONSTZ("OP_IAND");
        case OP_IOR:  return IR_CONSTZ("OP_IOR");
        case OP_ISHIFT_R:  return IR_CONSTZ("OP_ISHIFT_R");
        case OP_ISHIFT_L:  return IR_CONSTZ("OP_ISHIFT_L");
        case OP_LOAD64:  return IR_CONSTZ("OP_LOAD64");
        case OP_LOAD8:  return IR_CONSTZ("OP_LOAD8");
        case OP_STORE64:  return IR_CONSTZ("OP_STORE64");
        case OP_STORE8:  return IR_CONSTZ("OP_STORE8");
        case OP_ICMP_AND:  return IR_CONSTZ("OP_ICMP_AND");
        case OP_ICMP_OR:  return IR_CONSTZ("OP_ICMP_OR");
        case OP_ICMP_EQ:  return IR_CONSTZ("OP_ICMP_EQ");
        case OP_ICMP_LT:  return IR_CONSTZ("OP_ICMP_LT");
        case OP_ICMP_GT:  return IR_CONSTZ("OP_ICMP_GT");
        case OP_BEQ:  return IR_CONSTZ("OP_BEQ");
        case OP_BNE:  return IR_CONSTZ("OP_BNE");
        case OP_JMP:  return IR_CONSTZ("OP_JMP");
        default: return IR_CONSTZ("OPCODE PRINT NOT IMPLEMENTED");
    }
}

static
void print_instr(Instr instr, Metadata* meta)
{
    static FILE* dst = stdout;
    // R TYPE
    if(instr.R.opcode < OP_IADD)
    {
        fprintf(dst, "%.*s %u %u %u\n", 
                opcode_to_str((Opcodes)instr.R.opcode), 
                (u32)instr.R.dest,
                (u32)instr.R.op1,
                (u32)instr.R.op2);
    }
    //I TYPE
    else if(instr.R.opcode < OP_JMP)
    {
        fprintf(dst, "%.*s %u %u %u\n", 
                opcode_to_str((Opcodes)instr.I.opcode), 
                (u32)instr.I.dest,
                (u32)instr.I.op,
                (u32)instr.I.imm);
    }
    // J TYPE
    else
    {
        fprintf(dst, "%.*s %llu\n", 
                opcode_to_str((Opcodes)instr.J.opcode), 
                (u64)instr.J.jmp);
    }
    
    
    meta->cur_line++;
}


static void gen_node(Node* node, Metadata* meta);

static 
void gen_func(Node* node, Metadata* meta)
{
    Meta_Function mfunc = {};
    mfunc.func = node->func;
    mfunc.jmp_loc = meta->cur_line;
    
    meta->cur_func = ARR_PUSH(meta->funcs, mfunc);
    
    
    if(node->left)
    {
        gen_node(node->left, meta);
    }
    
    if(node->right)
    { 
        gen_node(node->right, meta);
    }
    
}

struct Expr_Result
{
    s64 value;
    b8 constant;
    b8 tmp;
};

static
Expr_Result gen_two_op(Opcodes opcode, Expr_Result left, Expr_Result right, Metadata* meta)
{
    
    Expr_Result result = {};
    
    Instr instr={};
    
    //TODO FIX LARGE CONSTANTS
    
    if(left.constant && !right.constant)
    {
        IR_ASSERT(left.value < (s64)(2147483647) && right.value > (s64)(-2147483647) && "TODO FIX LARGE CONSTANTS");
        instr.I.opcode = opcode + OP_IADD;
        instr.I.op = right.value;
        instr.I.imm = left.value;
        if(right.tmp)
        {
            instr.I.dest = right.value;
        }
        else
        {
            instr.I.dest = meta->treg_cnt;
            meta->treg_cnt++;
        }
        
        result.value = instr.I.dest;
    }
    else if(!left.constant && right.constant)
    {
        IR_ASSERT(right.value < (s64)(2147483647) && right.value > (s64)(-2147483647)  && "TODO FIX LARGE CONSTANTS");
        instr.I.opcode = opcode + OP_IADD;
        instr.I.op = left.value;
        instr.I.imm = right.value;
        if(left.tmp)
        {
            instr.I.dest = left.value;
        }
        else
        {
            instr.I.dest = meta->treg_cnt;
            meta->treg_cnt++;
        }
        
        
        result.value = instr.I.dest;
    }
    else
    {
        instr.R.opcode = opcode;
        instr.R.op1 = left.value;
        instr.R.op2 = right.value;
        instr.R.shift = 0;
        if(left.tmp)
        {
            instr.R.dest = left.value;
        }
        else if(right.tmp)
        {
            instr.R.dest = right.value;
        }
        else
        {
            instr.R.dest = meta->treg_cnt;
            meta->treg_cnt++;
        }
        
        result.value = instr.R.dest;
    }
    result.tmp = true;
    result.constant = false;
    
    print_instr(instr, meta);
    
    return result;
}

static 
Expr_Result gen_expr(Node* node, Metadata* meta)
{
    Expr_Result res_left = {};
    Expr_Result res_right = {};
    
    if(node->left)
        res_left = gen_expr(node->left, meta);
    if(node->right)
        res_right = gen_expr(node->right, meta);
    
    Expr_Result result = {};
    
    switch(node->type)
    {
        case N_EXPR:
        {
            result = res_left;
            break;
        }
        case N_NUM:
        {
            result.value = node->sValue;
            result.constant =true;
            result.tmp =false;
            break;
        }
        case N_VAR:
        {
            result.value = node->var->id;
            result.constant =false;
            result.tmp =false;
            break;
        }
        case N_ADD:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value + res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_ADD, res_left, res_right, meta);   
            }
            break;
        }
        case N_SUB:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value - res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_SUB, res_left, res_right, meta);   
            }
            break;
        }
        case N_MUL:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value * res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_MUL, res_left, res_right, meta);   
            }
            break;
        }
        case N_DIV:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value / res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_DIV, res_left, res_right, meta);   
            }
            break;
        }
        case N_MOD:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value % res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_MOD, res_left, res_right, meta);   
            }
            break;
        }
        case N_AND:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value && res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_AND, res_left, res_right, meta);   
            }
            break;
        }
        case N_OR:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value || res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_OR, res_left, res_right, meta);   
            }
            break;
        }
        case N_EQ:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value == res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_EQ, res_left, res_right, meta);   
            }
            break;
        }
        case N_NOT_EQ:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value == res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_EQ, res_left, res_right, meta);   
            }
            break;
        }
        
        default:
        {
            
            break;
        }
    }
    
    return result;
}

static
void gen_assign(Node* node, Metadata* meta)
{
    IR_ASSERT(node->left->type == N_VAR);// || node->left->type == N_DEREF);
    IR_ASSERT(node->right->type == N_EXPR);
    
    Expr_Result expr_result = gen_expr(node->right, meta);
    
    Instr instr = {};
    if(expr_result.constant)
    {
        instr.I.opcode = OP_IADD;
        instr.I.dest = node->left->var->id;
        instr.I.op = 0;
        instr.I.imm = expr_result.value;
        instr.I.shift = 0;
        
    }
    else
    {
        instr.R.opcode = OP_ADD;
        instr.R.dest = node->left->var->id;
        instr.R.op1 = expr_result.value;
        instr.R.op2 = 0;
        instr.R.shift = 0;
        
    }
    
    print_instr(instr, meta);
}

static
void gen_node(Node* node, Metadata* meta)
{
    
    switch(node->type)
    {
        case N_FUNC:
        {
            gen_func(node, meta);
            break;   
        }
        case N_ASSIGN:
        {
            gen_assign(node, meta);
            break;
        }
        case N_STMNT:
        case N_BLOCK:
        case N_EMPTY:
        default:
        {
            if(node->left)
            {
                gen_node(node->left, meta);
            }
            
            if(node->right)
            { 
                gen_node(node->right, meta);
            }
            break;   
        }
    }
    
    
}

static 
void generate(Node* ast, msi reg_max, Heap_Allocator* heap)
{
    Metadata meta = {};
    meta.treg_cnt = reg_max;
    ARR_INIT(meta.funcs, 64, heap);
    ARR_INIT(meta.var, 64, heap);
    
    if(ast)
    {
        gen_node(ast, &meta);
    }
    
}


#endif //GENERATOR_H
