#pragma once

#include "ir_types.h"
#include "ir_memory.h"

#ifndef IR_ASSERT
#define IR_ASSERT(ASSERT)
#define IR_NOT_NULL(PTR)
#define IR_INVALID_CASE
#define IR_SOFT_ASSERT(ASSERT)
#endif




static b8 cmp_string(String a, String b);
static b8 copy_string(String from, String to);
static b8 copy_string(String from, String to, msi start, msi length);
static b8 copy_string(String from, String to, msi start_read, msi start_write, msi length);
static b8 copy_string_to_asciiz(String from, c8* to, msi length_asciiz);
static msi count_asciiz_without_null(c8* asciiz);
static b8 copy_asciiz_to_string(c8* from, String to);
static String concat_string(String a, String b, Buffer buffer);
static String substring(String string, msi begin, msi length);
static b8 cmp_string_first_n_chars(String a, String b, msi length);
static String search_string_first_occurrence(String string, c8 to_search);
static String search_string_first_occurrence(String string, String to_search);
static String search_string_last_occurrence(String string, c8 c);
static void copyASCIIZ(c8* from, c8* to);
static String wrap_asciiz(c8* asciiz);
static b8 cmp_asciiz(c8* asciiz1, c8* asciiz2);
static b8 is_end_of_line(c8 c);
static b8 is_whitespace_without_end_of_line(c8 c);
static b8 is_whitespace(c8 c);
static b8 is_alpha(c8 c);
static b8 is_number(c8 c);
static c8 decimal_digit_to_c8(u64 digit);


static String alloc_and_copy_asciiz_to_string(c8* from,  Memory_Arena* arena);
static String alloc_and_concat_string(String a, String b, Memory_Arena* arena);
static String alloc_and_copy_string(String string, Memory_Arena* arena);
static c8* alloc_and_copy_to_asciiz(String string, Memory_Arena* arena);
static String s64_to_string(s64 number, Memory_Arena* arena);


static
String alloc_and_copy_asciiz_to_string(c8* from,  Memory_Arena* arena)
{
    String result = {};
    msi length_asciiz = count_asciiz_without_null(from);
    
    result = create_buffer(length_asciiz, arena);
    copy_string((String){length_asciiz, (u8*)from}, result);
    
    return result;
}

static
String alloc_and_concat_string(String a, String b, Memory_Arena* arena)
{
    msi size = a.length + b.length;
    Buffer buffer = create_buffer(size, arena);
    
    return concat_string(a, b, buffer); 
}

static
String alloc_and_copy_string(String string, Memory_Arena* arena)
{
    String result = create_buffer(string.length, arena);
    copy_string(string, result);
    return result;
    
}

static
c8* alloc_and_copy_to_asciiz(String string, Memory_Arena* arena)
{
    c8* result = nullptr;
    
    result = (c8*)push_size(string.length+1, arena);
    
    if(result)
    {
        copy_string_to_asciiz(string, result, string.length +1);
    }
    
    return result;
}

static
String s64_to_string(s64 number, Memory_Arena* arena)
{
    String result;
    if(number == 0)
    {
        result = alloc_and_copy_string(IR_CONSTZ("0"), arena);
    }
    else
    {
        c8 tmp[100] = {};
        s64 cursor = 0;
        s64 copy_of_number = number;
        b8 is_number = false;
        if(number < 0)
        {
            is_number = true;
            copy_of_number = -copy_of_number;
        }
        
        while(copy_of_number != 0 && cursor < 98)
        {
            u64 digit = copy_of_number % 10;
            tmp[cursor++] = decimal_digit_to_c8(digit);
            copy_of_number = (copy_of_number - digit) / 10;
        }
        if(is_number)
        {
            tmp[cursor++] = '-';
        }
        tmp[cursor--] = '\0';
        s64 length = cursor;
        
        while(cursor > length/2 && length > 0)
        {
            c8 swap_t = tmp[cursor];
            tmp[cursor] = tmp[length - cursor];
            tmp[length - cursor] = swap_t;
            --cursor;
        }
        
        result = alloc_and_copy_asciiz_to_string(tmp, arena);
    }
    
    
    
    
    return result;
}

static
b8 cmp_string(String a, String b)
{
    b8 result = false;
    
    if(a.length == b.length)
    {
        result = true;
        if(a.data != b.data)
        {
            //TODO(Michael): Make it faster!
            msi u64_length = a.length >> 3U;// divide by 8
            msi index = 0;
            for(; index < u64_length; ++index)
            {
                if(((u64*)a.data)[index] != ((u64*)b.data)[index])
                {
                    result = false;
                    return result;
                }
            }
            
            //Remaining bytes
            index = index << 3U;
            for(; index < a.length; ++index)
            {
                if(a.data[index] != b.data[index])
                {
                    result = false;
                    return result;
                }
            }
        }
    }
    
    return result;
}

//NOTE length of "to" string doesn't change
static
b8 copy_string(String from, String to)
{
    //TODO(Michael): Make it faster!
    b8 result = false;
    
    if(from.length <= to.length)
    {
        msi u64_length = from.length >> 3U;// divide by 8
        msi index = 0;
        for(; index < u64_length; ++index)
        {
            ((u64*)to.data)[index] = ((u64*)from.data)[index];
        }
        
        //Remaining bytes
        index = index << 3U;
        for(; index < from.length; ++index)
        {
            to.data[index] = from.data[index];
        }
        result = true;
    }
    
    return result;
}

static
b8 copy_string(String from, String to, msi start, msi length)
{
    b8 result = false;
    if((start + length) <= from.length)
    {
        Buffer tmp_from = from;
        tmp_from.data = tmp_from.data + start;
        tmp_from.length = length;
        result = copy_string(tmp_from, to);
    }
    
    return result;
}

