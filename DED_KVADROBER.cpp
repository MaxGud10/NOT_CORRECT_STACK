#define DEBUG
#define USE_CANARIES

#include "TXLib.h" 
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h> // TODO скачать TXLIbe
#include <stdint.h> // TODO написать богданы 

#pragma GCC diagnostic ignored "-Wredundant-tags"
#pragma GCC diagnostic ignored "-Wfree-nonheap-object"

#include "stack.h"

//TODO: files dividing
//main.cpp main.h
//stack.cpp stack.h
//error.h ...

#ifdef USE_CANARIES
    #define CANARY_PROTECT
    #define CANARY_VALUE (0x07060504030201U) // 0x07060504030201 // 0xDEADC0DEDEADC0DE
#endif

#ifdef DEBUG
    #define dbg        if(1)
    #define DBG(...) __VA_ARGS__
#else
    #define dbg        if(0)
    #define DBG(...)
#endif 


//typedef double Stack_Elem_t; 
typedef unsigned long long Stack_Elem_t;
// typedef int Stack_Elem_t;




enum Stack_Error 
{
    STACK_CTOR_NULL                  = 1,    //0x0001 
    STACK_POP_EMPTY                  = 2,    //0x0002
    STACK_PUSH_OVERFLOW              = 4,    //0x0004
    STACK_REALLOC_FAIL               = 8,    //0x0008
    STACK_INVALID_SIZE               = 16,   //0x0010
    STACK_INVALID_CANARY             = 32,   //0x0020
    STACK_INVALID_CAPACITY           = 64,   //0x0040
    STACK_CANARY_LEFT_INVALID        = 128,  //0x0080
    STACK_CANARY_RIGHT_INVALID       = 256,  //0x0100
    STACK_GIGA_CHICKEN_LEFT_INVALID  = 512,  //0x0200
    STACK_GIGA_CHICKEN_RIGHT_INVALID = 1028, //0x0400 // [x]1024
    STACK_INVALID_HASH               = 2048, //0x0800
    STACK_INVALID_HASH_BUFFER        = 4096, //0x1000
    STACK_INVALID_NULL_POINTER       = 8192  //0x2000
};

struct Stack_Information
{
    DBG (const char* name;)
    DBG (const char* file;)
    DBG (const char* function;)
    DBG (int line;)
};

struct Stack
{
    #ifdef CANARY_PROTECT
    canary_t left_canary;
    #endif

    Stack_Elem_t* data;  
    int size;      // количество элементов записывается в стек  
    int capacity; // емкость (максимальный размер буфера)
 
    //#ifdef HASH_PROTECT
    uint32_t hash; // TODO написать богдану
    uint32_t hash_buffer;
    Stack_Information info; 
    //#endif

    #ifdef CANARY_PROTECT
    canary_t right_canary;
    #endif
};



Stack_Error            stack_ctor (struct Stack* stack, int capacity DBG (, struct Stack_Information info));

void                   stack_dump (struct Stack* stack DBG (, struct Stack_Information info));

static inline uint32_t murmur_32_scramble                  (uint32_t k);
uint32_t               murmur3_32                  (const   uint8_t* key, size_t len, uint32_t seed);
uint32_t               calculate_stack_hash        (struct Stack* stack);
uint32_t               calculate_stack_hash_buffer (struct Stack* stack);

int                    stack_push (struct Stack* stack, Stack_Elem_t  elem DBG(, struct Stack_Information info));
int                    stack_pop  (struct Stack* stack, Stack_Elem_t* elem DBG(, struct Stack_Information info));

int                    stack_error      (struct Stack* stack); // верификатор 
int                    stack_assertFunc (struct Stack* stack);

int                    stack_destroy (struct Stack* stack);

void                   print_error_message (enum Stack_Error error);


