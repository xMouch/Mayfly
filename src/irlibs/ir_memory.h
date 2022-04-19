#pragma once

#include "ir_types.h"
#include "ir_maths.h"

#ifndef IR_ASSERT
#define IR_ASSERT(ASSERT)
#define IR_NOT_NULL(PTR)
#define IR_INVALID_CASE
#define IR_SOFT_ASSERT(ASSERT)
#endif

struct Memory_Arena
{
    Buffer buffer;
    u8* current_base;
};

static Memory_Arena create_memory_arena(Buffer buffer);
static Memory_Arena create_memory_arena(msi length, u8* data);
static void free_memory_arena(Memory_Arena* arena);
static void* push_size(msi size, Memory_Arena* arena);
static void* push_size(msi size, Memory_Arena* arena, msi alignment);
static void* realloc_size(void* old_ptr, msi old_size, msi new_size, Memory_Arena* arena);
static Buffer create_buffer(msi length, Memory_Arena* arena);
static Buffer create_buffer(msi length, Memory_Arena* arena, msi alignment);
static b8 copy_buffer(Buffer from, Buffer to);
static b8 copy_buffer(Buffer from, Buffer to, msi start, msi length);
static b8 copy_buffer(Buffer from, Buffer to, msi start_read, msi start_write, msi length);
static void zero_buffer(Buffer buffer);
static b8 cmp_buffer(Buffer a, Buffer b);
static b8 ptr_in_buffer(Buffer buffer, void* ptr);
static void free_buffer_if_last(Buffer* buffer, Memory_Arena* arena);

static
Memory_Arena create_memory_arena(Buffer buffer)
{
    IR_ASSERT(buffer.data != nullptr && buffer.length != 0);
    Memory_Arena result = {};
    result.buffer = buffer;
    result.current_base = buffer.data;
    
    return result;
}


static 
Memory_Arena create_memory_arena(msi length, u8* data)
{
    return create_memory_arena(Buffer{length, data});    
}

static
void free_memory_arena(Memory_Arena* arena)
{
    arena->current_base=arena->buffer.data;
}

static
void* push_size(msi size, Memory_Arena* arena)
{
    void* result = arena->current_base;
    arena->current_base += size; 
    
    if(arena->current_base > arena->buffer.data + arena->buffer.length)
    {
        //TODO(Michael): Allocate more memory
        IR_SOFT_ASSERT(false && "Not Enough Memory in arena! and auto expand is not yet implemented!");
        result = nullptr;
    }
    return result;
}

static
void* push_size(msi size, Memory_Arena* arena, msi alignment)
{
    msi disalignment =((msi)arena->current_base) % alignment; 
    
    if(disalignment != 0)
    {
        arena->current_base += alignment - disalignment;
        
    }
    void* result = arena->current_base;
    arena->current_base += size; 
    
    
    if(arena->current_base > arena->buffer.data + arena->buffer.length)
    {
        //TODO(Michael): Allocate more memory
        IR_ASSERT(false);
        result = nullptr;
    }
    
    return result;
}

static
void* realloc_size(void* old_ptr, msi old_size, msi new_size, Memory_Arena* arena)
{
    u8* result = (u8*)old_ptr;
    u8* end_ptr = (result + old_size);    
    u8* new_end_ptr = (result + new_size);
    
    if(end_ptr == arena->current_base && new_end_ptr <= (arena->buffer.data + arena->buffer.length))
    {
        arena->current_base = new_end_ptr;
    }
    else
    {
        result = (u8*)push_size(new_size, arena);
        if(result)
        {
            if(!copy_buffer(Buffer{old_size, (u8*)old_ptr}, Buffer{new_size, result}))
            {
                arena->current_base -= new_size;
                result = nullptr;
            }
        }
    }
    
    return result;
}

static
Buffer create_buffer(msi length, Memory_Arena* arena)
{
    IR_NOT_NULL(arena);
    Buffer result = {};
    
    if(length)
    {
        result.length = length;
        result.data = (u8*)push_size(length, arena);
        
        if(!result.data)
        {
            result.length = 0;
        }
    }
    
    return result;
}

