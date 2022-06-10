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

enum Opcode
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
    
    //8bit Integer
    OP_8_ADD,
    OP_8_SUB,
    OP_8_MUL,
    OP_8_DIV,
    OP_8_MOD,
    OP_8_SHIFT_R,
    OP_8_SHIFT_L,
    
    //COMPARE
    OP_CMP_AND,
    OP_CMP_OR,
    OP_CMP_EQ,
    OP_CMP_NEQ,
    OP_CMP_LT,
    OP_CMP_GT,
    
    //FLOAT
    OP_F_ADD,
    OP_F_SUB,
    OP_F_MUL,
    OP_F_DIV,
    OP_F_CMP_LT,
    OP_F_CMP_GT,
    
    //I_TYPES
    //INTEGER
    OP_IADD = 64,
    OP_ISUB,
    OP_IMUL,
    OP_IDIV,
    OP_IMOD,
    OP_IAND,
    OP_IOR,
    OP_ISHIFT_R,
    OP_ISHIFT_L,
    
    
    //8bit Integer
    OP_8_IADD,
    OP_8_ISUB,
    OP_8_IMUL,
    OP_8_IDIV,
    OP_8_IMOD,
    OP_8_ISHIFT_R,
    OP_8_ISHIFT_L,
    
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
    
    //FLOAT
    OP_F_IADD,
    OP_F_ISUB,
    OP_F_IMUL,
    OP_F_IDIV,
    OP_F_ICMP_LT,
    OP_F_ICMP_GT,
    
    //Conversion
    OP_TO_F64,
    OP_F64_TO_S64,
    OP_F64_TO_C8,
    
    
    //LOAD/STORE
    OP_LOAD64,
    OP_LOAD8,
    OP_STORE64,
    OP_STORE8,
    
    
    
    //J_Types
    OP_JMP,
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
        union{
            s32 imm;
            f32 fImm;
        };
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
    //c8 on_stack;
};

struct Meta_Loop_Manip
{
    c8 is_break;
    msi loop_id;
    msi line;
};

struct Metadata
{
    Meta_Function* funcs;
    Meta_Variable* var;
    
    Instr main_jmp;
    Instr* init_instr_list;
    Instr* instr_list; 
    String* line_to_instr_list;
    
    
    Meta_Loop_Manip* loop_manips;
    
    Meta_Function* cur_func;
    msi cur_line;
    msi treg_cnt;
    msi cur_loop_id;
};

static
Opcode opcode_to_float_opcode(Opcode opcode){
    switch(opcode) {
        case OP_ADD:  return OP_F_ADD;
        case OP_SUB:  return OP_F_SUB;
        case OP_MUL:  return OP_F_MUL;
        case OP_DIV:  return OP_F_DIV;
        case OP_CMP_LT:  return OP_F_CMP_LT;
        case OP_CMP_GT:  return OP_F_CMP_GT;
        case OP_IADD:  return OP_F_IADD;
        case OP_ISUB:  return OP_F_ISUB;
        case OP_IMUL:  return OP_F_IMUL;
        case OP_IDIV:  return OP_F_IDIV;
        case OP_ICMP_LT:  return OP_F_ICMP_LT;
        case OP_ICMP_GT:  return OP_F_ICMP_GT;
        default: return opcode;
    }
}

static
Opcode opcode_to_8_opcode(Opcode opcode){
    switch(opcode) {
        case OP_ADD:  return OP_8_ADD;
        case OP_SUB:  return OP_8_SUB;
        case OP_MUL:  return OP_8_MUL;
        case OP_DIV:  return OP_8_DIV;
        case OP_MOD:  return OP_8_MOD;
        case OP_IADD:  return OP_8_IADD;
        case OP_ISUB:  return OP_8_ISUB;
        case OP_IMUL:  return OP_8_IMUL;
        case OP_IDIV:  return OP_8_IDIV;
        default: return opcode;
    }
}

static
bool is_float_op(Opcode opcode){
    if((((s64)opcode) >= OP_F_ADD && ((s64)opcode) <= OP_F_CMP_GT) || 
       (((s64)opcode) >= OP_F_IADD && ((s64)opcode) <= OP_F_ICMP_GT))
    {
        return true;
    }
    return false;
}

