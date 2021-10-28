#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//define constants
#define CAPACITY 64
#define MULTIPLIER (37)


//struct for a hash table entry
typedef struct {
    const char* key;
    void* value;
} ht_entry;

typedef struct {
    ht_entry** entries;
    size_t capacity;
    size_t length;
} ht;

void destroy_hash_table(ht* table);

// creates an empty hash table with capacity CAPACITY
ht* create_hashtable();

unsigned long hash_key(const char *s);

int insert(ht* table, const char *key, void* value);

ht_entry* get(ht* table, const char *key);