static
Buffer create_buffer(msi length, Memory_Arena* arena, msi alignment)
{
    IR_NOT_NULL(arena);
    Buffer result = {};
    
    if(length)
    {
        result.length = length;
        result.data = (u8*)push_size(length, arena, alignment);
        
        if(!result.data)
        {
            result.length = 0;
        }
    }
    
    return result;
}

inline
b8 copy_buffer(Buffer from, Buffer to)
{
#if 1
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
#else
    
    memcpy(to.data, from.data, from.length);    
    return true;
#endif
}

static
b8 copy_buffer(Buffer from, Buffer to, msi start, msi length)
{
    b8 result = false;
    if((start + length) <= from.length)
    {
        Buffer tmp_from = from;
        tmp_from.data = tmp_from.data + start;
        tmp_from.length = length;
        result = copy_buffer(tmp_from, to);
    }
    
    return result;
}

static
b8 copy_buffer(Buffer from, Buffer to, msi start_read, msi start_write, msi length)
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
        
        result = copy_buffer(tmp_from, tmp_to);
    }
    
    return result;
}

//Copies one buffer to another but starts coping on the higher address (save for overlapping buffers from < to) @NOTE does not check if buffers overlap!
static
b8 copy_buffer_reverse(Buffer from, Buffer to)
{
#if 1
    //TODO(Michael): Make it faster!
    b8 result = false;
    
    if(from.length <= to.length)
    {
        u64* from_u64 = (u64*)(&from.data[from.length-8]);
        u64* to_u64 = (u64*)(&to.data[from.length-8]);
        
        for(;(u8*)from.data <= (u8*)from_u64; from_u64--, to_u64--)
        {
            *to_u64 = *from_u64;
        }
        
        s64 index = (from.length & 7LLU)-1;
        
        for(s64 i = index; i >= 0; --i)
        {
            to.data[i] = from.data[i];
        }
        
        result = true;
    }
    
    return result;
#else
    memcpy(to.data, from.data, from.length);    
    return true;
#endif
    
}

static
void zero_buffer(Buffer buffer)
{
    //TODO(Michael): Make it faster!
    msi u64_length = buffer.length >> 3U;// divide by 8
    msi index = 0;
    for(; index < u64_length; ++index)
    {
        ((u64*)buffer.data)[index] = 0L;
    }
    
    //Remaining bytes
    index = index << 3U;
    for(; index < buffer.length; ++index)
    {
        buffer.data[index] = 0L;
    }
    
}

static
b8 cmp_buffer(Buffer a, Buffer b)
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

static
b8 ptr_in_buffer(Buffer buffer, void* ptr)
{
    b8 result = false;
    if(buffer.data <= ptr && (buffer.data+buffer.length) > ptr)
    {
        result = true;
    }
    return result;
}


static
void free_buffer_if_last(Buffer* buffer, Memory_Arena* arena)
{
    if((buffer->data + buffer->length) == arena->current_base)//Trivial case
    {
        arena->current_base = buffer->data;
    }
    
    buffer->data = nullptr;
    buffer->length = 0;
}


union Heap_Partition
{
    struct
    {
        u8 size_exp : 7;//real size = 2^size_exp
        b8 used : 1;
        u64 offset;
        Heap_Partition* n;
        Heap_Partition* p;
    };
    u8 _combined_bitfield;
};

struct Heap_Allocator
{
    Memory_Arena* arena;
    Buffer data;
    Buffer metadata;
    Heap_Partition** p_lists;
    u8  min_exp;
    u8  max_exp;
    u8  initialized;
};



//TODO(Michael) Write docs for this
#define DYN_INIT(MEM_ARENA_PTR, SIZE, MIN_EXP)  create_heap((MEM_ARENA_PTR), (SIZE), (MIN_EXP))         
#define DYN_INIT_BUFFER(BUFFER, MIN_EXP)           
#define DYN_ALLOC(SIZE, HEAP) heap_alloc((SIZE), false, (HEAP))          
#define DYN_ZALLOC(SIZE, HEAP) heap_alloc((SIZE), true, (HEAP))              
#define DYN_FREE(PTR, HEAP)   ((PTR) = (typeof(PTR))heap_free((PTR), (HEAP)))         
#define DYN_REALLOC(PTR, NEW_SIZE, HEAP) heap_realloc((void**)&(PTR), (NEW_SIZE), (HEAP))
#define DYN_RESIZE(PTR, NEW_SIZE, HEAP) heap_resize_in_place((PTR), (NEW_SIZE), (HEAP))  