static
String opcode_to_str(Opcode opcode)
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
        case OP_F_ADD:  return IR_CONSTZ("OP_F_ADD");
        case OP_F_SUB:  return IR_CONSTZ("OP_F_SUB");
        case OP_F_MUL:  return IR_CONSTZ("OP_F_MUL");
        case OP_F_DIV:  return IR_CONSTZ("OP_F_DIV");
        case OP_F_CMP_LT:  return IR_CONSTZ("OP_F_CMP_LT");
        case OP_F_CMP_GT:  return IR_CONSTZ("OP_F_CMP_GT");
        case OP_IADD:  return IR_CONSTZ("OP_IADD");
        case OP_ISUB:  return IR_CONSTZ("OP_ISUB");
        case OP_IMUL:  return IR_CONSTZ("OP_IMUL");
        case OP_IDIV:  return IR_CONSTZ("OP_IDIV");
        case OP_IMOD:  return IR_CONSTZ("OP_IMOD");
        case OP_IAND:  return IR_CONSTZ("OP_IAND");
        case OP_IOR:  return IR_CONSTZ("OP_IOR");
        case OP_ISHIFT_R:  return IR_CONSTZ("OP_ISHIFT_R");
        case OP_ISHIFT_L:  return IR_CONSTZ("OP_ISHIFT_L");
        case OP_F_IADD:  return IR_CONSTZ("OP_F_IADD");
        case OP_F_ISUB:  return IR_CONSTZ("OP_F_ISUB");
        case OP_F_IMUL:  return IR_CONSTZ("OP_F_IMUL");
        case OP_F_IDIV:  return IR_CONSTZ("OP_F_IDIV");
        case OP_LOAD64:  return IR_CONSTZ("OP_LOAD64");
        case OP_LOAD8:  return IR_CONSTZ("OP_LOAD8");
        case OP_STORE64:  return IR_CONSTZ("OP_STORE64");
        case OP_STORE8:  return IR_CONSTZ("OP_STORE8");
        case OP_TO_F64:  return IR_CONSTZ("OP_TO_F64");
        case OP_F64_TO_S64:  return IR_CONSTZ("OP_F64_TO_S64");
        case OP_F64_TO_C8:  return IR_CONSTZ("OP_F64_TO_C8");
        case OP_ICMP_AND:  return IR_CONSTZ("OP_ICMP_AND");
        case OP_ICMP_OR:  return IR_CONSTZ("OP_ICMP_OR");
        case OP_ICMP_EQ:  return IR_CONSTZ("OP_ICMP_EQ");
        case OP_ICMP_NEQ:  return IR_CONSTZ("OP_ICMP_NEQ");
        case OP_ICMP_LT:  return IR_CONSTZ("OP_ICMP_LT");
        case OP_ICMP_GT:  return IR_CONSTZ("OP_ICMP_GT");
        case OP_BEQ:  return IR_CONSTZ("OP_BEQ");
        case OP_BNE:  return IR_CONSTZ("OP_BNE");
        case OP_F_ICMP_LT:  return IR_CONSTZ("OP_F_ICMP_LT");
        case OP_F_ICMP_GT:  return IR_CONSTZ("OP_F_ICMP_GT");
        case OP_JMP:  return IR_CONSTZ("OP_JMP");
        case OP_JMPR:  return IR_CONSTZ("OP_JMPR");
        case OP_8_ADD:  return IR_CONSTZ("OP_8_ADD");
        case OP_8_SUB:  return IR_CONSTZ("OP_8_SUB");
        case OP_8_MUL:  return IR_CONSTZ("OP_8_MUL");
        case OP_8_DIV:  return IR_CONSTZ("OP_8_DIV");
        case OP_8_MOD:  return IR_CONSTZ("OP_8_MOD"); 
        case OP_8_IADD:  return IR_CONSTZ("OP_8_IADD");
        case OP_8_ISUB:  return IR_CONSTZ("OP_8_ISUB");
        case OP_8_IMUL:  return IR_CONSTZ("OP_8_IMUL");
        case OP_8_IDIV:  return IR_CONSTZ("OP_8_IDIV");
        case OP_8_IMOD:  return IR_CONSTZ("OP_8_IMOD");
        case OP_8_ISHIFT_R:  return IR_CONSTZ("OP_8_ISHIFT_R");
        case OP_8_ISHIFT_L:  return IR_CONSTZ("OP_8_ISHIFT_L");
        default: return IR_CONSTZ("OPCODE PRINT NOT IMPLEMENTED");
    }
}

