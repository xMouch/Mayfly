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


static void F_ADD();
static void F_SUB();
static void F_MUL();
static void F_DIV();
static void F_MOD();
static void F_AND();
static void F_OR();
static void F_SHIFT_R();
static void F_SHIFT_L();
static void F_8_ADD();
static void F_8_SUB();
static void F_8_MUL();
static void F_8_DIV();
static void F_8_MOD();
static void F_8_SHIFT_R();
static void F_8_SHIFT_L();
static void F_CMP_AND();
static void F_CMP_OR();
static void F_CMP_EQ();
static void F_CMP_NEQ();
static void F_CMP_LT();
static void F_CMP_GT();
static void F_F_ADD();
static void F_F_SUB();
static void F_F_MUL();
static void F_F_DIV();
static void F_F_CMP_LT();
static void F_F_CMP_GT();
static void F_LOAD64();
static void F_LOAD8();
static void F_STORE64();
static void F_STORE8();
static void F_IADD();
static void F_ISUB();
static void F_IMUL();
static void F_IDIV();
static void F_IMOD();
static void F_IAND();
static void F_IOR();
static void F_ISHIFT_R();
static void F_ISHIFT_L();
static void F_8_IADD();
static void F_8_ISUB();
static void F_8_IMUL();
static void F_8_IDIV();
static void F_8_IMOD();
static void F_8_ISHIFT_R();
static void F_8_ISHIFT_L();
static void F_ICMP_AND();
static void F_ICMP_OR();
static void F_ICMP_EQ();
static void F_ICMP_NEQ();
static void F_ICMP_LT();
static void F_ICMP_GT();
static void F_BEQ();
static void F_BNE();
static void F_F_IADD();
static void F_F_ISUB();
static void F_F_IMUL();
static void F_F_IDIV();
static void F_F_ICMP_LT();
static void F_F_ICMP_GT();
static void F_TO_F64();
static void F_F64_TO_S64();
static void F_F64_TO_C8();
static void F_ILOAD64();
static void F_ILOAD8();
static void F_ISTORE64();
static void F_ISTORE8();
static void F_IADDR();
static void F_JMP();
static void F_JMPR();
static void F_WRITE_CONSTANT();
static void F_CREATE_CONTEXT();
 

Machine maschine = {};
Machine* m = {};
Instr i;

int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(1024), (u8*)malloc(IR_MEGABYTES(1024)));
    
    Heap_Allocator heap = create_heap(&arena, IR_MEGABYTES(512), 0);
    
    Instr* instr_list;
    
    if (argc > 1){
        instr_list = read_entire_file(argv[1], &heap);
    }else
    {
        fprintf(stdout, "USAGE: ./mayfly <path to binary>\n");
        return 0;
    }
    
    
    instruction* instructions = nullptr;
    
    ARR_INIT(instructions, (msi)OP_NUM_INSTRUCTIONS, &heap);
    
    ARR_PUSH(instructions, F_ADD);
    ARR_PUSH(instructions, F_SUB);
    ARR_PUSH(instructions, F_MUL);
    ARR_PUSH(instructions, F_DIV);
    ARR_PUSH(instructions, F_MOD);
    ARR_PUSH(instructions, F_AND);
    ARR_PUSH(instructions, F_OR);
    ARR_PUSH(instructions, F_SHIFT_R);
    ARR_PUSH(instructions, F_SHIFT_L);
    ARR_PUSH(instructions, F_8_ADD);
    ARR_PUSH(instructions, F_8_SUB);
    ARR_PUSH(instructions, F_8_MUL);
    ARR_PUSH(instructions, F_8_DIV);
    ARR_PUSH(instructions, F_8_MOD);
    ARR_PUSH(instructions, F_8_SHIFT_R);
    ARR_PUSH(instructions, F_8_SHIFT_L);
    ARR_PUSH(instructions, F_CMP_AND);
    ARR_PUSH(instructions, F_CMP_OR);
    ARR_PUSH(instructions, F_CMP_EQ);
    ARR_PUSH(instructions, F_CMP_NEQ);
    ARR_PUSH(instructions, F_CMP_LT);
    ARR_PUSH(instructions, F_CMP_GT);
    ARR_PUSH(instructions, F_F_ADD);
    ARR_PUSH(instructions, F_F_SUB);
    ARR_PUSH(instructions, F_F_MUL);
    ARR_PUSH(instructions, F_F_DIV);
    ARR_PUSH(instructions, F_F_CMP_LT);
    ARR_PUSH(instructions, F_F_CMP_GT);
    ARR_PUSH(instructions, F_LOAD64);
    ARR_PUSH(instructions, F_LOAD8);
    ARR_PUSH(instructions, F_STORE64);
    ARR_PUSH(instructions, F_STORE8);
    ARR_PUSH(instructions, F_IADD);
    ARR_PUSH(instructions, F_ISUB);
    ARR_PUSH(instructions, F_IMUL);
    ARR_PUSH(instructions, F_IDIV);
    ARR_PUSH(instructions, F_IMOD);
    ARR_PUSH(instructions, F_IAND);
    ARR_PUSH(instructions, F_IOR);
    ARR_PUSH(instructions, F_ISHIFT_R);
    ARR_PUSH(instructions, F_ISHIFT_L);
    ARR_PUSH(instructions, F_8_IADD);
    ARR_PUSH(instructions, F_8_ISUB);
    ARR_PUSH(instructions, F_8_IMUL);
    ARR_PUSH(instructions, F_8_IDIV);
    ARR_PUSH(instructions, F_8_IMOD);
    ARR_PUSH(instructions, F_8_ISHIFT_R);
    ARR_PUSH(instructions, F_8_ISHIFT_L);
    ARR_PUSH(instructions, F_ICMP_AND);
    ARR_PUSH(instructions, F_ICMP_OR);
    ARR_PUSH(instructions, F_ICMP_EQ);
    ARR_PUSH(instructions, F_ICMP_NEQ);
    ARR_PUSH(instructions, F_ICMP_LT);
    ARR_PUSH(instructions, F_ICMP_GT);
    ARR_PUSH(instructions, F_BEQ);
    ARR_PUSH(instructions, F_BNE);
    ARR_PUSH(instructions, F_F_IADD);
    ARR_PUSH(instructions, F_F_ISUB);
    ARR_PUSH(instructions, F_F_IMUL);
    ARR_PUSH(instructions, F_F_IDIV);
    ARR_PUSH(instructions, F_F_ICMP_LT);
    ARR_PUSH(instructions, F_F_ICMP_GT);
    ARR_PUSH(instructions, F_TO_F64);
    ARR_PUSH(instructions, F_F64_TO_S64);
    ARR_PUSH(instructions, F_F64_TO_C8);
    ARR_PUSH(instructions, F_ILOAD64);
    ARR_PUSH(instructions, F_ILOAD8);
    ARR_PUSH(instructions, F_ISTORE64);
    ARR_PUSH(instructions, F_ISTORE8);
    ARR_PUSH(instructions, F_IADDR);
    ARR_PUSH(instructions, F_JMP);
    ARR_PUSH(instructions, F_JMPR);
    ARR_PUSH(instructions, F_WRITE_CONSTANT);
    ARR_PUSH(instructions, F_CREATE_CONTEXT);
    
    

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
    
    for(;;)
    {
        i = instr_list[*R(m, R_PROG_CNT)];
        instructions[(msi)i.R.opcode]();
    }
    
    
    return 0;
}


