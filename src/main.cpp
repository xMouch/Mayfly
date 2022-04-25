#include "tokens.h"
#include "tokenizer.h"
#include "parser.h"

String read_entire_file(c8* file_name, Memory_Arena* arena)
{
    FILE* file;
    file = fopen(file_name, "rb+");
    
    u64 file_size = 0;
    
    fseek(file, 0, SEEK_END);
    file_size = ftell(file);
    fseek(file, 0, SEEK_SET);
    
    String result;
    result.length = file_size;
    result.data = (u8*)push_size(file_size, arena);
    
    fread(result.data, result.length, 1, file);
    
    fclose(file);
    
    return result;
}

int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(256), (u8*)malloc(IR_MEGABYTES(256)));

    String file;

    if (argc > 1){
        file = read_entire_file(argv[1], &arena);
    }else
        file = read_entire_file("testcode/test.mf", &arena);
    
    Heap_Allocator heap = create_heap(&arena, IR_MEGABYTES(64), 0);
    
    Token* tokens = tokenize(file, &heap);
    
    
    for(msi i = 0; i < ARR_LEN(tokens); ++i)
    {
        fprintf(stdout, "Type: %.*s, '%.*s'\n", type_to_str(tokens[i].type), tokens[i].text);
    }

    parse(tokens, &heap);
    
    return 0;
}
