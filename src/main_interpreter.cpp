#include "generator.h"

Instr* read_entire_file(c8* file_name, Heap_Allocator* heap)
{
    FILE* file;
    file = fopen(file_name, "rb+");
    
    u64 file_size = 0;
    
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    Instr* result = nullptr;
    
    msi num_instr = file_size/sizeof(Instr);
    
    ARR_INIT(result, num_instr, heap);
    
    fread(result, file_size, 1, file);
    
    arr_header(result)->length = file_size/sizeof(Instr);
    
    fclose(file);
    
    return result;
}

struct Machine 
{
    union{
        s64** r;
        f64** fR;
    };
};

s64*
R(Machine m, u32 reg)
{
    if(reg < R_FIRST_LOCAL)
    {
        if(ARR_LEN(m.r[0]) <= reg)
        {
            ARR_INS(m.r[0], reg, (u64)0);
        }
        return &m.r[0][reg];
    }
    else
    {
        reg -= R_FIRST_LOCAL;
        u64 context = m.r[0][R_CONTEXT];
        
        if(ARR_LEN(m.r) <= context)
        {
            ARR_INS(m.r, context, nullptr);
            ARR_INIT(m.r[context], 16, arr_header(m.r)->heap);
        }
        
        if(ARR_LEN(m.r[context]) <= reg)
        {
            ARR_INS(m.r[context], reg, (u64)0);
        }
        return &m.r[context][reg]; 
    }
}

f64*
RF(Machine m, u32 reg)
{ 
    return (f64*)((void*)R(m, reg));
}

