#include "feyutils.h"
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <math.h>
#include <stdbool.h>
static fey_arena_t global;
typedef char * string;
EnableArrayType(string);
static bool global_arena_set_up;
/*
typedef struct{
    void * ptr;
    size_t size;
}arena_chunk_t;
typedef struct{
    byte buffer[FEY_ARENA_SIZE];
    arena_chunk_t free_list[4096];
    size_t num_free;
    arena_chunk_t alloc_list[4096];
    size_t num_allocated;
} fey_arena_t;
*/
void fey_arena_hard_reset(fey_arena_t * arena){
    memset(arena->buffer, 0, FEY_ARENA_SIZE);
    memset(arena->alloc_list, 0, 4096*sizeof(arena_chunk_t));
    memset(arena->free_list, 0, 4096*sizeof(arena_chunk_t));
    arena->num_free =1;
    arena->num_allocated = 0;
    arena->free_list[0] = (arena_chunk_t){&arena->buffer, FEY_ARENA_SIZE};
}
void list_insert(arena_chunk_t * array, arena_chunk_t value, size_t * array_sz){
    int index = 0;
    for(int i =0; i<(*array_sz); i++){
        if(value.ptr<array[i].ptr){
            index = i;
            i = (*array_sz);
        }
    }
    for(int i =(*array_sz); i>=index; i--){
        array[i+1] = array[i]; 
    }
    array[index] = value;
    (*array_sz)++;
}
void list_remove(arena_chunk_t * array, void * pointer, size_t * array_sz){
    int index = -1;
    for(int i = 0; i<(*array_sz); i++){
        if(array[i].ptr == pointer){
            index = i;
            i = (*array_sz);
        }
    }
    if(index == -1){
        return;
    }
    for(int i = index+1; i<(*array_sz); i++){
        array[i-1] = array[i];
    }
    (*array_sz)--;
}
void * fey_arena_alloc(fey_arena_t * arena, size_t requested_size){
    if(arena == GLOBAL_ARENA){
        arena = &global;
        if(!global_arena_set_up){
            fey_arena_hard_reset(arena);
            global_arena_set_up = true;
        }
    }
    size_t size = requested_size;
    if(size<1){
        return NULL;
    }
    size = size/8+1;
    if(size<1){
        size = 8;
    }
    size = size*8;
    int index = -1;
    for(int i =0 ; i<arena->num_free; i++){
        if(arena->free_list[i].size>=size){
            index =i;
            i = arena->num_free;
        }
    }
    if(index == -1){
        return NULL;
    }
    void * ptr = arena->free_list[index].ptr;
    size_t new_sz = arena->free_list[index].size-size;
    list_insert(arena->alloc_list, (arena_chunk_t){ptr, size},&arena->num_allocated);
    list_remove(arena->free_list, ptr, &arena->num_free);
    if(new_sz>0){
        list_insert(arena->free_list, (arena_chunk_t){ptr+size, new_sz},&arena->num_free);
    }
    return ptr;
}
void fey_arena_free(fey_arena_t *arena, void * ptr){
    if(arena == GLOBAL_ARENA){
        arena = &global;
        if(!global_arena_set_up){
            fey_arena_hard_reset(arena);
            global_arena_set_up = true;
        }
    }
    int index = -1;
    for(int i = 0; i<arena->num_allocated; i++){
        if(arena->alloc_list[i].ptr == ptr){
            index = i;
            i = arena->num_allocated;
        }
    }
    if(index == -1){
        printf("\n failed\n");
        return;
    }
    list_insert(arena->free_list, arena->alloc_list[index], &arena->num_free);
    list_remove(arena->alloc_list, ptr, &arena->num_allocated);
merged:
    for(int i = 0;i<arena->num_free-1; i++){
        if(arena->free_list[i].ptr+arena->free_list[i].size == arena->free_list[i+1].ptr){
            arena->free_list[i].size+= arena->free_list[i+1].size;
            list_remove(arena->free_list, arena->free_list[i+1].ptr, &arena->num_free);
            goto merged;
        }
    }
}
void * fey_arena_realloc(fey_arena_t * arena, void * ptr, size_t requested_size){
    if(arena == GLOBAL_ARENA){
        arena = &global;
        if(!global_arena_set_up){
            fey_arena_hard_reset(arena);
            global_arena_set_up = true;
        }
    }
    int index = -1;
    for(int i = 0; i<arena->num_allocated; i++){
        if(arena->alloc_list[i].ptr == ptr){
            index = i;
            i = arena->num_allocated;
        }
    }
    if(index == -1){
        return NULL;
    }
    char * out = fey_arena_alloc(arena, requested_size);
    char * optr = arena->alloc_list[index].ptr;
    int sz = arena->alloc_list[index].size;
    if(sz>requested_size){
        sz = requested_size;
    }
    for(int i = 0; i<sz; i++){
        out[i] = optr[i];
    }
    fey_arena_free(arena, ptr);
    return out;
}

