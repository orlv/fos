#ifndef HASH_H_
#define HASH_H_

#include "types.h"
#include "synch.h"

typedef unsigned int hash_t;

typedef
struct hash_entry_s {
    hash_t hash;
    void *data;
} hash_entry_t;

#define HASH_DELETED NULL

typedef
struct hast_table_s {
    hash_entry_t *entries;
    size_t size;
    count_t count;
    mutex_t mutex;
} hash_table_t;

hash_table_t * hash_table_create(size_t initialSize);
hash_entry_t * hash_table_find(hash_table_t *hashTable, hash_t hash);
hash_entry_t * hash_table_add(hash_table_t *ht, hash_t hash, void *data);

void hash_table_free(hash_table_t *hashTable);
void hash_table_resize(hash_table_t *hashTable, size_t newSize);
void hash_table_delete(hash_table_t *hashTable, hash_t hash);

hash_t utf8_hash(char *str);

#endif /*HASH_H_*/

