

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
        s64* r;
        f64* fR;
    };
};

int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(256), (u8*)malloc(IR_MEGABYTES(256)));
    
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
    ARR_INIT(m.r, 64, &heap);
    m.r[R_ZERO] = 0;
    m.r[R_PROG_CNT] = 0;
    m.r[R_RETURN_ADDR] = (u64)-1;
    m.r[R_RETURN] = 0;
    m.r[R_FRAME_PTR] = 0;
    m.r[R_STACK_PTR] = 0;
    
    b8 after_abs_jmp = false;
    
    for(; m.r[R_PROG_CNT] < ARR_LEN(instr_list); ++m.r[R_PROG_CNT])
    {
        
        if(after_abs_jmp)
        {
            --m.r[R_PROG_CNT];
            after_abs_jmp = false;
        }
        
        Instr i = instr_list[m.r[R_PROG_CNT]];

        switch(i.R.opcode)
        {
            case OP_ADD:
            {
                m.r[i.R.dest] = m.r[i.R.op1] + m.r[i.R.op2];
                break;   
            }
            case OP_SUB:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] - m.r[i.R.op2];
                break;
            }
            case OP_MUL:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] * m.r[i.R.op2];
                break;
            }
            case OP_DIV:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] / m.r[i.R.op2];
                break;
            }
            case OP_MOD:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] % m.r[i.R.op2];
                break;
            }
            case OP_AND:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] & m.r[i.R.op2];
                break;
            }
            case OP_OR:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] | m.r[i.R.op2];
                break;
            }
            case OP_SHIFT_R:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] >> m.r[i.R.op2];
                break;
            }
            case OP_SHIFT_L:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] << m.r[i.R.op2];
                break;
            }
            case OP_8_ADD:
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] + (s8)m.r[i.R.op2]);
                break;   
            }
            case OP_8_SUB:  
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] - (s8)m.r[i.R.op2]);
                break;
            }
            case OP_8_MUL:  
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] * (s8)m.r[i.R.op2]);
                break;
            }
            case OP_8_DIV:  
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] / (s8)m.r[i.R.op2]);
                break;
            }
            case OP_8_MOD:  
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] % (s8)m.r[i.R.op2]);
                break;
            }
            case OP_8_SHIFT_R:  
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] >> (s8)m.r[i.R.op2]);
                break;
            }
            case OP_8_SHIFT_L:  
            {
                m.r[i.R.dest] = (s8)((s8)m.r[i.R.op1] << (s8)m.r[i.R.op2]);
                break;
            }
            case OP_CMP_AND:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] && m.r[i.R.op2];
                break;
            }
            case OP_CMP_OR:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] || m.r[i.R.op2];
                break;
            }
            case OP_CMP_EQ:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] == m.r[i.R.op2];
                break;
            }
            case OP_CMP_NEQ:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] != m.r[i.R.op2];
                break;
            }
            case OP_CMP_LT:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] < m.r[i.R.op2];
                break;
            }
            case OP_CMP_GT:  
            {
                m.r[i.R.dest] = m.r[i.R.op1] > m.r[i.R.op2];
                break;
            }
            case OP_IADD:  
            {
                m.r[i.I.dest] = m.r[i.I.op] + i.I.imm;
                break;
            }
            case OP_ISUB:  
            {
                m.r[i.I.dest] = m.r[i.I.op] - i.I.imm;
                break;
            }
            case OP_IMUL:  
            {
                m.r[i.I.dest] = m.r[i.I.op] * i.I.imm;
                break;
            }
            case OP_IDIV:  
            {
                m.r[i.I.dest] = m.r[i.I.op] / i.I.imm;
                break;
            }
            case OP_IMOD:  
            {
                m.r[i.I.dest] = m.r[i.I.op] % i.I.imm;
                break;
            }
            case OP_IAND:  
            {
                m.r[i.I.dest] = m.r[i.I.op] & i.I.imm;
                break;
            }
            case OP_IOR:  
            {
                m.r[i.I.dest] = m.r[i.I.op] | i.I.imm;
                break;
            }
            case OP_ISHIFT_R:  
            {
                m.r[i.I.dest] = m.r[i.I.op] >> i.I.imm;
                break;
            }
            case OP_ISHIFT_L:  
            {
                m.r[i.I.dest] = m.r[i.I.op] << i.I.imm;
                break;
            }
            case OP_8_IADD:  
            {
                m.r[i.I.dest] = (s8)((s8)m.r[i.I.op] + (s8)i.I.imm);
                break;
            }
            case OP_8_ISUB:  
            {
                m.r[i.I.dest] = (s8)((s8)m.r[i.I.op] - (s8)i.I.imm);
                break;
            }
            case OP_8_IMUL:  
            {
                m.r[i.I.dest] = (s8)((s8)m.r[i.I.op] * (s8)i.I.imm);
                break;
            }
            case OP_8_IDIV:  
            {
                m.r[i.I.dest] = ((s8)m.r[i.I.op] / (s8)i.I.imm);
                break;
            }
            case OP_8_IMOD:  
            {
                m.r[i.I.dest] = (s8)((s8)m.r[i.I.op] % (s8)i.I.imm);
                break;
            }
            case OP_8_ISHIFT_R:  
            {
                m.r[i.I.dest] = (s8)((s8)m.r[i.I.op] >> (s8)i.I.imm);
                break;
            }
            case OP_8_ISHIFT_L:  
            {
                m.r[i.I.dest] = (s8)((s8)m.r[i.I.op] << (s8)i.I.imm);
                break;
            }
            case OP_LOAD64:  
            {
                m.r[i.I.dest] = ((s64*)m.r[i.I.op])[i.I.imm];
                break;
            }
            case OP_LOAD8:  
            {
                m.r[i.I.dest] = ((s8*)m.r[i.I.op])[i.I.imm];
                break;
            }
            case OP_STORE64:  
            {
                ((s64*)m.r[i.I.dest])[i.I.imm] = m.r[i.I.op];
                break;
            }
            case OP_STORE8:  
            {
                ((s8*)m.r[i.I.dest])[i.I.imm] = (s8)m.r[i.I.op];
                break;
            }
            case OP_ICMP_AND:  
            {
                m.r[i.I.dest] = m.r[i.I.op] && i.I.imm;
                break;
            }
            case OP_ICMP_OR:  
            {
                m.r[i.I.dest] = m.r[i.I.op] || i.I.imm;
                break;
            }
            case OP_ICMP_EQ:  
            {
                m.r[i.I.dest] = m.r[i.I.op] == i.I.imm;
                break;
            }
            case OP_ICMP_NEQ:  
            {
                m.r[i.I.dest] = m.r[i.I.op] != i.I.imm;
                break;
            }
            case OP_ICMP_LT:  
            {
                m.r[i.I.dest] = m.r[i.I.op] < i.I.imm;
                break;
            }
            case OP_ICMP_GT:  
            {
                m.r[i.I.dest] = m.r[i.I.op] > i.I.imm;
                break;
            }
            case OP_BEQ:  
            {
                if(m.r[i.I.dest] == m.r[i.I.op])
                {
                    m.r[R_PROG_CNT] += i.I.imm;
                }
                break;
            }
            case OP_BNE:  
            {
                if(m.r[i.I.dest] != m.r[i.I.op])
                {
                    m.r[R_PROG_CNT] += i.I.imm;
                }
                break;
            }
            case OP_TO_F64:
            {
                m.fR[i.I.dest] = m.r[i.I.op];
                break;
            }
            case OP_TO_S64:
            {
                m.r[i.I.dest] = m.fR[i.I.op];
                break;
            }
            
            case OP_F_ADD:
            {
                m.fR[i.R.dest] = m.fR[i.R.op1] + m.fR[i.R.op2];
                break;   
            }
            case OP_F_SUB:  
            {
                m.fR[i.R.dest] = m.fR[i.R.op1] - m.fR[i.R.op2];
                break;
            }
            case OP_F_MUL:  
            {
                m.fR[i.R.dest] = m.fR[i.R.op1] * m.fR[i.R.op2];
                break;
            }
            case OP_F_DIV:  
            {
                m.fR[i.R.dest] = m.fR[i.R.op1] / m.fR[i.R.op2];
                break;
            }
            case OP_F_CMP_LT:  
            {
                m.r[i.R.dest] = m.fR[i.R.op1] < m.fR[i.R.op2];
                break;
            }
            case OP_F_CMP_GT:  
            {
                m.r[i.R.dest] = m.fR[i.R.op1] > m.fR[i.R.op2];
                break;
            }
            case OP_F_IADD:  
            {
                m.fR[i.I.dest] = m.fR[i.I.op] + i.I.fImm;
                break;
            }
            case OP_F_ISUB:  
            {
                m.fR[i.I.dest] = m.fR[i.I.op] - i.I.fImm;
                break;
            }
            case OP_F_IMUL:  
            {
                m.fR[i.I.dest] = m.fR[i.I.op] * i.I.fImm;
                break;
            }
            case OP_F_IDIV:  
            {
                m.fR[i.I.dest] = m.fR[i.I.op] / i.I.fImm;
                break;
            }
            case OP_F_ICMP_LT:  
            {
                m.r[i.I.dest] = m.fR[i.I.op] < i.I.imm;
                break;
            }
            case OP_F_ICMP_GT:  
            {
                m.r[i.I.dest] = m.fR[i.I.op] > i.I.imm;
                break;
            }
            case OP_JMP:  
            {
                m.r[R_PROG_CNT] = i.J.jmp;
                after_abs_jmp = true;
                break;
            }
            case OP_JMPR:  
            {
                m.r[R_PROG_CNT] = m.r[i.J.jmp];
                after_abs_jmp = true;
                if(i.J.jmp == R_RETURN_ADDR && m.r[R_RETURN_ADDR] == (u64)-1)
                {
                    printf("Program END: %i\n", (s32)m.r[R_RETURN]);
                    return m.r[R_RETURN];
                }
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