static bool is_commutative(Opcode opcode){
    
    switch(opcode)
    {
        case OP_ADD:
        case OP_MUL:
        case OP_AND:
        case OP_OR:
        case OP_8_ADD:
        case OP_8_MUL:
        case OP_F_ADD:
        case OP_F_MUL:
        case OP_CMP_AND:
        case OP_CMP_OR:
        case OP_CMP_EQ:
        case OP_CMP_NEQ:
        case OP_IADD:
        case OP_IMUL:
        case OP_8_IADD:
        case OP_8_IMUL:
        case OP_F_IADD:
        case OP_F_IMUL:
        case OP_IAND:
        case OP_IOR:
        case OP_ICMP_AND:
        case OP_ICMP_OR:
        case OP_ICMP_EQ:
        case OP_ICMP_NEQ:
        return true;
        default: return false;
    }
}

static 
void print_instr(Instr instr, msi line_num, FILE* dst)
{
    
    // R TYPE
    if(instr.R.opcode < OP_IADD)
    {
        fprintf(dst, "%llu:  %.*s %u %u %u\n", 
                line_num,
                opcode_to_str((Opcode)instr.R.opcode),
                (u32)instr.R.dest,
                (u32)instr.R.op1,
                (u32)instr.R.op2);
    }
    //I TYPE
    else if(instr.R.opcode < OP_JMP)
    {
        if (is_float_op((Opcode)instr.R.opcode)){
            fprintf(dst, "%llu:  %.*s %u %u %f\n",
                    line_num,
                    opcode_to_str((Opcode)instr.I.opcode),
                    (u32)instr.I.dest,
                    (u32)instr.I.op,
                    (f32)instr.I.fImm);
        }
        else{
            fprintf(dst, "%llu:  %.*s %u %u %i\n",
                    line_num,
                    opcode_to_str((Opcode)instr.I.opcode),
                    (u32)instr.I.dest,
                    (u32)instr.I.op,
                    (s32)instr.I.imm);
        }
    }
    // J TYPE
    else
    {
        fprintf(dst, "%llu:  %.*s %llu\n", 
                line_num,
                opcode_to_str((Opcode)instr.J.opcode),
                (u64)instr.J.jmp);
    }   
}

static
void print_all_instr(Metadata* meta)
{
    static FILE* dst = stdout;
    
    String prev_line = {};// meta->line_to_instr_list[0];
    msi line_num = 0;
    
    meta->instr_list[0].J.jmp = ARR_LEN(meta->instr_list);
    
    
    for(msi i = 0; i < ARR_LEN(meta->instr_list); ++i)
    {
        Instr instr = meta->instr_list[i];
        
        if(prev_line.data != meta->line_to_instr_list[i].data)
        { 
            prev_line = meta->line_to_instr_list[i];
            fprintf(dst, "\033[32m%.*s\033[97m", prev_line);
        }
        
        print_instr(instr, line_num, dst);
        line_num++;
        
    }
    
    for(msi i = 0; i < ARR_LEN(meta->init_instr_list); ++i)
    {   
        print_instr(meta->init_instr_list[i], line_num, dst);
        
        line_num++;
    }
    
    
    print_instr(meta->main_jmp, line_num, dst);
    
}

