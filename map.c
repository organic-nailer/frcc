#include "map.h"

Map *new_map(void) {
    Map *map = malloc(sizeof(Map));
    map->keys = new_vec();
    map->vals = new_vec();
    return map;
}

void map_put(Map *map, char *key, void *value) {
    vec_push(map->keys, key);
    vec_push(map->vals, value);
}

void map_put_i(Map *map, char *key, int value) {
    map_put(map, key, (void *)(intptr_t)value);
}

void *map_get(Map *map, char *key) {
    for(int i = map->keys->length - 1; i >= 0; i--) {
        if(!strcmp(map->keys->data[i], key)) {
            return map->vals->data[i];
        }
    }
    return NULL;
}

int map_get_i(Map *map, char *key, int default_) {
    for(int i = map->keys->length - 1; i >= 0; i--) {
        if(!strcmp(map->keys->data[i], key)) {
            return (intptr_t)map->vals->data[i];
        }
    }
    return default_;
}

bool map_exists(Map *map, char *key) {
    for(int i = map->keys->length - 1; i >= 0; i--) {
        if(!strcmp(map->keys->data[i], key)) {
            return true;
        }
    }
    return false;
}
