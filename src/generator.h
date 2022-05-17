#ifndef GENERATOR_H
#define GENERATOR_H

#include "tokens.h"
#include "tokenizer.h"
#include "nodes.h"

#pragma pack(push, 1)




/*
First 10 registers are reserved:

0 .. zero reg
1 .. program counter
2 .. return addr
3 .. return reg
4 .. frame_ptr
5 .. stack_ptr
*/

enum Registers
{
    R_ZERO=0,
    R_PROG_CNT,
    R_RETURN_ADDR,
    R_RETURN,
    R_FRAME_PTR,
    R_STACK_PTR,  
};

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
    OP_CMP_NEQ,
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
    OP_ICMP_NEQ,
    OP_ICMP_LT,
    OP_ICMP_GT,
    
    
    //Branch
    OP_BEQ,
    OP_BNE,
    
    
    //LOAD/STORE
    OP_LOAD64,
    OP_LOAD8,
    OP_STORE64,
    OP_STORE8,
    
    
    
    //J_Types
    OP_JMP = 64,
    OP_JMPR,
    
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

struct Meta_Loop_Manip
{
    b8 is_break;
    msi loop_id;
    msi line;
};

struct Metadata
{
    Meta_Function* funcs;
    Meta_Variable* var;
    
    Instr* instr_list; 
    String* line_to_instr_list;
    
    
    
    Meta_Loop_Manip* loop_manips;
    
    Meta_Function* cur_func;
    msi cur_line;
    msi treg_cnt;
    msi cur_loop_id;
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
        case OP_CMP_NEQ:  return IR_CONSTZ("OP_CMP_NEQ");
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
        case OP_ICMP_NEQ:  return IR_CONSTZ("OP_ICMP_NEQ");
        case OP_ICMP_LT:  return IR_CONSTZ("OP_ICMP_LT");
        case OP_ICMP_GT:  return IR_CONSTZ("OP_ICMP_GT");
        case OP_BEQ:  return IR_CONSTZ("OP_BEQ");
        case OP_BNE:  return IR_CONSTZ("OP_BNE");
        case OP_JMP:  return IR_CONSTZ("OP_JMP");
        case OP_JMPR:  return IR_CONSTZ("OP_JMPR");
        default: return IR_CONSTZ("OPCODE PRINT NOT IMPLEMENTED");
    }
}

static
void print_all_instr(Metadata* meta)
{
    static FILE* dst = stdout;
    
    String prev_line = {};// meta->line_to_instr_list[0];
    
    
    for(msi i = 0; i < ARR_LEN(meta->instr_list); ++i)
    {
        Instr instr = meta->instr_list[i];
        
        if(prev_line.data != meta->line_to_instr_list[i].data)
        { 
            prev_line = meta->line_to_instr_list[i];
            fprintf(dst, "\033[32m%.*s\033[97m", prev_line);
        }
        
        // R TYPE
        if(instr.R.opcode < OP_IADD)
        {
            fprintf(dst, "%llu:  %.*s %u %u %u\n", 
                    i,
                    opcode_to_str((Opcodes)instr.R.opcode), 
                    (u32)instr.R.dest,
                    (u32)instr.R.op1,
                    (u32)instr.R.op2);
        }
        //I TYPE
        else if(instr.R.opcode < OP_JMP)
        {
            fprintf(dst, "%llu:  %.*s %u %u %i\n",
                    i,
                    opcode_to_str((Opcodes)instr.I.opcode), 
                    (u32)instr.I.dest,
                    (u32)instr.I.op,
                    (s32)instr.I.imm);
        }
        // J TYPE
        else
        {
            fprintf(dst, "%llu:  %.*s %llu\n", 
                    i,
                    opcode_to_str((Opcodes)instr.J.opcode), 
                    (u64)instr.J.jmp);
        } 
    }
}

static
void generate_binary(c8* binary_loc, Metadata* meta)
{
    FILE* bin_file;
    bin_file = fopen(binary_loc, "wb");
    msi result = fwrite(&meta->instr_list[0], sizeof(Instr) * ARR_LEN(meta->instr_list), 1, bin_file);
    
    if(!result)
    {
        fprintf(stderr, "Error writing binary file!\n");
    }
    
    fclose(bin_file);
}