static
msi heap_num_used_partitions(Heap_Allocator* heap)
{
    IR_NOT_NULL(heap);
    msi result = 0;    
    Heap_Partition* p = heap->p_lists[heap->max_exp+1 -heap->min_exp];
    while(p)
    {
        p = p->n;
        result++;
    }
    
    return result;
}

inline
void heap_remove_from_partition_list(Heap_Partition* part, Heap_Allocator* heap)
{
    if(part->p == nullptr)
    {
        u32 list_offset;
        if(part->used)
        {
            list_offset = (heap->max_exp+1) - heap->min_exp;
        }
        else
        {
            list_offset = part->size_exp - heap->min_exp;
        }
        
        heap->p_lists[list_offset] = part->n;
        if(part->n)
            part->n->p=nullptr;
    }
    else
    {
        part->p->n=part->n;
        if(part->n)
            part->n->p=part->p;
    }
}

inline
void heap_add_to_free_list(Heap_Partition* part, Heap_Allocator* heap)
{
    u16 list_index = part->size_exp - heap->min_exp;
    part->n=heap->p_lists[list_index];
    if(part->n)
        part->n->p=part;
    part->p = nullptr;
    heap->p_lists[list_index] = part;  
}

inline
void heap_add_to_used_list(Heap_Partition* part, Heap_Allocator* heap)
{
    if(part->p == nullptr)
    {
        heap->p_lists[part->size_exp - heap->min_exp] = part->n;
        if(part->n)
            part->n->p=nullptr;
    }
    else
    {
        part->p->n=part->n;
        if(part->n)
            part->n->p=part->p;
    }
    
    u16 used_list_index = (heap->max_exp+1)-heap->min_exp;
    part->n = heap->p_lists[used_list_index];
    heap->p_lists[used_list_index] = part;
    part->p = nullptr;
    if(part->n)
        part->n->p=part;
    part->used =true;
}

inline
void heap_remove_from_used_list(Heap_Partition* part, Heap_Allocator* heap)
{
    if(part->p == nullptr)
    {
        heap->p_lists[(heap->max_exp+1)-heap->min_exp] = part->n;
        if(part->n)
            part->n->p=nullptr;
    }
    else
    {
        part->p->n=part->n;
        if(part->n)
            part->n->p=part->p;
    }
    
    u16 list_index = part->size_exp - heap->min_exp;
    part->n = heap->p_lists[list_index];
    heap->p_lists[list_index] = part;
    part->p=nullptr;
    if(part->n)
        part->n->p=part;
    part->used=false;
}

inline
void heap_set_partition_size_free_list(Heap_Partition* part, u8 new_exp, Heap_Allocator* heap)
{
    if(part->p == nullptr)
    {
        heap->p_lists[part->size_exp - heap->min_exp] = part->n;
        if(part->n)
            part->n->p=nullptr;
    }
    else
    {
        part->p->n=part->n;
        if(part->n)
            part->n->p=part->p;
    }
    
    u16 new_index = new_exp - heap->min_exp;
    part->size_exp = new_exp;
    
    part->n=heap->p_lists[new_index];
    heap->p_lists[new_index] = part;
    part->p = nullptr;
    if(part->n)
        part->n->p=part;
}

