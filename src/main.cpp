#include "tokens.h"
#include "tokenizer.h"
#include "parser.h"
#include "generator.h"

String read_entire_file(String str_file_name, Memory_Arena* arena)
{
    c8* file_name = alloc_and_copy_to_asciiz(str_file_name, arena);
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
    
    Buffer buf = IR_WRAP_INTO_BUFFER(file_name, str_file_name.length+1);
    free_buffer_if_last(&buf, arena);
    
    return result;
}




int main(s32 argc, c8** argv)
{
    Memory_Arena arena = create_memory_arena(IR_MEGABYTES(2048), (u8*)malloc(IR_MEGABYTES(2048)));
    
    String file; 
    String input_file_name;
    if (argc > 1){
        
        input_file_name = wrap_asciiz(argv[1]);
    }
    else
    {
        input_file_name = IR_CONSTZ("testcode/mandelbrot.mf");
    }
        
    file = read_entire_file(input_file_name, &arena);
    
    Heap_Allocator heap = create_heap(&arena, IR_MEGABYTES(1024), 0);
    
    Token* tokens = tokenize(file, &heap);
    

    /*
    for(msi i = 0; i < ARR_LEN(tokens); ++i)
    {
        fprintf(stdout, "Type: %.*s, '%.*s'  Line: %llu  Column: %llu\n", type_to_str(tokens[i].type), tokens[i].text, tokens[i].line, tokens[i].column);
    //    fprintf(stdout, "%.*s", tokens[i].line_text);
    }
    */
    
    Parser_Result parser_result = parse(tokens, &heap);
    
    Metadata meta = generate(parser_result.ast, parser_result.reg_max, &heap);
    
    if (argc > 2){
        generate_binary(argv[2], &meta);
    }else
    {
        String output_file = search_string_last_occurrence(input_file_name, '/');
        msi cut_amount = search_string_first_occurrence(output_file, '.').length;
        output_file = substring(output_file, 1, output_file.length - (cut_amount +1)); 
        output_file = alloc_and_concat_string(output_file, IR_CONSTZ(".mayfly"), &arena);
        c8* output_file_asciiz = alloc_and_copy_to_asciiz(output_file, &arena);
        generate_binary(output_file_asciiz, &meta);
        Buffer buf = IR_WRAP_INTO_BUFFER(output_file_asciiz, output_file.length+1);
        free_buffer_if_last(&buf, &arena);
        free_buffer_if_last(&output_file, &arena);
    }
    
    if(argc == 1)
    {
        fprintf(stdout, "USAGE: ./mayflyc input_file.mf  [output_file.mayfly]\nRunning default: ./mayflyc testcode/mandelbrot.mf\n");   
    }
    return 0;
}