#define STACK_CTOR(stack, capacity) stack_ctor (stack, capacity DBG (, (struct Stack_Information) {#stack, __FILE__, __func__, __LINE__}))
#define STACK_PUSH(stack, elem)     stack_push (stack, elem     DBG (, (struct Stack_Information) {#stack, __FILE__, __func__, __LINE__}))
#define STACK_POP(stack, elem)      stack_pop  (stack, elem     DBG (, (struct Stack_Information) {#stack, __FILE__, __func__, __LINE__}))
#define STACK_DUMP(stack)           stack_dump (stack           DBG (, (struct Stack_Information) {#stack, __FILE__, __func__, __LINE__}))

int main (void)
{
    dbg printf("\n------------------------------------------------------------------------------\n");

    struct Stack stack = {};
    $$;
    Stack_Error ded_loh = STACK_CTOR (&stack, 4);
    if (ded_loh != STACK_CTOR_NULL) // STACK_CTOR (&stack, 4) != 0
    {
        print_error_message (ded_loh);
        return ded_loh;
    }
    $$;
    printf("\nsizeof(STACK) = %zu\n", sizeof(struct Stack));

    $$;
    STACK_PUSH (&stack, 1);
    $$;
    STACK_PUSH (&stack, 2);
    $$;
    STACK_PUSH (&stack, 2);
    STACK_PUSH (&stack, 4);

    STACK_DUMP (&stack);
    //txDump(stack.data - 1);

    Stack_Elem_t x = 0;
    if (STACK_POP (&stack, &x) == 0)
        printf("Popped element: %llu\n", x);
    else
        print_error_message(STACK_POP_EMPTY);

    STACK_POP(&stack, &x);

    STACK_DUMP (&stack);

    dbg printf("\n------------------------------------------------------------------------------\n");

    stack_destroy (&stack);

    return 0;
}

void print_error_message (enum Stack_Error error)
{
    switch (error)
    {
        //TODO: WARNING all enum cases
        case STACK_CTOR_NULL:
            printf("\nStack initialization error\n");
            break;
        case STACK_POP_EMPTY:
            printf("\nError: Stack is empty, cannot pop element\n");
            break;
        case STACK_PUSH_OVERFLOW:
            printf("\nError: Stack overflow\n");
            break;
        case STACK_REALLOC_FAIL:
            printf("\nError: Failed to allocate memory\n");
            break;
        case STACK_INVALID_CANARY:
            printf("\nError: Stack canaries are corrupted\n");
            break;
        case STACK_INVALID_SIZE:
            printf("\nERROR: Size less than 0\n");
            break;
        case STACK_INVALID_CAPACITY:
            printf("\nERROR: Capacity less than 0\n");
            break;
        case STACK_CANARY_LEFT_INVALID:
            printf("\nERROR: Left canary invalid\n");
            break;
        case STACK_CANARY_RIGHT_INVALID:
            printf("\nERROR: Right canary invalid\n");
            break;
        case STACK_INVALID_NULL_POINTER:
            printf("\nERROR: stack invalid. PIZDEC occurred\n");
            break;
        default:
            printf("\nded broke stack\n");
            break;
    }
}

enum Stack_Error stack_ctor (struct Stack* stack, int capacity DBG (, struct Stack_Information info))
{
    assert(stack);

    #ifdef CANARY_VALUE
    stack->left_canary = CANARY_VALUE;
    #endif 

    DBG(stack->info = info;)

    stack->data = 2 + (Stack_Elem_t*) calloc (capacity + 2, sizeof(Stack_Elem_t));
    if (!stack -> data) 
        return STACK_REALLOC_FAIL; 
    printf("\n--------------------stack->data = %p------------------\n", stack->data);

 
    (stack->data[-1])       = giga_chicken_left; 
    (stack->data[capacity]) = giga_chicken_right;

    for (int i = 0; i < capacity; i++) 
    {
        stack->data[i] = 0xDEADBAAD; // Инициализируем "ядовитыми" значениями
    }

    dbg printf("sizeof(DATA) = %zu, sizeof(CAPACITY) = %zu\n", sizeof(stack->data), sizeof(capacity));

