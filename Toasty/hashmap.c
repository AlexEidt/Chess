#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include "hashmap.h"

HashMap* hashmap_alloc(int size) {
    HashMap* hashmap = (HashMap*) malloc(sizeof(HashMap));
    hashmap->size = 1 << size;
    hashmap->data = (Item*) calloc(hashmap->size, sizeof(Item));
    return hashmap;
}

void hashmap_free(HashMap* hashmap) {
    free(hashmap->data);
    free(hashmap);
}

void hashmap_set(HashMap* hashmap, uint64_t key, int value, int depth) {
    Item* item = &hashmap->data[key & (hashmap->size - 1)];
    if (depth >= item->depth) {
        item->key = key;
        item->value = value;
        item->depth = depth;
    }
}

bool hashmap_get(HashMap* hashmap, uint64_t key, int depth, int* ret) {
    Item* found = &hashmap->data[key & (hashmap->size - 1)];
    if (depth >= found->depth && key == found->key) {
        *ret = found->value;
        return true;
    }
    return false;
}

void hashmap_clear(HashMap* hashmap) {
    memset(hashmap->data, 0, hashmap->size * sizeof(Item));
}