void fey_arena_debug(fey_arena_t * arena){
    if(arena == GLOBAL_ARENA){
        arena = &global;
        if(!global_arena_set_up){
            fey_arena_hard_reset(arena);
            global_arena_set_up = true;
        }
    }
    for(int i = 0; i<arena->num_free; i++){
        printf("{ptr: %p, size: %lu, free}", arena->free_list[i].ptr, arena->free_list[i].size);
    }
    for(int i = 0; i<arena->num_allocated; i++){
       printf("{ptr: %p, size: %lu, allocated}", arena->alloc_list[i].ptr, arena->alloc_list[i].size);
    }
    printf("\n");
}
/*
typedef struct{
    const char * data;
    size_t len;
    size_t alloc_len;
}fstr;
*/
fstr substring(char * v, int start, int end, fey_arena_t * arena){
    fey_arena_init();
	char * out = fey_arena_alloc(local, end-start);
	for(int i = start; i<end; i++){
		out[i-start] = v[i];
	}
	return from_str(local,out);
}
fstr from_str(fey_arena_t * arena, char * c){
    fstr out;
    out.len = strlen(c)+1;
    int actlen = ceil(log2(out.len));
    out.alloc_len = 1;
    for(int i =0 ; i<actlen; i++){
        out.alloc_len *= 2;
    }
    out.data = fey_arena_alloc(arena,out.alloc_len);
    for(int i = 0; i<strlen(c); i++){
        out.data[i] = c[i];
    }
    return out;
}
void fstr_push(fey_arena_t * arena, fstr * str, char c){
    if(str->alloc_len>str->len+1){
        str->data[str->len-1] = c;
        str->len++;
    }
    else{
        str->alloc_len *= 2;
        char * buff = fey_arena_alloc(arena, str->alloc_len);
        for(int i = 0; i<str->len-1; i++){
            buff[i] = str->data[i];
        }
        buff[str->len-1] = c;
        buff[str->len] = '\0';
        str->len++;
        fey_arena_free(arena,str->data);
        str->data = buff;
    }
}
fstr fstr_add(fey_arena_t * arena, fstr a, fstr b){
    fstr out = {0,0,0};
    for(int i = 0; i<a.len; i++){
        fstr_push(arena, &out, a.data[i]);
    }
    for(int i = 0; i<b.len; i++){
        fstr_push(arena, &out, b.data[i]);
    }
    return out;
}
bool fstr_eq(fstr a, fstr b){
    if(a.len != b.len){
        return false;
    }
    for(int i =0 ; i<a.len; i++){
        if(a.data[i] != b.data[i]){
            return false;
        }
    }
    return true;
}
bool scmp(const char * str, const char * cmp){
    int al = strlen(str);
    int bl = strlen(cmp);
    int len = (al*(al<bl))+(bl*(bl<=al));
    if(al>bl){
        return false;
    }
    for(int i = 0; i<len; i++){
        if(str[i] != cmp[i]){
            return false;
        }
    }
    return true;
}
static bool stringArrayContains(const stringArray_t *arr, const char * key){
    for(int i =0 ; i<arr->len; i++){
        if(scmp(arr->arr[i], key)){
            return true;
        }
    }
    return false;
}
static int stringArrayContainsLen(const stringArray_t *arr, const char * key){
    for(int i =0 ; i<arr->len; i++){
        if(scmp(arr->arr[i], key)){
            return strlen(arr->arr[i]);
        }
    }
    return 0;
}
typedef struct{
    char buffer[64000];
    char * ptrs[64000];
    int num_ptrs;
    int next;
} parsed_string_t;
stringArray_t parse_token_seperators(fey_arena_t *local, const char * seperators){
    stringArray_t out  = (stringArray_t){.alloc_len= 512, .arr = fey_arena_alloc(local, sizeof(string)*512), .len = 0};
    int index = 0;
    int len = strlen(seperators);
    char * current = fey_arena_alloc(local, 100);
    memset(current, 0, 100);
    for(int i = 0; i<strlen(seperators); i++){
        if(seperators[i] != ' '){
            if(seperators[i] != '\n'){
                current[index] = seperators[i];
                index++;
            }
        }
        else{
           // printf("<%s>",current);
           if(!stringArrayContains(&out, current)){
                stringArray_Push(local, &out, current);
                current = fey_arena_alloc(local, 100); 
            }
            memset(current, 0, 100);
            index = 0;
        }
    }
    if(current[0] && !stringArrayContains(&out, current)){
        stringArray_Push(local, &out, current);
    }
    return out;
}
void parsed_string_push(parsed_string_t * ps, char * string, int len){
    memcpy(&ps->buffer[ps->next], string, len);
    ps->ptrs[ps->num_ptrs] = &ps->buffer[ps->next];
    ps->next+=len+1;
    ps->num_ptrs++;
}
parsed_string_t parse_string(fey_arena_t * local, char * string, const char * token_seperators){
    parsed_string_t out;
    out.num_ptrs = 0;
    memset(out.buffer, 0, 64000);
    out.next = 0;
    stringArray_t tokens = parse_token_seperators(local, token_seperators);
    int len = strlen(string);
    const char * current =&string[0]; 
    for(int i =0; i<len; i++){
        const char * c = &string[i];
        if(c[0] ==' ' || c[0] == '\n'){
            parsed_string_push(&out, (char *)current, c-current);
            current +=(c-current+1);
            continue;
        }
        int l = stringArrayContainsLen(&tokens, c);
        if(l){
            parsed_string_push(&out, (char *)current, c-current);
            parsed_string_push(&out, (char *)c, l);
            current +=(c-current+l);
            i+= l;
        }
    }
    return out; 
}
fstrArray_t parse_fstr(fey_arena_t * arena, char * string, char * token_seperators){
    parsed_string_t t = parse_string(arena,string, token_seperators);
    fstrArray_t out;
    out.arr  = fey_arena_alloc(arena, t.num_ptrs*sizeof(fstr));
    out.alloc_len = t.num_ptrs;
    out.len = 0;
    for(int i = 0; i<t.num_ptrs; i++){
        fstr n = from_str(arena,t.ptrs[i]);
        if(strlen(n.data)){
            //printf("%s,", n.data);
            fstrArray_Push(arena, &out, n);
        }
    }
    return out;
}
void fstr_cat(fey_arena_t * arena, fstr * a, char * b){
    int l = strlen(b);
    for(int i =0 ; i<l; i++){
        fstr_push(arena, a, b[i]);
    }
}
void fstrprint(fstr str){
    printf("%s", str.data);
}
void fstrprintln(fstr str){
    printf("%s\n",str.data);
}
fstr fstrsprintf(fey_arena_t * arena, char * fmt_string,...){
    va_list ptr;
    va_start(ptr, fmt_string);
    char buff[1];
    int len = snprintf(buff, 1,fmt_string, ptr);
    fstr out;
    va_start(ptr, fmt_string);
    out.data = fey_arena_alloc(arena, len);
    sprintf(out.data, fmt_string, ptr);
    out.len = strlen(out.data);
    out.alloc_len = len;
    return out;
}