static
void generate_binary(c8* binary_loc, Metadata* meta)
{   
    FILE* bin_file;
    bin_file = fopen(binary_loc, "wb");
    
    meta->instr_list[0].J.jmp = ARR_LEN(meta->instr_list);
    
    msi result = fwrite(&meta->instr_list[0], sizeof(Instr) * ARR_LEN(meta->instr_list), 1, bin_file);
    
    if(!result)
    {
        fprintf(stderr, "Error writing instr binary file!\n");
    }
    
    if(ARR_LEN(meta->init_instr_list) > 0)
    {
        result = fwrite(&meta->init_instr_list[0], sizeof(Instr) * ARR_LEN(meta->init_instr_list), 1, bin_file);
        
        if(!result)
        {
            fprintf(stderr, "Error writing init func in binary file!\n");
        }
    }
    
    
    result = fwrite(&meta->main_jmp, sizeof(Instr), 1, bin_file);
    
    if(!result)
    {
        fprintf(stderr, "Error writing main jmp in binary file!\n");
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
    
    
    if(cmp_string(mfunc.func->name, IR_CONSTZ("main")))
    {
        meta->main_jmp = {};
        meta->main_jmp.J.opcode = OP_JMP;
        meta->main_jmp.J.jmp = mfunc.jmp_loc;
    }
    
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
    union {
        s64 value;
        f64 fValue;
        c8 cValue;
    };
    DataType dataType;
    c8 constant;
    c8 tmp;
};

#define get_value(var) (((var.dataType)==F64)?var.fValue:((var.dataType)==S64)?var.value:var.cValue)
//no matter datatype everything is cast to f64 due to ternary op; is this an issue?
#define get_int_value(var) (((var.dataType)==S64)?var.value:var.cValue)

static
void assign_expr_value(Expr_Result &res, s64 value){
    if (res.dataType == S64)
        res.value = value;
    else
        res.cValue = (c8)value;
}

static
void assign_expr_value(Expr_Result &res, f64 value){
    if (res.dataType == F64)
        res.fValue = value;
    else if (res.dataType == S64)
        res.value = (s64)value;
    else
        res.cValue = (c8)value;
}

static
Expr_Result gen_two_op(Opcode opcode, Expr_Result left, Expr_Result right, String cur_line, Metadata* meta)
{
    
    Expr_Result result = {};
    
    Instr instr={};
    
    b8 floatOp = left.dataType == F64;
    b8 isC8op = left.dataType == C8 && right.dataType == C8;
    
    //TODO FIX LARGE CONSTANTS
    
    if(left.constant && !right.constant)
    {
        IR_ASSERT(left.value < (s64)(2147483647) && right.value > (s64)(-2147483647) && "TODO FIX LARGE CONSTANTS");
        if (is_commutative(opcode)){
            if (floatOp)
            {
                instr.I.opcode = opcode_to_float_opcode((Opcode)(opcode + OP_IADD));
                instr.I.fImm = left.fValue;
            }
            else if(isC8op)
            {
                instr.I.opcode = opcode_to_8_opcode((Opcode)(opcode + OP_IADD));
                instr.I.imm = left.value;
            }
            else
            {
                instr.I.opcode = opcode + OP_IADD;
                instr.I.imm = left.value;
            }
            instr.I.op = right.value;
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
        else{
            if (floatOp)
            {
                instr.I.opcode = opcode_to_float_opcode(OP_IADD);
                instr.I.fImm = left.fValue;
            }
            else if(isC8op)
            {
                instr.I.opcode = opcode_to_8_opcode(OP_IADD);
                instr.I.imm = left.value;
            }
            else
            {
                instr.I.opcode = OP_IADD;
                instr.I.imm = left.value;
            }
            instr.I.dest = meta->treg_cnt;
            meta->treg_cnt++;
            instr.I.op = R_ZERO;
            instr.I.shift = 0;
            add_instr(instr, cur_line, meta);
            
            u8 tempReg = instr.I.dest;
            
            if (floatOp)
            {
                instr.R.opcode = opcode_to_float_opcode(opcode);
            }
            else if(isC8op)
            {
                instr.R.opcode = opcode_to_8_opcode(opcode);
            }
            else
            {
                instr.R.opcode = opcode;
            }
            instr.R.op1 = tempReg;
            instr.R.op2 = right.value;
            instr.R.shift = 0;
            instr.R.dest = tempReg;
            
            result.value = instr.R.dest;
        }
    }
    else if(!left.constant && right.constant)
    {
        IR_ASSERT(right.value < (s64)(2147483647) && right.value > (s64)(-2147483647)  && "TODO FIX LARGE CONSTANTS");
        if (floatOp)
        {
            instr.I.opcode = opcode_to_float_opcode((Opcode)(opcode + OP_IADD));
            instr.I.fImm = right.fValue;
        }
        else if(isC8op)
        {
            instr.I.opcode = opcode_to_8_opcode((Opcode)(opcode + OP_IADD));
            instr.I.imm = right.value;
        }
        else
        {
            instr.I.opcode = opcode + OP_IADD;
            instr.I.imm = right.value;
        }
        instr.I.op = left.value;
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
        if (floatOp)
        {
            instr.R.opcode = opcode_to_float_opcode(opcode);
        }
        else if(isC8op)
        {
            instr.R.opcode = opcode_to_8_opcode(opcode);
        }
        else
        {
            instr.R.opcode = opcode;
        }
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
    result.dataType = node->dataType;
    
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
        case N_FLOAT:
        {
            result.fValue = node->fValue;
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
        case N_TO_S64:
        {
            if (res_left.constant){
                result.value = get_value(res_left);
                result.constant = true;
                result.tmp = false;
            }else{
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if(res_left.tmp)
                {
                    instr.I.opcode = OP_F64_TO_S64;
                    instr.I.dest = res_left.value;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                    //result.dataType = C8; //already set through node
                }
                else
                {
                    instr.I.opcode = OP_F64_TO_S64;
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
        case N_TO_C8:
        {
            if (res_left.constant){
                result.value = (s8)get_value(res_left);
                result.constant = true;
                result.tmp = false;
            }else{
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if(res_left.tmp)
                {
                    instr.I.opcode = OP_F64_TO_C8;
                    instr.I.dest = res_left.value;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                    //result.dataType = C8; //already set through node
                }
                else
                {
                    instr.I.opcode = OP_F64_TO_C8;
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
        case N_TO_F64:
        {
            if (res_left.constant){
                result.fValue = get_value(res_left);
                result.constant = true;
                result.tmp = false;
            }else{
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if(res_left.tmp)
                {
                    instr.I.opcode = OP_TO_F64;
                    instr.I.dest = res_left.value;
                    instr.I.imm = 0;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                    //result.dataType = C8; //already set through node
                }
                else
                {
                    instr.I.opcode = OP_TO_F64;
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
        case N_ADD:
        {
            if(res_left.constant && res_right.constant)
            {
                assign_expr_value(result, get_value(res_left) + get_value(res_right));
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
                assign_expr_value(result, get_value(res_left) - get_value(res_right));
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
                assign_expr_value(result, get_value(res_left) * get_value(res_right));
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
                assign_expr_value(result, get_value(res_left) / get_value(res_right));
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
                assign_expr_value(result, get_int_value(res_left) % get_int_value(res_right));
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
                assign_expr_value(result, get_int_value(res_left) & get_int_value(res_right));
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
                assign_expr_value(result, get_int_value(res_left) | get_int_value(res_right));
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
                assign_expr_value(result, (s64) (get_value(res_left) == get_value(res_right)));
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
                assign_expr_value(result, (s64) (get_value(res_left) != get_value(res_right)));
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
                assign_expr_value(result, (s64) (get_value(res_left) && get_value(res_right)));
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
                assign_expr_value(result, (s64) (get_value(res_left) || get_value(res_right)));
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
                assign_expr_value(result, (s64) (get_value(res_left) < get_value(res_right)));
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
                assign_expr_value(result, (s64) (get_value(res_left) > get_value(res_right)));
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
                assign_expr_value(result, (s64) (get_value(res_left) <= get_value(res_right)));
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_GT, res_left, res_right, node->line_text, meta); 
                Instr instr={};
                instr.I.imm = 0; //same as float 0
                instr.I.dest = result.value;
                instr.I.op = result.value;
                add_instr(instr, node->line_text, meta);
            }
            break;
        }
        case N_CMP_GEQ:
        {
            if(res_left.constant && res_right.constant)
            {
                assign_expr_value(result, (s64) (get_value(res_left) >= get_value(res_right)));
                result.constant = true;
            }
            else
            {
                result = gen_two_op(OP_CMP_LT, res_left, res_right, node->line_text, meta); 
                Instr instr={};
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
                assign_expr_value(result, -get_value(res_left));
                result.constant = true;   
                result.tmp = false;
            }
            else
            {
                result.tmp = true;
                result.constant = false;
                
                Instr instr={};
                
                if (node->dataType == F64){
                    instr.I.opcode = OP_F_IMUL;
                    instr.I.fImm = -1;
                }else{
                    instr.I.opcode = OP_IMUL;
                    instr.I.imm = -1;
                }
                
                if(res_left.tmp)
                {
                    instr.I.dest = res_left.value;
                    instr.I.op = res_left.value;
                    result.value = res_left.value;
                }
                else
                {
                    instr.I.dest =  meta->treg_cnt;;
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
                assign_expr_value(result, (s64) (!get_value(res_left)));
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
                    //same for floats
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
    result.dataType = node->dataType;
    
    return result;
}

static
void gen_assign(Node* node, Metadata* meta)
{
    IR_ASSERT(node->left->type == N_VAR);// || node->left->type == N_DEREF);
    IR_ASSERT(node->right->type == N_EXPR);
    
    Expr_Result expr_result = gen_expr(node->right, meta);
    
    b8 isFloat = expr_result.dataType == F64;
    b8 isC8 = node->left->dataType == C8;
    
    Instr instr = {};
    if(expr_result.constant)
    {
        if (isFloat)
        {
            instr.I.opcode = OP_F_IADD;
            instr.I.fImm = expr_result.fValue;
        }
        else if(isC8)
        {
            instr.I.opcode = OP_8_IADD;
            instr.I.imm = expr_result.value;   
        }
        else
        {
            instr.I.opcode = OP_IADD;
            instr.I.imm = expr_result.value;
        }
        instr.I.dest = node->left->var->id;
        instr.I.op = R_ZERO;
        instr.I.shift = 0;
        
        add_instr(instr, node->line_text, meta);
    }
    else if(!expr_result.tmp){
        if (isFloat)
        {
            instr.I.opcode = OP_F_IADD;
        }
        else if(isC8)
        {
            instr.I.opcode = OP_8_IADD; 
        }
        else
        {
            instr.I.opcode = OP_IADD;
        }
        instr.I.dest = node->left->var->id;
        instr.I.op = expr_result.value;
        instr.I.imm = R_ZERO;
        instr.I.shift = 0;
        
        add_instr(instr, node->line_text, meta);
    }
    else
    {
        //if there is already a tmp register we can change the output to our dest
        if(isC8 && expr_result.dataType != C8)
        {
            instr.I.opcode = OP_8_IADD;
            instr.I.dest = node->left->var->id;
            instr.I.op = expr_result.value;
            instr.I.imm = R_ZERO;
            instr.I.shift = 0;
            
            add_instr(instr, node->line_text, meta);
        }
        else
        {
            ARR_LAST(meta->instr_list)->R.dest = node->left->var->id;    
        }
    }
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
        if (node->dataType==F64)
        {
            instr.I.opcode = OP_F_IADD;
            instr.I.fImm = expr_res.fValue;
        }
        else if(node->dataType==C8)
        {
            instr.I.opcode = OP_8_IADD;
            instr.I.imm = get_value(expr_res);
        }
        else
        {
            instr.I.opcode = OP_IADD;
            instr.I.imm = get_value(expr_res);
        }
        instr.I.dest = R_RETURN;
        instr.I.op = R_ZERO;
        instr.I.shift = 0;
        
    }
    else
    {
        if(node->dataType==C8)
        {
            instr.R.opcode = OP_8_ADD;
        }
        else
        {
            instr.R.opcode = OP_ADD;
        }
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
    meta.main_jmp = {};
    ARR_INIT(meta.funcs, 64, heap);
    ARR_INIT(meta.var, 64, heap);
    ARR_INIT(meta.init_instr_list, 32, heap);
    ARR_INIT(meta.instr_list, 128, heap);
    ARR_INIT(meta.line_to_instr_list, 128, heap);
    ARR_INIT(meta.loop_manips, 16, heap);
    
    Instr init_jmp = {};
    init_jmp.J.opcode = OP_JMP;
    add_instr(init_jmp, IR_CONSTZ("INIT_JMP\n"), &meta);
    
    if(ast)
    {
        gen_node(ast, &meta);
    }
    
    print_all_instr(&meta);
    
    return meta;
}


#endif //GENERATOR_H
