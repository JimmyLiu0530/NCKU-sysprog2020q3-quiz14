#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#define GC_INITIAL_THRESHOLD 16
#define STACK_MAX 256  /* maximum stack size for the virtual machine */

typedef enum { OBJECT_TYPE_INT, OBJECT_TYPE_PAIR } ObjectType;

typedef struct sObject {
    union {
        int value;
        struct { struct sObject *head, *tail; };
    };
    
    struct sObject *next;  /* the next object chained to it */
    ObjectType type;
    unsigned char mark;    /* 'b': black, 'g': gray, 'w': white */
    
} Object;

typedef struct {
    /* GC related */
    Object *first_object;
    size_t object_num, object_max;

    size_t stack_size;
    Object *stack[STACK_MAX];
} VM;

void assertMsg(const char* message, int condition) {
  if (!condition) {
    printf("%s\n", message);
    exit(1);
  }
}

VM *new_vm() {
    VM *vm = malloc(sizeof(VM));
    assert(vm);
    if (vm) {
        vm->stack_size = 0; vm->first_object = 0; vm->object_num = 0;
        vm->object_max = GC_INITIAL_THRESHOLD;
    }
    return vm;
}

void vm_push(VM *vm, Object *value) {
    assertMsg("stack overflow", vm->stack_size < STACK_MAX);
    vm->stack[vm->stack_size++] = value;
}

Object *vm_pop(VM *vm) {
    assertMsg("stack underflow", vm->stack_size > 0);
    return vm->stack[--vm->stack_size];
}

void gc(VM *);

Object *new_object(VM *vm, ObjectType type, int i) {
    if (vm->object_num == vm->object_max)
        gc(vm);
    Object *obj = malloc(sizeof(Object));
    assert(obj);
    if (obj) {
        obj->mark = (i == 'w') ? 'w' : 'g'; /* i=='w': initial object; i=='g': objects during gc*/
        obj->type = type;
        obj->next = vm->first_object;
        vm->first_object = obj;
        ++vm->object_num;
    }
    return obj;
}

Object *vm_push_int(VM *vm, int value, char i) {
    Object *obj = new_object(vm, OBJECT_TYPE_INT, i);
    if (obj) {
        obj->value = value;
        vm_push(vm, obj);
    }
    return obj;
}

Object *vm_push_pair(VM *vm, char i) {
    Object *obj = new_object(vm, OBJECT_TYPE_PAIR, i);
    if (obj) {
        obj->tail = vm_pop(vm);
        obj->head = vm_pop(vm);
        vm_push(vm, obj);
    }
    return obj;
}

void free_vm(VM *vm) {
    vm->stack_size = 0;
    gc(vm);
    free(vm);
}

static void gc_mark(Object *obj) {
    if (obj->mark == 'b')  /* for recursive reference */
        return;    
    obj->mark = 'b';
    if (obj->type == OBJECT_TYPE_PAIR) {
        if (obj->head->mark == 'w')
            obj->head->mark = 'g';
        gc_mark(obj->head);
        if (obj->tail->mark == 'w')
            obj->tail->mark = 'g';
        gc_mark(obj->tail);  
    }
}

static void gc_mark_all(VM *vm) {
    /* add new objects during GC, and color them gray */
    vm_push_int(vm, 5, 'g'); vm_push_int(vm, 6, 'g'); 
    vm_push_pair(vm, 'g');

    for (size_t i = 0; i < vm->stack_size; ++i) {
        gc_mark(vm->stack[i]);      
    }
        
}

static void gc_sweep(VM *vm) {
    Object **obj = &(vm->first_object);
    while (*obj) {
        if ((*obj)->mark == 'w') {
            Object *unreached = *obj; 
            *obj = unreached->next;
            free(unreached);
            --vm->object_num;
        } else {
            (*obj)->mark = 'w';
            obj = &(*obj)->next;
        }
    }
}

void gc(VM *vm) {
    gc_mark_all(vm);
    gc_sweep(vm);
    /* adjust GC threshold */
    vm->object_max = vm->object_num ? vm->object_num * 2 : GC_INITIAL_THRESHOLD;
}

void test1() {
    puts(__func__);;
    VM *vm = new_vm();
    vm_push_int(vm, 1, 'w'); vm_push_int(vm, 2, 'w');
    gc(vm);
    assertMsg("GC should skip preserved objects", vm->object_num == 5);
    free_vm(vm);
}

void test2() {
    puts(__func__);
    VM *vm = new_vm();
    vm_push_int(vm, 1, 'w'); vm_push_int(vm, 2, 'w');
    vm_pop(vm); vm_pop(vm);
    gc(vm);
    assertMsg("GC should collect unreached objects", vm->object_num == 3);
    free_vm(vm);
}

void test3() {
    puts(__func__);
    VM *vm = new_vm();
    vm_push_int(vm, 1, 'w'); vm_push_int(vm, 2, 'w');
    vm_push_pair(vm, 'w');
    vm_push_int(vm, 3, 'w'); vm_push_int(vm, 4, 'w');
    vm_push_pair(vm, 'w');
    vm_push_pair(vm, 'w');
    gc(vm);
    assertMsg("GC should reach nested objects", vm->object_num == 10);
    free_vm(vm);
}

void test4() {
    puts(__func__);
    VM *vm = new_vm();
    vm_push_int(vm, 1, 'w'); vm_push_int(vm, 2, 'w');
    Object *a = vm_push_pair(vm, 'w');
    vm_push_int(vm, 3, 'w'); vm_push_int(vm, 4, 'w');
    Object *b = vm_push_pair(vm, 'w');
    a->tail = b;
    b->tail = a;
    gc(vm);
    assertMsg("GC should deal with recursive reference", vm->object_num == 7);
    free_vm(vm);
}

int main() {
    test1();
    test2();
    test3();
    test4();
    return 0;
}