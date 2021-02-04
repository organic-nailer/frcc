#ifndef VECTOR_H
#define VECTOR_H

#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct {
    void **data;
    int capacity;
    int length;
} Vector;

Vector* new_vec();
void vec_push(Vector* v, void *element);
void vec_push_i(Vector* v, int value);
void* vec_pop(Vector* v);
void* vec_last(Vector* v);
bool vec_contains(Vector* v, void* element);
bool vec_push_union(Vector* v, void* element);

#endif