static
void add_instr(Instr instr, String cur_line, Metadata* meta)
{
    
    ARR_PUSH(meta->line_to_instr_list, cur_line);
    ARR_PUSH(meta->instr_list, instr);
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
Expr_Result gen_two_op(Opcodes opcode, Expr_Result left, Expr_Result right, String cur_line, Metadata* meta)
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
    
    add_instr(instr, cur_line, meta);
    
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
                result = gen_two_op(OP_ADD, res_left, res_right, node->line_text, meta);   
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
                result = gen_two_op(OP_SUB, res_left, res_right, node->line_text, meta);   
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
                result = gen_two_op(OP_MUL, res_left, res_right, node->line_text, meta);   
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
                result = gen_two_op(OP_DIV, res_left, res_right, node->line_text, meta);   
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
                result = gen_two_op(OP_MOD, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_AND:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value & res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_AND, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_OR:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value | res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_OR, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_EQ:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value == res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_EQ, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_NEQ:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value != res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_NEQ, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_AND:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value && res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_AND, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_OR:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value || res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_OR, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_LT:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value < res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_LT, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_GT:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value > res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_GT, res_left, res_right, node->line_text, meta);   
            }
            break;
        }
        case N_CMP_LEQ:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value <= res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_GT, res_left, res_right, node->line_text, meta); 
                Instr instr={};
                instr.I.opcode = OP_ICMP_NEQ;
                instr.I.dest = result.value;
                instr.I.imm = 0;
                instr.I.op = result.value;
                add_instr(instr, node->line_text, meta);
            }
            break;
        }
        case N_CMP_GEQ:
        {
            if(res_left.constant && res_right.constant)
            {
                result.value = res_left.value >= res_right.value;
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_LT, res_left, res_right, node->line_text, meta); 
                Instr instr={};
                instr.I.opcode = OP_ICMP_NEQ;
                instr.I.dest = result.value;
                instr.I.imm = 0;
                instr.I.op = result.value;
                add_instr(instr, node->line_text, meta);
            }
            break;
        }
        case N_NEG:
        {
            if(res_left.constant)
            {
                result.value = -res_left.value;
                result.constant = true;   
                result.tmp = false;
            }
            else
            {
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if(res_left.tmp)
                {
                    
                    instr.I.opcode = OP_IMUL;
                    instr.I.dest = res_left.value;
                    instr.I.imm = -1;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                }
                else
                {
                    instr.I.opcode = OP_IMUL;
                    instr.I.dest =  meta->treg_cnt;;
                    instr.I.imm = -1;
                    instr.I.op = res_left.value;
                    
                    result.value = meta->treg_cnt;
                    meta->treg_cnt++; 
                } 
                add_instr(instr, node->line_text, meta);
            }
            break;
        }  
        case N_DEREF:
        {
            if(res_left.constant)
            {
                IR_ASSERT(false && "Cannot dereference a constant!\n")
            }
            else
            {
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if(res_left.tmp)
                {
                    
                    instr.I.opcode = OP_LOAD64;
                    instr.I.dest = res_left.value;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                }
                else
                {
                    instr.I.opcode = OP_LOAD64;
                    instr.I.dest =  meta->treg_cnt;;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    
                    result.value = meta->treg_cnt;
                    meta->treg_cnt++; 
                }  
                
                add_instr(instr, node->line_text, meta);
            }
            
            break;
        }
        case N_NOT:
        {
            if(res_left.constant)
            {
                result.value = !res_left.value;
                result.constant = true;   
                result.tmp = false;
            }
            else
            {
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if(res_left.tmp)
                {
                    
                    instr.I.opcode = OP_ICMP_EQ;
                    instr.I.dest = res_left.value;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                }
                else
                {
                    instr.I.opcode = OP_ICMP_EQ;
                    instr.I.dest =  meta->treg_cnt;;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    
                    result.value = meta->treg_cnt;
                    meta->treg_cnt++; 
                }  
                
                add_instr(instr, node->line_text, meta);
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
        instr.I.op = R_ZERO;
        instr.I.imm = expr_result.value;
        instr.I.shift = 0;
        
    }
    else
    {
        instr.R.opcode = OP_ADD;
        instr.R.dest = node->left->var->id;
        instr.R.op1 = expr_result.value;
        instr.R.op2 = R_ZERO;
        instr.R.shift = 0;
        
    }
    
    add_instr(instr, node->line_text, meta);
}