    stack -> capacity = capacity;
    stack -> size     = 0;

    uint32_t hash_buffer = calculate_stack_hash_buffer(stack);
    stack->hash_buffer = hash_buffer;
    printf("\nCtor: Initial Hash_buffer (hash_buffer): %08X\n", hash_buffer);

    dbg printf("\n%s(): stack->hash = %08X\n", __func__, stack->hash);
    uint32_t hash = calculate_stack_hash(stack);
    stack->hash = hash;
    dbg printf("\nCtor: Initial Hash (hash): %08X\n", hash);
    
    stack->hash = hash;
    dbg printf("%s(): stack->hash = %08X (after)\n", __func__, stack->hash);
    
    uint32_t hash_2 = calculate_stack_hash(stack);
    dbg printf("%s(): hash_2 = %08X (have to == hash)\n", __func__, hash_2);
    
    #ifdef CANARY_PROTECT
    stack -> right_canary = CANARY_VALUE; 
    #endif

    return STACK_CTOR_NULL;
}
 
void stack_dump (struct Stack* stack DBG (, struct Stack_Information info)) 
{
    dbg printf("\n_____________________\\o/___________________________________\n");

    printf("\nStack dump: size = %d, capacity = %d\n", stack -> size, stack -> capacity);

    #ifdef CANARY_PROTECT
    printf("\nLeft Canary: %llx\n", stack->data[-1]); // 0x07060504030201
    #endif

    unsigned err = stack_error(stack);
    printf("err = %016X", err);

    if (err & STACK_INVALID_NULL_POINTER)       printf("\nstack = NULL and I have to write ......\n");

    if (err & STACK_INVALID_SIZE)               printf("\nsize < 0\n");

    if (err & STACK_INVALID_CAPACITY)           printf("\ncapacity <=0 \n");

    if (err & STACK_CANARY_LEFT_INVALID)        printf("\nleft_canary  in stack (%016llX) != CANARY_VALUE (%016llX)\n", stack->left_canary,  CANARY_VALUE);
    if (err & STACK_CANARY_RIGHT_INVALID)       printf("\nright_canary in stack (%016llX) != CANARY_VALUE (%016llX)\n", stack->right_canary, CANARY_VALUE);

    if (err & STACK_GIGA_CHICKEN_LEFT_INVALID)  printf("\ngiga_chicken_left_invalid in buffer  (%016llX) != CANARY_VALUE (%016llX)\n", stack->data[-1],              CANARY_VALUE);
    if (err & STACK_GIGA_CHICKEN_RIGHT_INVALID) printf("\ngiga_chicken_right_invalid in buffer (%016llX) != CANARY_VALUE (%016llX)\n", stack->data[stack->capacity], CANARY_VALUE);
    
    DBG (printf("\nStack[0x%p] called from %s: %d (----%s----) name %s born at %s:%d\n",
            stack, info.function, info.line, info.function, stack->info.name, stack->info.file, stack->info.line));

    for (int i = 0; i < (stack->capacity); i++) // <= 
    {
        printf("%s data[%2d] = ", (i < stack->size ? " * " : "   "), i);
        
        if (stack->data[i] == 0xDEADBAAD) 
            printf("0xDEADBAAD\n"); 
        else 
            printf("%llu\n", stack->data[i]);
    }

    uint32_t hash_buffer = calculate_stack_hash_buffer(stack);
    printf("\nStack Hash_buffer: %08X", hash_buffer);

    uint32_t hash = calculate_stack_hash(stack);
    printf("\nStack Hash: %08X\n", hash);

    if (err & STACK_INVALID_HASH) printf("\nhack in stack in stack (%08X) != hash of STACK (%08X)\n", stack->hash, hash);

    #ifdef CANARY_PROTECT
    printf("\nRight Canary: %llx\n", stack->data[stack->capacity]);
    #endif


    dbg printf("\n__________________________________________________________\n");
}


