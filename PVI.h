#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//define constants
#define CAPACITY 64
#define MULTIPLIER (37)


//struct for a hash table entry
typedef struct ht_entry {
    const char* key;
    void* value;
    int level;
    time_t billStart; //the time that billing was started
    time_t billEnd; //the time that billing was stopped ()
} ht_entry_t;

typedef struct ht {
    ht_entry_t** entries;
    size_t capacity;
    size_t length;
} ht_t;

void destroy_hash_table(ht_t* table);

// creates an empty hash table with capacity CAPACITY
ht_t* create_hashtable();

unsigned long hash_key(const char *s);

int insert(ht_t* table, const char *key, void* value);

ht_entry_t* get(ht_t* table, const char *key);
