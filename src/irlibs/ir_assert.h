#pragma once
#include <stdlib.h>

#ifndef IR_ASSERT
#define IR_STRINGIFY(x) #x
#define IR_TO_STRING(x) IR_STRINGIFY(x)

#define IR_ASSERT( EXPR ) do{ if(!(EXPR)){  fprintf(stderr, "ASSERTION FAILED: (%s) FILE: %s LINE: %s\n", #EXPR, __FILE__, IR_TO_STRING(__LINE__)); *(volatile int*)nullptr = 0; } }while(false)
#define IR_NOT_NULL( PTR ) do{ if(!(PTR)){ fprintf(stderr, "ASSERTION FAILED: (%s != nullptr) FILE: %s LINE: %s\n", #PTR, __FILE__, IR_TO_STRING(__LINE__));  *(volatile int*)nullptr = 0; } }while(false)
#define IR_INVALID_CASE    do{ if(!(false)){ fprintf(stderr, "ASSERTION FAILED: (INVALID_CASE) FILE: %s LINE: %s\n", __FILE__, IR_TO_STRING(__LINE__)); *(volatile int*)nullptr = 0; } }while(false)
#define IR_SOFT_ASSERT( EXPR )   if(!(EXPR)){ fprintf(stderr, "SOFT ASSERTION FAILED: (%s) FILE: %s LINE: %s\n", #EXPR, __FILE__, IR_TO_STRING(__LINE__)); }
#endif