int stack_push(struct Stack* stack, Stack_Elem_t elem DBG(, struct Stack_Information info)) // TODO как сделаю верификатор. переделай функцию
{
    assert(stack);

    stack_assertFunc(stack); 

    DBG(stack->info = info;)

    if ((stack->size) >= (stack->capacity)) 
    {
        int new_capacity = stack->capacity * 2;

        Stack_Elem_t* new_data = 1 + (Stack_Elem_t*) realloc (stack->data + 1, (new_capacity + 2) * sizeof(Stack_Elem_t));
        //TODO: capacity - size - заполнить Poison
        if (!new_data)
            return STACK_REALLOC_FAIL;

        (stack->data[new_capacity]) = giga_chicken_right;

        for (int i = stack->capacity; i < new_capacity; i++) 
        {
            new_data[i] = 0xDEADBAAD;  
        }

        stack->data     = new_data;
        stack->capacity = new_capacity;
    }

    stack->data[stack->size++] = elem;

    uint32_t hash_buffer = calculate_stack_hash_buffer(stack);
    stack->hash_buffer = hash_buffer;
    printf("\nPush: Current Hash_Buffer: %08X\n", hash_buffer);

    uint32_t hash = calculate_stack_hash(stack);
    stack->hash = hash;
    printf("\nPush: Current Hash: %08X\n", hash);

    stack_assertFunc(stack);

    return 0;
}

// int stack_pop(struct Stack* stack, Stack_Elem_t* elem DBG(, const char* function, int line))
int stack_pop(struct Stack* stack, Stack_Elem_t* elem DBG(, struct Stack_Information info))
{
    assert(stack);

    DBG(stack->info = info;)    

    *elem = stack -> data[--stack->size];

    if (stack->size > 0 && stack->size <= stack->capacity / 4) 
    {
        int new_capacity = stack->capacity / 2;

        Stack_Elem_t* new_data = 0 + (Stack_Elem_t*) realloc (stack->data + 1, (new_capacity + 2) * sizeof(Stack_Elem_t)); // [x] Stack_Elem_t* new_data = 1 + (Stack_Elem_t*) realloc (stack->data + 1, (new_capacity + 2) * sizeof(Stack_Elem_t));
        if (!new_data) return STACK_REALLOC_FAIL;

        (stack->data[new_capacity]) = giga_chicken_right;

        stack->data     = new_data;
        stack->capacity = new_capacity;
    }

    stack->data[stack->size] = 0xDEADBAAD;

    uint32_t hash_buffer = calculate_stack_hash_buffer(stack);
    stack->hash_buffer = hash_buffer;
    printf("\nPop: Current Hash_Buffer: %08X\n", hash_buffer);

    uint32_t hash = calculate_stack_hash(stack);
    stack->hash = hash;
    printf("\nPop: Current Hash: %08X\n", hash);

    stack_assertFunc(stack);

    return 0; 
}

int stack_error(struct Stack* stack)
{
    int err = 0;

    if (!stack) return STACK_INVALID_NULL_POINTER;

    if (stack->size < 0)     err |= STACK_INVALID_SIZE;

    if (stack->capacity < 0) err |= STACK_INVALID_CAPACITY; // [x] if (stack->capacity <= 0) err |= STACK_INVALID_CAPACITY;

    #ifdef CANARY_PROTECT
    if (stack->left_canary           != CANARY_VALUE)       err |= STACK_CANARY_LEFT_INVALID;
    if (stack->right_canary          != CANARY_VALUE)       err |= STACK_CANARY_RIGHT_INVALID;
    if (stack->data[-1]              != giga_chicken_left)  err |= STACK_GIGA_CHICKEN_LEFT_INVALID; // TODO проверить
    if (stack->data[stack->capacity] != giga_chicken_right) err |= STACK_GIGA_CHICKEN_RIGHT_INVALID; // TODO переименовать
    #endif

    uint32_t current_hash_buffer  = calculate_stack_hash_buffer (stack);
    uint32_t previous_hash_buffer = stack->hash_buffer;
    if (current_hash_buffer != previous_hash_buffer) err |= STACK_INVALID_HASH_BUFFER;
    dbg printf("\n%s(): current_hash_buffer = %08X\n", __func__, current_hash_buffer);

    uint32_t current_hash  = calculate_stack_hash (stack);
    uint32_t previous_hash = stack->hash;
    if (current_hash != previous_hash) err |= STACK_INVALID_HASH;
    dbg printf ("\n%s(): current_hush = %08X\n", __func__, current_hash);

    return err;
}