static
void heap_split_partition(Heap_Partition* part, u8 target, Heap_Allocator* heap)
{
    IR_ASSERT(target <= part->size_exp);
    IR_ASSERT(target >= heap->min_exp);
    msi num_splits = part->size_exp - target;
    
    
    Heap_Partition split_partition = *part;
    split_partition.offset+= 1 << split_partition.size_exp;
    for(msi i = 0; i<num_splits;++i)
    {
        --split_partition.size_exp;
        split_partition.offset-= 1 << split_partition.size_exp;
        Heap_Partition* new_entry = ((Heap_Partition*)(&heap->data.data[split_partition.offset])); 
        *new_entry = split_partition;
        new_entry->used = false;
        
        heap_add_to_free_list(new_entry, heap);
    }
    if(part->used)
        part->size_exp = split_partition.size_exp;
    else
        heap_set_partition_size_free_list(part, split_partition.size_exp, heap);
}

static
void heap_try_merge_partition(Heap_Partition* part, Heap_Allocator* heap)
{
    if(!part->used)
    {
        msi buddy_offset = part->offset ^ ((msi)1U << part->size_exp);
        Heap_Partition* buddy = (Heap_Partition*)(&heap->data.data[buddy_offset]);
        if(buddy->offset == buddy_offset && !buddy->used && buddy->size_exp == part->size_exp)
        {
            if(part->offset < buddy->offset)
            {   
                heap_remove_from_partition_list(buddy, heap);
                heap_set_partition_size_free_list(part, part->size_exp+1, heap);
                heap_try_merge_partition(part, heap);
            } 
            else
            {
                heap_remove_from_partition_list(part, heap);
                heap_set_partition_size_free_list(buddy, buddy->size_exp+1, heap);
                heap_try_merge_partition(buddy, heap);
            }
        }
    }
    
}

static
Heap_Allocator create_heap(Memory_Arena* arena, msi size, u8 min_exp)
{
    IR_NOT_NULL(arena);
    Heap_Allocator heap = {};
    heap.arena = arena;
    
    heap.min_exp = u64_max(u64_log2_rounded_up(sizeof(Heap_Partition)*2), min_exp);
    heap.max_exp = u64_log2_rounded_down(size);
    
    if(heap.max_exp <= min_exp)
    {
        IR_ASSERT(size && "Size for heap is too small for given min_exp (max_exp must be > min_exp)!\n");
        return heap;
    }
    
    if(!size)
    {
        IR_SOFT_ASSERT(size && "Size for heap is zero!\n");
        return heap;
    }
    
    heap.data = create_buffer(size, arena);
    IR_SOFT_ASSERT(heap.data.data && "Create buffer failed for heap data!");
    heap.metadata = create_buffer(((2 + heap.max_exp) - heap.min_exp) * sizeof(Heap_Partition*), arena);
    IR_SOFT_ASSERT(heap.metadata.data && "Create buffer failed for heap metadata!");
    
    if(!heap.data.data || !heap.metadata.data)
    {
        IR_SOFT_ASSERT(false && "Heap creation failed!");
        heap = {};
        return heap;
    }
    zero_buffer(heap.metadata);
    
    heap.p_lists = (Heap_Partition**)heap.metadata.data;
    
    //Partition the buffer
    msi min_size = 1 << heap.min_exp;
    msi remaining_size = heap.data.length;
    msi offset = 0;
    while(remaining_size >= min_size)
    {
        Heap_Partition* entry = (Heap_Partition*)(&heap.data.data[offset]);
        entry->size_exp = u64_log2_rounded_down(remaining_size);
        entry->used = false;
        entry->offset = offset;
        heap_add_to_free_list(entry, &heap);
        offset+= 1 << entry->size_exp;        
        remaining_size -= 1 << entry->size_exp;
    }
    
    IR_SOFT_ASSERT(remaining_size == 0 && "The buffer given to the heap allocator cannot be partitioned effectively!");
    
    heap.data.length-=remaining_size;
    
    heap.initialized = true;
    return heap;
    
}


static
void* heap_free(void* ptr, Heap_Allocator* heap)
{
    if(!ptr)
        return nullptr;
    
    IR_ASSERT(ptr_in_buffer(heap->data, ptr));
    
    Heap_Partition* entry = &(((Heap_Partition*)ptr)[-1]);
    
    IR_ASSERT(entry->offset == ((msi)entry) - (msi)heap->data.data);
    
    heap_remove_from_used_list(entry, heap);
    
    heap_try_merge_partition(entry, heap);
    
    return nullptr;
}