static 
void gen_if(Node* node, Metadata* meta)
{
    IR_ASSERT(node->left->type == N_EXPR);
    
    Expr_Result expr_res = gen_expr(node->left, meta);
    
    if(expr_res.constant)
    {
        if(expr_res.value)
        {
            if(node->right->type == N_ELSE)
            {
                gen_node(node->right->left, meta);   
            }
            else
            {
                gen_node(node->right, meta); 
            }
        }
        else
        {
            if(node->right->type == N_ELSE)
            {
                gen_node(node->right->right, meta);   
            }
            else
            {
                //gen_node(node->right, meta); 
            } 
        }
    }
    else
    {
        msi branch_index = meta->cur_line;
        
        Instr b_instr = {};
        b_instr.I.opcode = OP_BEQ;
        b_instr.I.dest = R_ZERO;
        b_instr.I.op = expr_res.value;  
        b_instr.I.imm = 0;
        b_instr.I.shift = 0;
        
        add_instr(b_instr, node->line_text, meta);
        
        if(node->right->type == N_ELSE)
        {
            gen_node(node->right->left, meta);
            
            meta->instr_list[branch_index].I.imm = meta->cur_line - branch_index -1;
            
            gen_node(node->right->right, meta);
        }
        else
        {
            gen_node(node->right, meta);
            
            meta->instr_list[branch_index].I.imm = meta->cur_line - branch_index -1;
        }
        
    }
}

static
void gen_return(Node* node, Metadata* meta)
{
    IR_ASSERT(node->left->type == N_EXPR);
    Expr_Result expr_res = gen_expr(node->left, meta);
    
    Instr instr = {};
    if(expr_res.constant)
    {
        instr.I.opcode = OP_IADD;
        instr.I.dest = R_RETURN;
        instr.I.op = R_ZERO;
        instr.I.imm = expr_res.value;
        instr.I.shift = 0;
        
    }
    else
    {
        instr.R.opcode = OP_ADD;
        instr.R.dest = R_RETURN;
        instr.R.op1 = expr_res.value;
        instr.R.op2 = R_ZERO;
        instr.R.shift = 0;
        
    }
    add_instr(instr, node->line_text, meta);
    
    instr = {};
    instr.J.opcode = OP_JMPR;
    instr.J.jmp = R_RETURN_ADDR;
    
    add_instr(instr, node->line_text, meta);
    
    
}

static
void gen_for(Node* node, Metadata* meta)
{
    IR_ASSERT(node->left->type == N_FOR && node->left->left->type == N_FOR);
    
    
    meta->cur_loop_id++;
    
    gen_node(node->left->left->left, meta);
    
    msi for_start_index = meta->cur_line;
    msi branch_index = (msi)-1;
    
    if(node->left->left->right->type == N_EXPR)
    {
        Expr_Result expr_res = gen_expr(node->left->left->right, meta);
        
        if(expr_res.constant)
        {
            if(!expr_res.value)
            {
                
                meta->cur_loop_id--;
                return;   
            }
        }
        else
        {
            branch_index = meta->cur_line;
            
            Instr b_instr = {};
            b_instr.I.opcode = OP_BEQ;
            b_instr.I.dest = R_ZERO;
            b_instr.I.op = expr_res.value;  
            b_instr.I.imm = 0;
            b_instr.I.shift = 0;  
            
            add_instr(b_instr, node->line_text, meta);
        }
        
    }
    else
    {
        gen_node(node->left->left->right, meta);
    }
    
    
    gen_node(node->right, meta);
    
    msi break_jmp_loc = meta->cur_line;
    
    gen_node(node->left->right, meta);
    
    Instr instr = {};
    instr = {};
    instr.J.opcode = OP_JMP;
    instr.J.jmp = for_start_index;
    
    
    add_instr(instr, node->line_text, meta);
    
    
    if(branch_index != (msi)-1)
    {
        meta->instr_list[branch_index].I.imm = meta->cur_line - branch_index -1;
    }
    
    
    for(msi i = ARR_LEN(meta->loop_manips) -1; i >= 0 ; --i)
    {
        Meta_Loop_Manip manip = meta->loop_manips[i];
        
        if(manip.loop_id != meta->cur_loop_id)
        {
            break;   
        }
        
        if(manip.is_break)
        {
            meta->instr_list[manip.line].J.jmp = meta->cur_line;   
        }
        else
        {
            meta->instr_list[manip.line].J.jmp = break_jmp_loc;
        }
        
        ARR_POP(meta->loop_manips);
    }
    
    meta->cur_loop_id--;
}

