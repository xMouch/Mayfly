#pragma once

#include "ir_types.h"
#include "ir_memory.h"

#ifndef IR_ASSERT
#define IR_ASSERT(ASSERT)
#define IR_NOT_NULL(PTR)
#define IR_INVALID_CASE
#define IR_SOFT_ASSERT(ASSERT)
#endif


struct Dyn_Array_Header
{
    msi length;
    msi capacity;
    Heap_Allocator* heap;
};

inline
Dyn_Array_Header* arr_header(void* arr)
{
    return (Dyn_Array_Header*) (arr) - 1;
}

inline
void* arr_growth_helper(void* arr, msi elem_size, msi min_cap)
{
    IR_NOT_NULL(arr);
    Dyn_Array_Header* header = arr_header(arr);
    IR_ASSERT(header->length < 0xffffffffffffff && "ARRAY length underrun?");
    if(min_cap < header->length)
    {
        return arr;
    }
    msi optimal_size = sizeof(Dyn_Array_Header) + elem_size * min_cap + sizeof(Heap_Partition);
    optimal_size = u64_max(u64_get_nearest_higher_or_equal_pow2(optimal_size), 1 << header->heap->min_exp);
    msi optimal_cap = (optimal_size - sizeof(Dyn_Array_Header) - sizeof(Heap_Partition))/elem_size;
    if(optimal_cap > header->capacity)
    {
        if(DYN_REALLOC(header, optimal_size, header->heap))
        {
            header->capacity=optimal_cap;
        }
        else
        {
            IR_ASSERT(false && "Array growth failed");
        }
        
        return header+1;
    }
    return arr;
}

inline 
void* arr_maybe_growth_helper(void* arr, msi elem_size, msi grow_by)
{
    IR_NOT_NULL(arr); 
    Dyn_Array_Header* header = arr_header(arr);
    msi min_cap = header->length + grow_by; 
    if(min_cap > header->capacity)
    {
        return arr_growth_helper(arr, elem_size, min_cap);
    }
    else
    {
        return arr;
    }
}


inline 
void* init_dyn_arr_helper(void* arr, msi elem_size, msi init_cap, Heap_Allocator* heap)
{
    IR_ASSERT(arr == nullptr);
    msi optimal_size = sizeof(Dyn_Array_Header) + elem_size * init_cap + sizeof(Heap_Partition);
    optimal_size = u64_max(u64_get_nearest_higher_or_equal_pow2(optimal_size), 1 <<heap->min_exp);
    u8* result = (u8*)DYN_ALLOC(optimal_size, heap);
    
    if(result)
    {
        msi optimal_cap = (optimal_size - sizeof(Dyn_Array_Header) - sizeof(Heap_Partition))/elem_size;
        result += sizeof(Dyn_Array_Header);
        *arr_header(result)={0,optimal_cap, heap};
    }
    IR_SOFT_ASSERT(result && "Could not initialize array because the heap allocation failed!");
    return result;
}

inline
b8 arr_del_n_helper(void* arr, msi elem_size, msi index, msi n)
{
    Dyn_Array_Header* header = arr_header(arr);
    IR_ASSERT(index + n <= header->length);
    b8 result = false;
    msi size = (header->length - (index + n)) * elem_size;
    result = copy_buffer(IR_WRAP_INTO_BUFFER((((u8*)arr)) + ((index + n) * elem_size), size), 
                         IR_WRAP_INTO_BUFFER(((u8*)arr) + (index * elem_size), size));
    IR_ASSERT(result);
    header->length -= n;
    
    return result;
}

inline
void* arr_insert_n_helper(void* arr, msi elem_size, msi index, msi n)
{
    arr = arr_maybe_growth_helper(arr, elem_size, n);
    Dyn_Array_Header* header = arr_header(arr);
    u8* start = ((u8*)arr) + (index*elem_size);
    msi size = (header->length - index) * elem_size;
    msi offset = n * elem_size;
    
    if(index >= header->length)
    {
        zero_buffer(IR_WRAP_INTO_BUFFER(((u8*)arr)+(header->length * elem_size),
                                        (index - header->length) * elem_size));
        header->length = index + n;
    }
    else
    {
        copy_buffer_reverse(IR_WRAP_INTO_BUFFER(start, size), IR_WRAP_INTO_BUFFER(start+offset, size));
        header->length+=n;
    }
    
    
    return arr;    
}

//TODO(Michael) Complete Documentation
/* DOCUMENTATION MSL DYNAMIC ARRAYS
 *
 * Declare an empty dynamic array of type T
 *  T* foo = nullptr;
 *  ARR_INIT(foo, init_cap, heap);
 * 
 * Access ith item in the array
 *  T item = foo[i];
 *
 *  Macro API
 *
 *  ARR_INIT:
 *     
 *     void ARR_INIT(T* arr, msi init_cap, Heap_Allocator* heap)
 *      Initializes arr with the heap and a start capacity
 *      The given heap allocator will be used for all further operations
 *
 * 
*/
#define ARR_INIT(arr, init_cap, heap_ptr) (arr = ((typeof(arr))init_dyn_arr_helper(arr, sizeof(*(arr)), init_cap, heap_ptr)))
#define ARR_FREE(arr) (heap_free(arr_header(arr), arr_header(arr)->heap) ? ((arr)=nullptr, true) : ((arr)=nullptr, false))
#define ARR_DEL_ALL(arr) (arr_header(arr)->length=0)
#define ARR_LEN_S(arr) ((s64)arr_header((arr))->length)
#define ARR_LEN(arr)  ((msi)arr_header((arr))->length)
#define ARR_PUSH(arr, elem)  (((arr) = (typeof(arr))arr_maybe_growth_helper((arr), sizeof(*(arr)), 1)) ? \
&((arr)[arr_header((arr))->length++] = (elem)) : nullptr)
#define ARR_POP(arr) (arr_header((arr))->length--, (arr)[arr_header((arr))->length])
#define ARR_DEL(arr, i) (arr_del_n_helper((arr), sizeof(*(arr)), (i), 1))
#define ARR_DEL_N(arr, i, n) (arr_del_n_helper((arr), sizeof(*(arr)), (i), (n)))
#define ARR_DEL_SWAP(arr, i) ((arr)[(i)] = (arr)[--arr_header((arr))->length])
#define ARR_CAP(arr) (arr_header((arr))->capacity)
#define ARR_SET_CAP(arr, n) ((arr) = (typeof(arr))arr_growth_helper((arr), sizeof(*(arr)), (n)))
#define ARR_INS_N(arr, i, n) ((arr) = (typeof(arr))arr_insert_n_helper((arr), sizeof(*(arr)), (i), (n)))
#define ARR_INS(arr, i, elem) ((arr) = ((typeof(arr))arr_insert_n_helper((arr), sizeof(*(arr)), (i), 1)), &((arr)[(i)] = (elem)))
#define ARR_ADD_N_PTR(arr, n) ((arr) = ((typeof(arr))arr_maybe_growth_helper((arr), sizeof(*(arr)), (n))),\
arr_header((arr))->length+=(n), &(arr)[arr_header((arr))->length-(n)])
#define ARR_ADD_N_INDEX(arr, n) ((arr) = ((typeof(arr))arr_maybe_growth_helper((arr), sizeof(*(arr)), (n))),\
arr_header((arr))->length+=(n), arr_header((arr))->length-(n))
#define ARR_LAST(arr) (&(arr)[arr_header((arr))->length-1])