static
void* heap_alloc(msi size, b8 clear, Heap_Allocator* heap)
{
    if(!size)
    {
        return nullptr;
    }
    
    u8 alloc_exp = u64_max(u64_log2_rounded_up(size + sizeof(Heap_Partition)), heap->min_exp);
    Heap_Partition* part = nullptr;
    
    for(msi i = alloc_exp; i < heap->max_exp+1; ++i)
    {
        part = heap->p_lists[i-heap->min_exp];
        
        if(part)
        {
            break;
        }
    }
    
    if(!part)
    {
        IR_SOFT_ASSERT(part && "Heap Allocation failed!\n");
        return nullptr;
    }
    
    
    if(alloc_exp < part->size_exp)
    {
        heap_split_partition(part, alloc_exp, heap);
        heap_add_to_used_list(part, heap);
    }
    else
    {
        heap_add_to_used_list(part, heap);
    }
    
    if(clear)
    {
        zero_buffer(IR_WRAP_INTO_BUFFER( (part+1),size));
    }
    
    return (part+1);
}


static
b8 heap_resize_in_place(void* ptr, msi new_size, Heap_Allocator* heap)
{   
    if(!ptr && new_size != 0)
        return false;
    
    IR_ASSERT(ptr_in_buffer(heap->data, ptr));
    Heap_Partition* entry = &(((Heap_Partition*)ptr)[-1]);
    
    u8 new_size_exp = u64_max(u64_log2_rounded_up(new_size + sizeof(Heap_Partition)), heap->min_exp);
    
    if(new_size == 0)
    {
        heap_free(ptr, heap);
        return true;
    }
    else if(new_size_exp == entry->size_exp)
    {
        return true;
    }
    else if(new_size_exp > entry->size_exp)
    {
        u8 size_diff = new_size_exp - entry->size_exp;
        Heap_Partition new_part = *entry;
        
        b8 can_expand = true;
        for(msi i = 0; i < size_diff; ++i)
        {
            msi buddy_offset = new_part.offset ^ ((msi)1U << new_part.size_exp);
            Heap_Partition* buddy = (Heap_Partition*)(&heap->data.data[buddy_offset]);
            
            if(!(buddy->offset == buddy_offset && !buddy->used && buddy->size_exp == new_part.size_exp && buddy->offset > new_part.offset))
            {
                can_expand = false;
                break;
            }
            new_part.size_exp++;
        }
        
        if(can_expand)
        {
            Heap_Partition new_part = *entry;
            for(msi i = 0; i < size_diff; ++i)
            {
                msi buddy_offset = new_part.offset ^ ((msi)1U << new_part.size_exp);
                Heap_Partition* buddy = (Heap_Partition*)(&heap->data.data[buddy_offset]);
                heap_remove_from_partition_list(buddy, heap);
                new_part.size_exp++;
            }
            *entry = new_part;
            return true;
        }   
    }
    else
    {
        heap_split_partition(entry, new_size_exp, heap);
        return true;
    }
    
    return false;
}

static
b8 heap_realloc(void** ptr, msi new_size, Heap_Allocator* heap)
{
    void* new_ptr = *ptr;
    
    if(!ptr || !new_ptr)
    {
        new_ptr = heap_alloc(new_size, false, heap);
    }
    else if(!heap_resize_in_place(*ptr, new_size, heap))
    {
        new_ptr = heap_alloc(new_size, false, heap);
        if(new_ptr)
        {
            Heap_Partition* old_entry = &((Heap_Partition*)*ptr)[-1];
            msi old_size = 1 << old_entry->size_exp;
            msi smallest_size = u64_min(new_size, old_size);
            copy_buffer(IR_WRAP_INTO_BUFFER(*ptr, smallest_size), IR_WRAP_INTO_BUFFER(new_ptr, smallest_size));
            
            heap_remove_from_used_list(old_entry, heap);    
        }
    }
    
    
    
    if(new_ptr)
    {
        *ptr = new_ptr;
        return true;
    }
    else
    {
        return false;
    }
}