static
b8 copy_string(String from, String to, msi start_read, msi start_write, msi length)
{
    b8 result = false;
    if((start_read + length) <= from.length && (start_read + length) <= to.length)
    {
        Buffer tmp_from = from;
        tmp_from.data = tmp_from.data + start_read;
        tmp_from.length = length;
        
        
        Buffer tmp_to = to;
        tmp_to.data = tmp_to.data + start_write;
        tmp_to.length = length;
        
        result = copy_string(tmp_from, tmp_to);
    }
    
    return result;
}

/**
 * @param from string to copy
 * @param to c8* to copy to
 * @param lengthOfASCIIZ length of buffer of c8* must be at least from.length+1
 * @return true if success false if failure
 */
static
b8 copy_string_to_asciiz(String from, c8* to, msi length_asciiz)
{
    b8 result = false;
    
    if(from.length < length_asciiz)
    {
        String tmp = {length_asciiz, (u8*)to};
        result = copy_string(from, tmp);
        to[from.length] = '\0';
    }
    
    return result;
}

static
msi count_asciiz_without_null(c8* asciiz)
{
    msi i = 0;
    for(;asciiz[i] != '\0'; ++i)
    {}
    return i;
}


static
b8 copy_asciiz_to_string(c8* from, String to)
{
    b8 result = false;
    msi length_asciiz = count_asciiz_without_null(from);
    
    if(length_asciiz <= to.length)
    {
        result = copy_string((String){length_asciiz, (u8*)from}, to);
    }
    
    return result;
}


static
String concat_string(String a, String b, Buffer buffer)
{
    String result = {};
    msi size = a.length + b.length;
    if(buffer.length >= size)
    {
        result.data = buffer.data;
        result.length = size;
        if(copy_string(a, result))
        {
            if(!copy_string(b, result, 0, a.length, b.length))
            {
                IR_INVALID_CASE;
                result = {};
            }
        }
        else
        {
            IR_INVALID_CASE;
            result = {};
        }
        
    }
    return result;
}

static
String substring(String string, msi begin, msi length)
{
    String result = {};
    if(begin <= string.length)
    {
        if(length + begin > string.length)
        {
            length = string.length - begin;
        }
        result.length = length;
        result.data = string.data + begin;
    }
    return result;
}

static
b8 cmp_string_first_n_chars(String a, String b, msi length)
{
    b8 result = true;
    
    for(msi i = 0; i < length; ++i)
    {
        
        if((i >= a.length || i >= b.length) && a.length != b.length)
        {
            result = false;
            break;
        }
        
        if(a.data[i] != b.data[i])
        {
            result = false;
            break;
        }
    }
    
    return result;
}

static
String search_string_first_occurrence(String string, c8 to_search)
{
    String result = string;
    
    if(string.length >= 1)
    {
        while(result.length && result.data[0] != to_search)
        {
            result = substring(result, 1, result.length-1);
        }
    }
    
    return result;
}

static
String search_string_first_occurrence(String string, String to_search)
{
    String result = string;
    
    if(string.length >= to_search.length)
    {
        while(!cmp_string_first_n_chars(result, to_search, to_search.length) && result.length)
        {
            result = substring(result, 1, result.length-1);
        }
    }
    
    return result;
}

static
String search_string_last_occurrence(String string, c8 c)
{
    String result = {};
    msi cursor = string.length -1;
    for(; cursor < string.length; --cursor)
    {
        if(string.data[cursor] == (u8)c)
        {
            result = substring(string, cursor, string.length - cursor);
            break;
        }
    }
    
    return result;
}


/**
 * Copies ASCIIZ from to including null terminator the to buffer must be big enough
 * @param from
 * @param to
 */
static
void copyASCIIZ(c8* from, c8* to)
{
    IR_NOT_NULL(from);
    IR_NOT_NULL(to);
    while(*from != '\0')
    {
        *to = *from;
        ++to;
        ++from;
    }
    *to = *from;
}


static
String wrap_asciiz(c8* asciiz)
{
    String result = {};
    result.length = count_asciiz_without_null(asciiz);
    result.data = (u8*)asciiz;
    return result;
}

static
b8 cmp_asciiz(c8* asciiz1, c8* asciiz2)
{
    if(asciiz1 == asciiz2)
    {
        return true;
    }
    
    msi i = 0;
    for(; ((asciiz1[i] != '\0') && (asciiz2[i] != '\0')); ++i)
    {
        if(asciiz1[i] != asciiz2[i])
            return false;
    }
    
    return asciiz1[i] == asciiz2[i];
}



static
b8 is_end_of_line(c8 c)
{
    return  (c == '\n' ||
             c == '\r');
}

static
b8 is_whitespace_without_end_of_line(c8 c)
{
    return (c == ' ' ||
            c == '\t'||
            c == '\v'||
            c == '\f');
};

static
b8 is_whitespace(c8 c)
{
    return (is_whitespace_without_end_of_line(c) ||
            is_end_of_line(c));
};

static
b8 is_alpha(c8 c)
{
    return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static
b8 is_number(c8 c)
{
    return (c == '0' || c == '1' ||
            c == '2' || c == '3' ||
            c == '4' || c == '5' ||
            c == '6' || c == '7' ||
            c == '8' || c == '9');
}


static
c8 decimal_digit_to_c8(u64 digit)
{
    switch(digit)
    {
        case 0: { return '0'; } break;
        case 1: { return '1'; } break;
        case 2: { return '2'; } break;
        case 3: { return '3'; } break;
        case 4: { return '4'; } break;
        case 5: { return '5'; } break;
        case 6: { return '6'; } break;
        case 7: { return '7'; } break;
        case 8: { return '8'; } break;
        case 9: { return '9'; } break;
        default:{ return '\0'; } break;
    }
}