int stack_assertFunc(struct Stack* stack) // верификационный палач 
{
    int error_code = stack_error(stack);
    if (error_code)
    {

        stack_dump(stack DBG(, (struct Stack_Information) {"stack_assertFunc", __FILE__, __func__, __LINE__}));
        if (stack == NULL)
            printf("ERROR: stack = %p\n", stack);

        if ((stack->data) == NULL)
            printf("ERROR: stack->data = %p\n", (stack->data));

        if ((stack->size) > (stack->capacity))
            printf("ERROR: stack->size = %d | stack->capacity = %d\n", (stack->size), (stack->capacity));

        assert(0);
    } 

    return error_code;
}

int stack_destroy (struct Stack* stack)
{
    free (stack->data - 2);

    stack->data      = NULL;
    stack->size      = 0;    
    stack->capacity  = 0; 
    //TODO: canary destroy ?? она же и так в массиве // сделать мертвую канарейку 
    
    return 0;
}

static inline uint32_t murmur_32_scramble (uint32_t k) 
{
    k *= 0xcc9e2d51;
    k = (k << 15) | (k >> 17);
    k *= 0x1b873593;
    return k;
}

uint32_t murmur3_32 (const uint8_t* key, size_t len, uint32_t seed)
{
    uint32_t h = seed;
    uint32_t k = 0;
    /* Read in groups of 4. */
    for (size_t i = len >> 2; i; i--) 
    {
    // Вот источник различий в результатах по порядковым номерам.
    // Однако замена здесь не влияет на свойства хэш
        memcpy (&k, key, sizeof(uint32_t));
        key += sizeof(uint32_t);
        h ^= murmur_32_scramble(k);
        h = (h << 13) | (h >> 19); // g >> 4 |   00001001 << 3  01001000
        h = h * 5 + 0xe6546b64;
    }
    /* Read the rest. */
    k = 0;
    for (size_t i = len & 3; i; i--)  //0011 & 1111  = 0011
    {
        k <<= 8;
        k |= key[i - 1];
    }
    // Подкачка здесь * не* необходима, потому что предыдущий цикл уже
    // помещает младшие байты в младшие разряды в соответствии с тем порядком байтов, который мы используем
    //. Подкачки применяются только тогда, когда память копируется фрагментарно.
    h ^= murmur_32_scramble(k);

    h ^= len;
    h ^= h >> 16;
    h *= 0x85ebca6b;
    h ^= h >> 13;
    h *= 0xc2b2ae35;
    h ^= h >> 16;

  return h; 
}

// Функция для вычисления хэша стека
uint32_t calculate_stack_hash (struct Stack* stack)
{
    uint32_t old_hash = stack->hash;
    stack->hash = 0;
    uint32_t new_hash = murmur3_32 ((const uint8_t*)stack, stack->capacity * sizeof(Stack_Elem_t), 0);
    stack->hash = old_hash;

    return new_hash;
}

uint32_t calculate_stack_hash_buffer(struct Stack* stack)
{
    uint32_t old_hash_buffer = stack->hash_buffer;
    stack->hash_buffer = 0;
    uint32_t new_hash_buffer = murmur3_32 ((const uint8_t*)stack->data, stack->capacity * sizeof(Stack_Elem_t), 0);
    stack->hash_buffer = old_hash_buffer;

    return new_hash_buffer;
}