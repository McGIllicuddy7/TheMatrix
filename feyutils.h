#ifndef FEY_UTILS_H
#define FEY_UTILS_H
#include <stdlib.h>
#include <stdbool.h>
#define FEY_ARENA_SIZE 1280000
#define GLOBAL_ARENA 0
typedef unsigned char byte;
//the struct for handling memory allocation in arenas, you do not have to worry about this just please don't mess with it.
typedef struct{
    void * ptr;
    size_t size;
}arena_chunk_t;

//the struct for arena memory allocators, all memory allocated using an arena is automatically freed when the arena goes out of scope. you can manually request memory up to 1,280,000 bytes by using fey_arena_alloc and passing the arena as the a parameter
typedef struct{
    byte buffer[FEY_ARENA_SIZE];
    arena_chunk_t free_list[4096];
    size_t num_free;
    arena_chunk_t alloc_list[4096];
    size_t num_allocated;
} fey_arena_t;
//hard resets an arena, sets every bit in it to zero, and then resets all memory allocation done by that arena. the setting every bit to zero is for your convenience as it should become immediately obvious when this happens that something is wrong
void fey_arena_hard_reset(fey_arena_t * arena);
/*initializes an arena and creates a reference to it called local, pass the arena to functions that need to allocate memory by using "local" as the first parameter, if you want an arena to be passed as a parameter to a function the convention is 
"fey_arena_t * arena" as the first parameter. any function that calls for a fey_arena_t pointer will allocate or deallocate memory and that memory will be gone when the stack frame the arena is in is closed*/
#define fey_arena_init() fey_arena_t local_arena; fey_arena_t *__restrict__ const local = &local_arena; fey_arena_hard_reset(local);
//returns a pointer to a block of memory the size of the nearest multiple of eight to requested size(minimum 8 bytes), basically malloc for an arena allocator
void * fey_arena_alloc(fey_arena_t * arena, size_t requested_size);
//frees memory allocated an arena allocator, basically free for an arena allocator
void fey_arena_free(fey_arena_t *arena, void * ptr);
//reallocates memory allocated by an arena allocator, basically realloc for an arena allocator
void * fey_arena_realloc(fey_arena_t * arena, void * ptr, size_t requested_size);
//prints the memory address pointed to by every chunk of memory in an arena alloactor, whether or not it is free, and the size of it,
void fey_arena_debug(fey_arena_t * arena);
/*creates a dynamic array type for whatever parameter is passed to the macro which uses an arena allocator for its memory operations. The name of the type is TArray_t where T is the name of the type, TArray_New() returns 
a fully set up array, TArray_Push() adds an element of type T to the highest index of the array, TArray_Pop removes the element with the highest index from the array, TArray_Insert() adds an element at a given position and moves the element currently 
occupting the position and every element above it up one position, TArray_Remove() removes an element at the given index by moving element above that index down one,TArray_Iterate() calls the inputed function on each element of the array. TArray_Free 
frees only the memory that the array owns, e.g if it is an array of pointers it will not free any of the pointers. The convention for arrays of pointers is "typedef T* Tptr" where T is the type you want to have a pointer t*/
#define EnableArrayType(T)\
typedef struct{\
    T* arr;\
    size_t len;\
    size_t alloc_len;\
\
}T##Array_t;\
static T##Array_t T##Array_New(fey_arena_t *arena){\
    T* arr = fey_arena_alloc(arena,4*sizeof(T)); \
    return (T##Array_t){.arr = arr, .len = 0, .alloc_len = 4};\
}\
static void T##Array_Push(fey_arena_t * arena, T##Array_t*arr, T val){\
    if(arr->len+1>arr->alloc_len){\
        arr->alloc_len *=2;\
        T * arr_new = fey_arena_alloc(arena,arr->alloc_len*sizeof(T));\
        for(int i = 0; i<arr->len; i++){\
            arr_new[i] = arr->arr[i];\
        }\
    }\
        arr->arr[arr->len] = val;\
        arr->len++;\
}\
static void T##Array_Pop(T##Array_t *arr){\
    arr->len--;\
}\
static void T##Array_Insert(fey_arena_t * arena,T##Array_t *arr, long index, T val){\
    if(arr->len<=index){\
        T##Array_Push(arena,arr, val);\
        return;\
    }\
    if(arr->len+1>=arr->alloc_len){\
        arr->alloc_len *=2;\
        T * arr_new = fey_arena_alloc(arena, arr->alloc_len*sizeof(T));\
        for(int i = 0; i<arr->len; i++){\
            arr_new[i] = arr->arr[i];\
        }\
    }\
    for(int i = arr->len; i>=index; i--){\
        arr->arr[i+1] = arr->arr[i];\
    }\
    arr->arr[index] = val;\
    arr->len++;\
}\
static void T##Array_Remove(fey_arena_t * arena,T##Array_t *arr, long index){\
    if(index<0 || index>=arr->len){\
        return;\
    }\
    for(int i = index+1; i<arr->len; i++){\
         arr->arr[i-1] = arr->arr[i];\
    }\
    arr->len--;\
}\
static void T##Array_Free(fey_arena_t * arena, T##Array_t * arr){\
    fey_arena_free(arena,arr->arr);\
}\
static void T##Array_Iterate(T##Array_t * arr, void (*func)(T*)){\
    for(int i= 0; i<arr->len; i++){\
        func(&arr->arr[i]);\
    }\
}
/*creates a dynamic array with type TDynArray_t where T is the type name passed into the macro. This array is for long term state managament and it is not recommended for use other than that as it is more of a hassle memory wise.TDynArray_New() returns 
a fully set up array, TDynArray_Push() adds an element of type T to the highest index of the array, TDynArray_Pop removes the element with the highest index from the array, TDynArray_Insert() adds an element at a given position and moves the element currently 
occupting the position and every element above it up one position, TDynArray_Remove() removes an element at the given index by moving element above that index down one, TDynArray_Iterate() calls the inputted function on every element in the array,
TDynArray_Free() frees only the memory that the array owns, e.g if it is an array of pointers it will not free any of the pointers. The convention for arrays of pointers is "typedef T* Tptr" where T is the type you want to have a pointer to*/
#define EnableDynArrayType(T)\
typedef struct{\
    T* arr;\
    size_t len;\
    size_t alloc_len;\
\
}T##DynArray_t;\
static T##DynArray_t T##DynArray_New(){\
    T* arr = malloc(4*sizeof(T)); \
    return (T##DynArray_t){.arr = arr, .len = 0, .alloc_len = 4};\
}\
static void T##DynArray_Push(T##DynArray_t*arr, T val){\
    if(arr->len+1>arr->alloc_len){\
        arr->alloc_len *=2;\
        T * arr_new = malloc(arr->alloc_len*sizeof(T));\
        for(int i = 0; i<arr->len; i++){\
            arr_new[i] = arr->arr[i];\
        }\
    }\
        arr->arr[arr->len] = val;\
        arr->len++;\
}\
static void T##DynArray_Pop(T##DynArray_t *arr){\
    arr->len--;\
}\
static void T##DynArray_Insert(T##DynArray_t *arr, long index, T val){\
    if(arr->len<=index){\
        T##DynArray_Push(arr, val);\
        return;\
    }\
    if(arr->len+1>=arr->alloc_len){\
        arr->alloc_len *=2;\
        T * arr_new = malloc(arr->alloc_len*sizeof(T));\
        for(int i = 0; i<arr->len; i++){\
            arr_new[i] = arr->arr[i];\
        }\
    }\
    for(int i = arr->len; i>=index; i--){\
        arr->arr[i+1] = arr->arr[i];\
    }\
    arr->arr[index] = val;\
    arr->len++;\
}\
static void T##DynArray_Remove(T##DynArray_t *arr, long index){\
    if(index<0 || index>=arr->len){\
        return;\
    }\
    for(int i = index+1; i<arr->len; i++){\
         arr->arr[i-1] = arr->arr[i];\
    }\
    arr->len--;\
}\
static void T##DynArray_Free(T##DynArray_t * arr){\
    free(arr->arr);\
}\
static void T##DynArray_Iterate(T##DynArray_t * arr, void (*func)(T*)){\
    for(int i= 0; i<arr->len; i++){\
        func(&arr->arr[i]);\
    }\
}
typedef struct{
    char * data;
    size_t len;
    size_t alloc_len;
}fstr;
EnableArrayType(fstr);
fstr substring(char * v, int start, int end, fey_arena_t * arena);
fstr from_str(fey_arena_t * arena, char * c);
void fstr_push(fey_arena_t * arena, fstr * str, char c);
fstr fstr_add(fey_arena_t * arena, fstr a, fstr b);
void fstr_cat(fey_arena_t * arena, fstr * a, char * b);
bool fstr_eq(fstr a, fstr b);
fstrArray_t parse_fstr(fey_arena_t * arena, char * string, char * token_seperators);
void fstrprint(fstr str);
void fstrprintln(fstr str);
fstr fstrsprintf(fey_arena_t * arena, char * fmt_string, ...);
#endif