static
void F_ADD()
{
    *R(m, i.R.dest) = *R(m, i.R.op1) + *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));
}

static
void F_SUB()  
{
    *R(m, i.R.dest) = *R(m, i.R.op1) - *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_MUL()  
{
    *R(m, i.R.dest) = *R(m, i.R.op1) * *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_DIV()  
{
    *R(m, i.R.dest) = *R(m, i.R.op1) / *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_MOD()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) % *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_AND()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) & *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_OR()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) | *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_SHIFT_R()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) >> *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_SHIFT_L()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) << *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_ADD()
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) + (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_SUB()  
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) - (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_MUL()  
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) * (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_DIV()  
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) / (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_MOD()  
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) % (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_SHIFT_R()  
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) >> (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_SHIFT_L()  
{
    *R(m,i.R.dest) = (s8)((s8)*R(m, i.R.op1) << (s8)*R(m, i.R.op2));
    ++(*R(m, R_PROG_CNT));    
}

static
void F_CMP_AND()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) && *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_CMP_OR()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) || *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_CMP_EQ()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) == *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_CMP_NEQ()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) != *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_CMP_LT()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) < *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_CMP_GT()  
{
    *R(m,i.R.dest) = *R(m, i.R.op1) > *R(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IADD()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) + i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ISUB()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) - i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IMUL()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) * i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IDIV()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) / i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IMOD()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) % i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IAND()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) & i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IOR()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) | i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ISHIFT_R()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) >> i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ISHIFT_L()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) << i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_IADD()  
{
    *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) + (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_ISUB()  
{
    *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) - (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_IMUL()  
{
    *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) * (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_IDIV()  
{
    *R(m, i.I.dest) = ((s8)*R(m, i.I.op) / (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_IMOD()  
{
    *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) % (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_ISHIFT_R()  
{
    *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) >> (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_8_ISHIFT_L()  
{
    *R(m, i.I.dest) = (s8)((s8)*R(m, i.I.op) << (s8)i.I.imm);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ILOAD64()
{
    *R(m, i.I.dest) = ((s64*)*R(m, i.I.op))[i.I.imm];
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ILOAD8()
{
    *R(m, i.I.dest) = ((s8*)*R(m, i.I.op))[i.I.imm];
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ISTORE64()
{
    ((s64*)*R(m, i.I.dest))[i.I.imm] = *R(m, i.I.op);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ISTORE8()
{
    ((s8*)*R(m, i.I.dest))[i.I.imm] = (s8)*R(m, i.I.op);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_LOAD64()
{
    *R(m, i.R.dest) = ((s64*)*R(m, i.R.op1))[*R(m, i.R.op2)];
    ++(*R(m, R_PROG_CNT));    
}

static
void F_LOAD8()
{
    *R(m, i.R.dest) = ((s8*)*R(m, i.R.op1))[*R(m, i.R.op2)];
    ++(*R(m, R_PROG_CNT));    
}

static
void F_STORE64()
{
    ((s64*)*R(m, i.R.dest))[*R(m, i.R.op2)] = *R(m, i.R.op1);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_STORE8()
{
    ((s8*)*R(m, i.R.dest))[*R(m, i.R.op2)] = (s8)*R(m, i.R.op1);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_IADDR()
{
    *R(m, i.I.dest) = (s64)R(m, i.I.op);
    ++(*R(m, R_PROG_CNT));
}

static
void F_ICMP_AND()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) && i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ICMP_OR()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) || i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ICMP_EQ()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) == i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ICMP_NEQ()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) != i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ICMP_LT()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) < i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_ICMP_GT()  
{
    *R(m, i.I.dest) = *R(m, i.I.op) > i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_BEQ()  
{
    if(*R(m, i.I.dest) == *R(m, i.I.op))
    {
        *R(m, R_PROG_CNT) += i.I.imm;
    }
    ++(*R(m, R_PROG_CNT));    
}

static
void F_BNE()  
{
    if(*R(m, i.I.dest) != *R(m, i.I.op))
    {
        *R(m, R_PROG_CNT) += i.I.imm;
    }
    ++(*R(m, R_PROG_CNT));    
}

static
void F_TO_F64()
{
    *RF(m, i.I.dest) = *R(m, i.I.op);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F64_TO_S64()
{
    *R(m, i.I.dest) = *RF(m, i.I.op);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F64_TO_C8()
{
    *R(m, i.I.dest) = (s8)*RF(m, i.I.op);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_ADD()
{
    *RF(m, i.R.dest) = *RF(m, i.R.op1) + *RF(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_SUB()  
{
    *RF(m, i.R.dest) = *RF(m, i.R.op1) - *RF(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_MUL()  
{
    *RF(m, i.R.dest) = *RF(m, i.R.op1) * *RF(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_DIV()  
{
    *RF(m, i.R.dest) = *RF(m, i.R.op1) / *RF(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_CMP_LT()  
{
    *R(m,i.R.dest) = *RF(m, i.R.op1) < *RF(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_CMP_GT()  
{
    *R(m,i.R.dest) = *RF(m, i.R.op1) > *RF(m, i.R.op2);
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_IADD()  
{
    *RF(m, i.I.dest) = *RF(m, i.I.op) + i.I.fImm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_ISUB()  
{
    *RF(m, i.I.dest) = *RF(m, i.I.op) - i.I.fImm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_IMUL()  
{
    *RF(m, i.I.dest) = *RF(m, i.I.op) * i.I.fImm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_IDIV()  
{
    *RF(m, i.I.dest) = *RF(m, i.I.op) / i.I.fImm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_ICMP_LT()  
{
    *R(m, i.I.dest) = *RF(m, i.I.op) < i.I.imm;
    ++(*R(m, R_PROG_CNT));    
}

static
void F_F_ICMP_GT()  
{
    *R(m, i.I.dest) = *RF(m, i.I.op) > i.I.imm;   
    ++(*R(m, R_PROG_CNT));    
}

static
void F_JMP()  
{
    if(i.J.jmp == (u64) -1)
    {
        //REALLOC
        *R(m, R_RETURN) = (s64)(void*)realloc((void*)(*R(m, R_FIRST_ARG)), *R(m, R_FIRST_ARG+1));
        zero_buffer(IR_WRAP_INTO_BUFFER(*R(m, R_RETURN),*R(m, R_FIRST_ARG + 1)));
        *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
    }
    else if(i.J.jmp == (u64) -2)
    {
        //PRINTS64
        *R(m, R_RETURN) = 0;
        printf("%lli\n", *R(m, R_FIRST_ARG));
        *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
    }
    else if(i.J.jmp == (u64) -3)
    {
        //PRINTF64
        //PRINTS64
        *R(m, R_RETURN) = 0;
        printf("%f\n", *RF(m, R_FIRST_ARG));
        *R(m, R_PROG_CNT) = *R(m, R_RETURN_ADDR);
    }
    else
    {
        *R(m, R_PROG_CNT) = i.J.jmp;
    }
}

static
void F_JMPR()  
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
}

static
void F_WRITE_CONSTANT()
{
    *R(m, i.C.dest) = i.C.imm;
    ++(*R(m, R_PROG_CNT));
}



static
void F_CREATE_CONTEXT()
{
    u64 context = get_next_free_context(m);
    u64 prev_context = *R(m, R_CONTEXT);
    *R(m, R_CONTEXT) = context;
    *R(m, R_FIRST_LOCAL + 1) = prev_context;
    ++(*R(m, R_PROG_CNT));
}
