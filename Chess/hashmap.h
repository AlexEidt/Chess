#ifndef HASHMAP_H_
#define HASHMAP_H_

#include <stdint.h>
#include <stdbool.h>

#define BOUND_EXACT 1
#define BOUND_UPPER 2
#define BOUND_LOWER 3

#define KEY_OFFSET 20

typedef struct {
    uint64_t key;
    int value;
    int depth;
    int flag;
} Item;

typedef struct {
    int size;
    Item* data;
} HashMap;

HashMap* hashmap_alloc(int size);
void hashmap_free(HashMap* hashmap);
void hashmap_clear(HashMap* hashmap);

void hashmap_set(HashMap* hashmap, uint64_t key, int value, int depth, int flag);
int hashmap_get(HashMap* hashmap, uint64_t key, int depth, int* ret);

#endif