static
void gen_while(Node* node, Metadata* meta)
{
    IR_ASSERT(node->left->type == N_EXPR);
    
    meta->cur_loop_id++;
    
    msi while_start_index = meta->cur_line;
    msi branch_index = (msi)-1;
    
    Expr_Result expr_res = gen_expr(node->left, meta);
    
    if(expr_res.constant)
    {
        if(!expr_res.value)
        {
            
            meta->cur_loop_id--;
            return;   
        }
    }
    else
    {
        branch_index = meta->cur_line;
        
        Instr b_instr = {};
        b_instr.I.opcode = OP_BEQ;
        b_instr.I.dest = R_ZERO;
        b_instr.I.op = expr_res.value;  
        b_instr.I.imm = 0;
        b_instr.I.shift = 0;  
        
        add_instr(b_instr, node->line_text, meta);
    }
    
    gen_node(node->right, meta);
    
    Instr instr = {};
    instr = {};
    instr.J.opcode = OP_JMP;
    instr.J.jmp = while_start_index;
    
    
    add_instr(instr, node->line_text, meta);
    
    
    if(branch_index != (msi)-1)
    {
        meta->instr_list[branch_index].I.imm = meta->cur_line - branch_index -1;
    }
    
    
    for(msi i = ARR_LEN(meta->loop_manips) -1; i >= 0 ; --i)
    {
        Meta_Loop_Manip manip = meta->loop_manips[i];
        
        if(manip.loop_id != meta->cur_loop_id)
        {
            break;   
        }
        
        if(manip.is_break)
        {
            meta->instr_list[manip.line].J.jmp = meta->cur_line;   
        }
        else
        {
            meta->instr_list[manip.line].J.jmp = while_start_index;
        }
        
        ARR_POP(meta->loop_manips);
    }
    
    meta->cur_loop_id--;
    
}

void gen_break(Node* node, Metadata* meta)
{
    Instr instr = {};
    instr = {};
    instr.J.opcode = OP_JMP;
    instr.J.jmp = 0;
    
    Meta_Loop_Manip loop_manip = {};
    loop_manip.is_break = true;
    loop_manip.loop_id = meta->cur_loop_id;
    loop_manip.line = meta->cur_line;
    
    ARR_PUSH(meta->loop_manips, loop_manip);
    
    add_instr(instr, node->line_text, meta);
}

void gen_continue(Node* node, Metadata* meta)
{
    Instr instr = {};
    instr = {};
    instr.J.opcode = OP_JMP;
    instr.J.jmp = 0;
    
    Meta_Loop_Manip loop_manip = {};
    loop_manip.is_break = false;
    loop_manip.loop_id = meta->cur_loop_id;
    loop_manip.line = meta->cur_line;
    
    ARR_PUSH(meta->loop_manips, loop_manip);
    
    add_instr(instr, node->line_text, meta);
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
        case N_IF:
        {
            gen_if(node, meta);
            break;   
        }
        case N_RETURN:
        {
            gen_return(node, meta);
            break;   
        }
        case N_FOR:
        {
            gen_for(node, meta);
            break;   
        }
        case N_WHILE:
        {
            gen_while(node, meta);
            break;   
        }
        case N_BREAK:
        {
            gen_break(node, meta);
            break;   
        }
        case N_CONTINUE:
        {
            gen_continue(node, meta);
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
Metadata generate(Node* ast, msi reg_max, Heap_Allocator* heap)
{
    Metadata meta = {};
    meta.treg_cnt = reg_max;
    ARR_INIT(meta.funcs, 64, heap);
    ARR_INIT(meta.var, 64, heap);
    ARR_INIT(meta.instr_list, 128, heap);
    ARR_INIT(meta.line_to_instr_list, 128, heap);
    ARR_INIT(meta.loop_manips, 16, heap);
    
    if(ast)
    {
        gen_node(ast, &meta);
    }
    
    print_all_instr(&meta);
    
    return meta;
}


#endif //GENERATOR_H
