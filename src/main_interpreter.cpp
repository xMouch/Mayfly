#include "generator.h"

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

Instr* read_entire_file(c8* file_name, Heap_Allocator* heap, String** str_literals)
{
    FILE* file;
    file = fopen(file_name, "rb+");

    ARR_INIT(*str_literals, 5, heap);

    msi lit_cnt;
    fread(&lit_cnt, sizeof(lit_cnt),1, file);
    for (msi i = 0; i<lit_cnt; i++){
        msi ln;
        fread(&ln, sizeof(ln),1, file);
        String s = create_buffer(ln, heap->arena);
        fread(s.data, sizeof(c8)*ln,1, file);
        s.length = ln;

        ARR_PUSH(*str_literals, s);
    }

    u64 instr_start = ftell(file);
    fseek(file, 0, SEEK_END);
    u64 instr_size = ftell(file) - instr_start;
    fseek(file, instr_start, SEEK_SET);
    
    Instr* result = nullptr;
    
    msi num_instr = instr_size/sizeof(Instr);
    
    ARR_INIT(result, num_instr, heap);
    
    fread(result, instr_size, 1, file);
    
    arr_header(result)->length = instr_size/sizeof(Instr);
    
    fclose(file);
    
    return result;
}

struct Machine 
{
    union{
        s64** r;
        f64** fR;
    };
    
    u64* context_free_list;
};

inline
s64*
R(Machine* m, u32 reg)
{
    if(reg < R_FIRST_LOCAL)
    {
        if(ARR_LEN(m->r[0]) <= reg)
        {
            ARR_INS(m->r[0], reg, (u64)0);
        }
        return &m->r[0][reg];
    }
    else
    {
        reg -= R_FIRST_LOCAL;
        u64 context = m->r[0][R_CONTEXT];
        
        if(ARR_LEN(m->r[context]) <= reg)
        {
            ARR_INS(m->r[context], reg, (u64)0);
        }
        return &m->r[context][reg]; 
    }
}

inline
f64* RF(Machine* m, u32 reg)
{ 
    return (f64*)(void*)R(m, reg);
}

inline
void free_context(u64 context, Machine* m)
{
    ARR_PUSH(m->context_free_list, context);
    ARR_DEL_ALL(m->r[context]);
}

inline
u64 get_next_free_context(Machine* m)
{
    u64 result = 0;
    if(ARR_LEN(m->context_free_list) == 0)
    {
        u64 context = ARR_LEN(m->r);
        ARR_PUSH(m->r, nullptr);
        ARR_INIT(m->r[context], 8, arr_header(m->r)->heap);
        result = context;
    }
    else
    {
        result = ARR_POP(m->context_free_list);
    }
    return result;
}

typedef void (*instruction)();