int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(512), (u8*)malloc(IR_MEGABYTES(512)));
    
    Heap_Allocator heap = create_heap(&arena, IR_MEGABYTES(128), 0);
    
    Instr* instr_list;
    
    if (argc > 1){
        instr_list = read_entire_file(argv[1], &heap);
    }else
    {
        fprintf(stdout, "USAGE: ./mayfly <path to binary>\n");
        return 0;
    }
    
    
    
    
    Machine m = {};
    ARR_INIT(m.r, 16, &heap);
    ARR_PUSH(m.r, nullptr);
    ARR_INIT(m.r[0], 16, arr_header(m.r)->heap);
    *R(m, R_ZERO) = 0;
    *R(m, R_PROG_CNT) = 0;
    *R(m, R_RETURN) = 0;
    *R(m, R_CONTEXT) = 0;
    *R(m, R_RETURN_ADDR) = (u64)-1;
    
    b8 after_abs_jmp = false;
    
    for(; *R(m,R_PROG_CNT) < ARR_LEN(instr_list) || (after_abs_jmp && *R(m, R_PROG_CNT) == ARR_LEN(instr_list)); ++(*R(m, R_PROG_CNT)))
    {
        
        if(after_abs_jmp)
        {
            --(*R(m, R_PROG_CNT));
            after_abs_jmp = false;
        }
        
        Instr i = instr_list[*R(m, R_PROG_CNT)];
        
        switch(i.R.opcode)
        {
            case OP_ADD:
            {
                *R(m, i.R.dest) = *R(m, i.R.op1) + *R(m, i.R.op2);
                break;   
            }
            case OP_SUB:  
            {
                *R(m, i.R.dest) = *R(m, i.R.op1) - *R(m, i.R.op2);
                break;
            }
            case OP_MUL:  
            {
                *R(m, i.R.dest) = *R(m, i.R.op1) * *R(m, i.R.op2);
                break;
            }
            case OP_DIV:  
            {
                *R(m, i.R.dest) = *R(m, i.R.op1) / *R(m, i.R.op2);
                break;
            }
            case OP_MOD:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) % *R(m, i.R.op2);
                break;
            }
            case OP_AND:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) & *R(m, i.R.op2);
                break;
            }
            case OP_OR:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) | *R(m, i.R.op2);
                break;
            }
            case OP_SHIFT_R:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) >> *R(m, i.R.op2);
                break;
            }
            case OP_SHIFT_L:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) << *R(m, i.R.op2);
                break;
            }
            case OP_8_ADD:
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) + (s8)*R(m, i.R.op2));
                break;   
            }
            case OP_8_SUB:  
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) - (s8)*R(m, i.R.op2));
                break;
            }
            case OP_8_MUL:  
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) * (s8)*R(m, i.R.op2));
                break;
            }
            case OP_8_DIV:  
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) / (s8)*R(m, i.R.op2));
                break;
            }
            case OP_8_MOD:  
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) % (s8)*R(m, i.R.op2));
                break;
            }
            case OP_8_SHIFT_R:  
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) >> (s8)*R(m, i.R.op2));
                break;
            }
            case OP_8_SHIFT_L:  
            {
                *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) << (s8)*R(m, i.R.op2));
                break;
            }
            case OP_CMP_AND:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) && *R(m, i.R.op2);
                break;
            }
            case OP_CMP_OR:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) || *R(m, i.R.op2);
                break;
            }
            case OP_CMP_EQ:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) == *R(m, i.R.op2);
                break;
            }
            case OP_CMP_NEQ:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) != *R(m, i.R.op2);
                break;
            }
            case OP_CMP_LT:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) < *R(m, i.R.op2);
                break;
            }
            case OP_CMP_GT:  
            {
                *R(m,i.R.dest) = *R(m, i.R.op1) > *R(m, i.R.op2);
                break;
            }
            case OP_IADD:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) + i.I.imm;
                break;
            }
            case OP_ISUB:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) - i.I.imm;
                break;
            }
            case OP_IMUL:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) * i.I.imm;
                break;
            }
            case OP_IDIV:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) / i.I.imm;
                break;
            }
            case OP_IMOD:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) % i.I.imm;
                break;
            }
            case OP_IAND:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) & i.I.imm;
                break;
            }
            case OP_IOR:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) | i.I.imm;
                break;
            }
            case OP_ISHIFT_R:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) >> i.I.imm;
                break;
            }
            case OP_ISHIFT_L:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) << i.I.imm;
                break;
            }
            case OP_8_IADD:  
            {
                *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) + (s8)i.I.imm);
                break;
            }
            case OP_8_ISUB:  
            {
                *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) - (s8)i.I.imm);
                break;
            }
            case OP_8_IMUL:  
            {
                *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) * (s8)i.I.imm);
                break;
            }
            case OP_8_IDIV:  
            {
                *R(m, i.I.dest) = ((s8)*R(m, i.I.op) / (s8)i.I.imm);
                break;
            }
            case OP_8_IMOD:  
            {
                *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) % (s8)i.I.imm);
                break;
            }
            case OP_8_ISHIFT_R:  
            {
                *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) >> (s8)i.I.imm);
                break;
            }
            case OP_8_ISHIFT_L:  
            {
                *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) << (s8)i.I.imm);
                break;
            }
            case OP_LOAD64:  
            {
                *R(m, i.I.dest) = ((s64*)*R(m, i.I.op))[i.I.imm];
                break;
            }
            case OP_LOAD8:  
            {
                *R(m, i.I.dest) = ((s8*)*R(m, i.I.op))[i.I.imm];
                break;
            }
            case OP_STORE64:  
            {
                ((s64*)*R(m, i.I.dest))[i.I.imm] = *R(m, i.I.op);
                break;
            }
            case OP_STORE8:  
            {
                ((s8*)*R(m, i.I.dest))[i.I.imm] = (s8)*R(m, i.I.op);
                break;
            }
            case OP_ICMP_AND:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) && i.I.imm;
                break;
            }
            case OP_ICMP_OR:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) || i.I.imm;
                break;
            }
            case OP_ICMP_EQ:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) == i.I.imm;
                break;
            }
            case OP_ICMP_NEQ:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) != i.I.imm;
                break;
            }
            case OP_ICMP_LT:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) < i.I.imm;
                break;
            }
            case OP_ICMP_GT:  
            {
                *R(m, i.I.dest) = *R(m, i.I.op) > i.I.imm;
                break;
            }
            case OP_BEQ:  
            {
                if(*R(m, i.I.dest) == *R(m, i.I.op))
                {
                    *R(m, R_PROG_CNT) += i.I.imm;
                }
                break;
            }
            case OP_BNE:  
            {
                if(*R(m, i.I.dest) != *R(m, i.I.op))
                {
                    *R(m, R_PROG_CNT) += i.I.imm;
                }
                break;
            }
            case OP_TO_F64:
            {
                *RF(m, i.I.dest) = *R(m, i.I.op);
                break;
            }
            case OP_F64_TO_S64:
            {
                *R(m, i.I.dest) = *RF(m, i.I.op);
                break;
            }
            case OP_F64_TO_C8:
            {
                *R(m, i.I.dest) = (s8)*RF(m, i.I.op);
                break;
            }
            case OP_F_ADD:
            {
                *RF(m, i.R.dest) = *RF(m, i.R.op1) + *RF(m, i.R.op2);
                break;   
            }
            case OP_F_SUB:  
            {
                *RF(m, i.R.dest) = *RF(m, i.R.op1) - *RF(m, i.R.op2);
                break;
            }
            case OP_F_MUL:  
            {
                *RF(m, i.R.dest) = *RF(m, i.R.op1) * *RF(m, i.R.op2);
                break;
            }
            case OP_F_DIV:  
            {
                *RF(m, i.R.dest) = *RF(m, i.R.op1) / *RF(m, i.R.op2);
                break;
            }
            case OP_F_CMP_LT:  
            {
                *R(m,i.R.dest) = *RF(m, i.R.op1) < *RF(m, i.R.op2);
                break;
            }
            case OP_F_CMP_GT:  
            {
                *R(m,i.R.dest) = *RF(m, i.R.op1) > *RF(m, i.R.op2);
                break;
            }
            case OP_F_IADD:  
            {
                *RF(m, i.I.dest) = *RF(m, i.I.op) + i.I.fImm;
                break;
            }
            case OP_F_ISUB:  
            {
                *RF(m, i.I.dest) = *RF(m, i.I.op) - i.I.fImm;
                break;
            }
            case OP_F_IMUL:  
            {
                *RF(m, i.I.dest) = *RF(m, i.I.op) * i.I.fImm;
                break;
            }
            case OP_F_IDIV:  
            {
                *RF(m, i.I.dest) = *RF(m, i.I.op) / i.I.fImm;
                break;
            }
            case OP_F_ICMP_LT:  
            {
                *R(m, i.I.dest) = *RF(m, i.I.op) < i.I.imm;
                break;
            }
            case OP_F_ICMP_GT:  
            {
                *R(m, i.I.dest) = *RF(m, i.I.op) > i.I.imm;
                break;
            }
            case OP_JMP:  
            {
                *R(m, R_PROG_CNT) = i.J.jmp;
                after_abs_jmp = true;
                break;
            }
            case OP_JMPR:  
            {
                *R(m, R_PROG_CNT) = *R(m, i.J.jmp);
                after_abs_jmp = true;
                if(i.J.jmp == R_FIRST_LOCAL && *R(m, R_FIRST_LOCAL) == (u64)-1)
                {
                    printf("Program int END: %lli\n", (s64)(*R(m, R_RETURN)) );
                    printf("Program float END: %f\n", (*RF(m, R_RETURN)));
                    return (*R(m, R_RETURN));
                }
                break;
            }
            case OP_WRITE_CONSTANT:
            {
                *R(m, i.C.dest) = i.C.imm;
                break;   
            }
            default:
            {
                fprintf(stderr, "INSTRUCTION NOT IMPLEMENTED: %.*s\n", opcode_to_str((Opcode)i.R.opcode));
                break;   
            }
        }
        
    }
    
    
    
    return 0;
}