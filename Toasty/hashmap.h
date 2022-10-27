#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdint.h>
#include <stdbool.h>

typedef struct {
    uint64_t key;
    int value;
    int depth;
} Item;

typedef struct {
    int size;
    Item* data;
} HashMap;

HashMap* hashmap_alloc(int size);
void hashmap_free(HashMap* hashmap);

void hashmap_set(HashMap* hashmap, uint64_t key, int value, int depth);
bool hashmap_get(HashMap* hashmap, uint64_t key, int depth, int* ret);
void hashmap_clear(HashMap* hashmap);

#endif