int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(1024), (u8*)malloc(IR_MEGABYTES(1024)));
    
    Heap_Allocator heap = create_heap(&arena, IR_MEGABYTES(512), 0);
    
    Instr* instr_list;

    String* str_literals;
    
    if (argc > 1){
        instr_list = read_entire_file(argv[1], &heap, &str_literals);
    }else
    {
        fprintf(stdout, "USAGE: ./mayfly <path to binary>\n");
        return 0;
    }
    
    
    //instruction* instructions = nullptr;
    void* jump_table[] = 
    {
        &&L_ADD,
        &&L_SUB,
        &&L_MUL,
        &&L_DIV,
        &&L_MOD,
        &&L_AND,
        &&L_OR,
        &&L_SHIFT_R,
        &&L_SHIFT_L,
        &&L_8_ADD,
        &&L_8_SUB,
        &&L_8_MUL,
        &&L_8_DIV,
        &&L_8_MOD,
        &&L_8_SHIFT_R,
        &&L_8_SHIFT_L,
        &&L_CMP_AND,
        &&L_CMP_OR,
        &&L_CMP_EQ,
        &&L_CMP_NEQ,
        &&L_CMP_LT,
        &&L_CMP_GT,
        &&L_F_ADD,
        &&L_F_SUB,
        &&L_F_MUL,
        &&L_F_DIV,
        &&L_F_CMP_LT,
        &&L_F_CMP_GT,
        &&L_LOAD64,
        &&L_LOAD8,
        &&L_STORE64,
        &&L_STORE8,
        &&L_IADD,
        &&L_ISUB,
        &&L_IMUL,
        &&L_IDIV,
        &&L_IMOD,
        &&L_IAND,
        &&L_IOR,
        &&L_ISHIFT_R,
        &&L_ISHIFT_L,
        &&L_8_IADD,
        &&L_8_ISUB,
        &&L_8_IMUL,
        &&L_8_IDIV,
        &&L_8_IMOD,
        &&L_8_ISHIFT_R,
        &&L_8_ISHIFT_L,
        &&L_ICMP_AND,
        &&L_ICMP_OR,
        &&L_ICMP_EQ,
        &&L_ICMP_NEQ,
        &&L_ICMP_LT,
        &&L_ICMP_GT,
        &&L_BEQ,
        &&L_BNE,
        &&L_F_IADD,
        &&L_F_ISUB,
        &&L_F_IMUL,
        &&L_F_IDIV,
        &&L_F_ICMP_LT,
        &&L_F_ICMP_GT,
        &&L_TO_F64,
        &&L_F64_TO_S64,
        &&L_F64_TO_C8,
        &&L_ILOAD64,
        &&L_ILOAD8,
        &&L_ISTORE64,
        &&L_ISTORE8,
        &&L_IADDR,
        &&L_STR,
        &&L_JMP,
        &&L_JMPR,
        &&L_WRITE_CONSTANT,
        &&L_CREATE_CONTEXT,
    };
    
    Machine maschine = {};
    Machine* m = {};
    Instr i;

    ARR_INIT(maschine.r, 16, &heap);
    ARR_INIT(maschine.context_free_list, 16, &heap);
    ARR_PUSH(maschine.r, nullptr);
    ARR_PUSH(maschine.r, nullptr);
    ARR_INIT(maschine.r[0], 16, arr_header(maschine.r)->heap);
    ARR_INIT(maschine.r[1], 16, arr_header(maschine.r)->heap);
    m = &maschine;
    *R(m, R_ZERO) = 0;
    *R(m, R_PROG_CNT) = 0;
    *R(m, R_RETURN) = 0;
    *R(m, R_CONTEXT) = 0;
    *R(m, R_RETURN_ADDR) = (u64)-1;
    
    i={};
    
    
    NEXT:
    i = instr_list[*R(m, R_PROG_CNT)];
    goto *jump_table[(msi)i.R.opcode];
    
    
    L_ADD:
    {
        *R(m, i.R.dest) = *R(m, i.R.op1) + *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;
    }
    
    L_SUB:  
    {
        *R(m, i.R.dest) = *R(m, i.R.op1) - *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_MUL:  
    {
        *R(m, i.R.dest) = *R(m, i.R.op1) * *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_DIV:  
    {
        *R(m, i.R.dest) = *R(m, i.R.op1) / *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_MOD:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) % *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_AND:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) & *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_OR:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) | *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_SHIFT_R:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) >> *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_SHIFT_L:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) << *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_ADD:
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) + (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_SUB:  
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) - (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_MUL:  
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) * (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_DIV:  
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) / (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_MOD:  
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) % (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_SHIFT_R:  
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) >> (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_SHIFT_L:  
    {
        *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) << (s8)*R(m, i.R.op2));
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_CMP_AND:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) && *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_CMP_OR:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) || *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_CMP_EQ:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) == *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_CMP_NEQ:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) != *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_CMP_LT:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) < *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_CMP_GT:  
    {
        *R(m,i.R.dest) = *R(m, i.R.op1) > *R(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IADD:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) + i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ISUB:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) - i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IMUL:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) * i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IDIV:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) / i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IMOD:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) % i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IAND:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) & i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IOR:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) | i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ISHIFT_R:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) >> i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ISHIFT_L:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) << i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_IADD:  
    {
        *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) + (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_ISUB:  
    {
        *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) - (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_IMUL:  
    {
        *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) * (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_IDIV:  
    {
        *R(m, i.I.dest) = ((s8)*R(m, i.I.op) / (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_IMOD:  
    {
        *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) % (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_ISHIFT_R:  
    {
        *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) >> (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_8_ISHIFT_L:  
    {
        *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) << (s8)i.I.imm);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ILOAD64:
    {
        *R(m, i.I.dest) = ((s64*)*R(m, i.I.op))[i.I.imm];
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ILOAD8:
    {
        *R(m, i.I.dest) = ((s8*)*R(m, i.I.op))[i.I.imm];
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ISTORE64:
    {
        ((s64*)*R(m, i.I.dest))[i.I.imm] = *R(m, i.I.op);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ISTORE8:
    {
        ((s8*)*R(m, i.I.dest))[i.I.imm] = (s8)*R(m, i.I.op);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_LOAD64:
    {
        *R(m, i.R.dest) = ((s64*)*R(m, i.R.op1))[*R(m, i.R.op2)];
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_LOAD8:
    {
        *R(m, i.R.dest) = ((s8*)*R(m, i.R.op1))[*R(m, i.R.op2)];
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_STORE64:
    {
        ((s64*)*R(m, i.R.dest))[*R(m, i.R.op2)] = *R(m, i.R.op1);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_STORE8:
    {
        ((s8*)*R(m, i.R.dest))[*R(m, i.R.op2)] = (s8)*R(m, i.R.op1);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_IADDR:
    {
        *R(m, i.I.dest) = (s64)R(m, i.I.op);
        ++maschine.r[0][R_PROG_CNT];
        goto NEXT;    
    }
    
    L_ICMP_AND:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) && i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ICMP_OR:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) || i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ICMP_EQ:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) == i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ICMP_NEQ:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) != i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ICMP_LT:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) < i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_ICMP_GT:  
    {
        *R(m, i.I.dest) = *R(m, i.I.op) > i.I.imm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_BEQ:  
    {
        if(*R(m, i.I.dest) == *R(m, i.I.op))
        {
            *R(m, R_PROG_CNT) += i.I.imm;
        }
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_BNE:  
    {
        if(*R(m, i.I.dest) != *R(m, i.I.op))
        {
            *R(m, R_PROG_CNT) += i.I.imm;
        }
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_TO_F64:
    {
        *RF(m, i.I.dest) = *R(m, i.I.op);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F64_TO_S64:
    {
        *R(m, i.I.dest) = *RF(m, i.I.op);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F64_TO_C8:
    {
        *R(m, i.I.dest) = (s8)*RF(m, i.I.op);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_ADD:
    {
        *RF(m, i.R.dest) = *RF(m, i.R.op1) + *RF(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_SUB:  
    {
        *RF(m, i.R.dest) = *RF(m, i.R.op1) - *RF(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_MUL:  
    {
        *RF(m, i.R.dest) = *RF(m, i.R.op1) * *RF(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_DIV:  
    {
        *RF(m, i.R.dest) = *RF(m, i.R.op1) / *RF(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_CMP_LT:  
    {
        *R(m,i.R.dest) = *RF(m, i.R.op1) < *RF(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_CMP_GT:  
    {
        *R(m,i.R.dest) = *RF(m, i.R.op1) > *RF(m, i.R.op2);
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_IADD:  
    {
        *RF(m, i.I.dest) = *RF(m, i.I.op) + i.I.fImm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_ISUB:  
    {
        *RF(m, i.I.dest) = *RF(m, i.I.op) - i.I.fImm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_IMUL:  
    {
        *RF(m, i.I.dest) = *RF(m, i.I.op) * i.I.fImm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_IDIV:  
    {
        *RF(m, i.I.dest) = *RF(m, i.I.op) / i.I.fImm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_ICMP_LT:  
    {
        *R(m, i.I.dest) = *RF(m, i.I.op) < i.I.fImm;
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }
    
    L_F_ICMP_GT:  
    {
        *R(m, i.I.dest) = *RF(m, i.I.op) > i.I.fImm;   
        ++maschine.r[0][R_PROG_CNT];    
        goto NEXT;    
    }

    L_STR:
    {
        *R(m, i.I.dest) = (s64)str_literals[i.I.imm].data;
        ++maschine.r[0][R_PROG_CNT];
        goto NEXT;
    }
    
    L_JMP:  
    {
        switch(i.J.jmp)
        {
            case (u64) -1:
            {
                //REALLOC
                *R(m, R_RETURN) = (s64)(void*)realloc((void*)(*R(m, R_FIRST_ARG)), *R(m, R_FIRST_ARG+1));
                zero_buffer(IR_WRAP_INTO_BUFFER(*R(m, R_RETURN),*R(m, R_FIRST_ARG + 1)));
                *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
                break;
            }
            case (u64) -2:
            {
                //PRINTS64
                *R(m, R_RETURN) = 0;
                printf("%lli", *R(m, R_FIRST_ARG));
                *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
                break;
            }
            case (u64) -3:
            {
                //PRINTF64
                *R(m, R_RETURN) = 0;
                printf("%f", *RF(m, R_FIRST_ARG));
                *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
                break;
            }
            case (u64) -4:
            {
                //PRINTSTRING
                *R(m, R_RETURN) = 0;
                printf("%s", (c8*)*R(m, R_FIRST_ARG));
                *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
                break;
            }
            case (u64) -5:
            {
                //Write image
                *R(m, R_RETURN) = 0;
                stbi_write_bmp((c8*)*R(m, R_FIRST_ARG), (s64)*R(m, R_FIRST_ARG+1), (s64)*R(m, R_FIRST_ARG+2),
                               (s64)*R(m, R_FIRST_ARG+3), (c8*)*R(m, R_FIRST_ARG+4));
                *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
                break;
            }
            default:
            {
                *R(m, R_PROG_CNT) = i.J.jmp;
            }
        }
        goto NEXT;    
    }
    
    L_JMPR:  
    {
        *R(m, R_PROG_CNT) = *R(m, i.J.jmp);
        if(i.J.jmp == R_FIRST_LOCAL && *R(m, R_FIRST_LOCAL) == (u64)-1)
        {
            printf("Program int END: %lli\n", (s64)(*R(m, R_RETURN)) );
            printf("Program c8 END: %lli\n", (s64)(s8)(*R(m, R_RETURN)) );
            printf("Program float END: %f\n", (*RF(m, R_RETURN)));
            exit(*R(m, R_RETURN));
        }
        else if(i.J.jmp == R_FIRST_LOCAL)
        {
            u64 cur_context = *R(m, R_CONTEXT);
            u64 prev_context = *R(m, R_FIRST_LOCAL + 1);
            *R(m, R_CONTEXT) = prev_context;
            free_context(cur_context, m);
        }
        goto NEXT;    
    }
    
    L_WRITE_CONSTANT:
    {
        *R(m, i.C.dest) = i.C.imm;
        ++maschine.r[0][R_PROG_CNT];
        goto NEXT;
    }
    
    
    
    L_CREATE_CONTEXT:
    {
        u64 context = get_next_free_context(m);
        u64 prev_context = maschine.r[0][R_CONTEXT];
        maschine.r[0][R_CONTEXT] = context;
        *R(m, R_FIRST_LOCAL + 1) = prev_context;
        *R(m, R_FIRST_LOCAL) = maschine.r[0][R_RETURN_ADDR];
        for(msi index = 0; index < i.C.imm; ++index)
        {
            *R(m, R_FIRST_LOCAL + 2 + index) = maschine.r[0][R_FIRST_ARG+index];
        }
        ++maschine.r[0][R_PROG_CNT];
        goto NEXT;
    }
    
    
    return 0;
}
