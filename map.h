#ifndef MAP_H
#define MAP_H

#include "vector.h"
#include <string.h>

typedef struct {
    Vector *keys;
    Vector *vals;
} Map;

Map *new_map(void);
void map_put(Map *map, char *key, void *value);
void map_put_i(Map *map, char *key, int value);
void *map_get(Map *map, char *key);
int map_get_i(Map *map, char *key, int default_);
bool map_exists(Map *map, char *key);

#endif
