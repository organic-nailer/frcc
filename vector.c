#include "vector.h"

Vector* new_vec() {
    Vector* v = malloc(sizeof(Vector));
    v->data = malloc(sizeof(void*) * 16);
    v->capacity = 16;
    v->length = 0;
    return v;
}

void vec_push(Vector* v, void* element) {
    if(v->length == v->capacity) {
        v->capacity *= 2;
        v->data = realloc(v->data, sizeof(void*) * v->capacity);
    }
    v->data[v->length++] = element;
}

void vec_push_i(Vector* v, int value) {
    vec_push(v, (void*)(intptr_t)value);
}

void* vec_pop(Vector* v) {
    return v->data[--v->length];
}

void* vec_last(Vector* v) {
    return v->data[v->length - 1];
}

bool vec_contains(Vector* v, void* element) {
    for(int i = 0; i < v->length; i++) {
        if(v->data[i] == element) {
            return true;
        }
    }
    return false;
}

bool vec_push_union(Vector* v, void* element) {
    if(vec_contains(v, element)) {
        return false;
    }
    vec_push(v, element);
    return true;
}
