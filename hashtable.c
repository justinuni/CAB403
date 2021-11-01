#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "hashtable.h"

void destroy_hash_table(ht_t* table){
    //delete the entries
    for (int i = 0; i < table->length; i++){
        free(table->entries[i]);
    }
    //delete the hash table itself
    free(table);
}

// creates an empty hash table with capacity CAPACITY
ht_t* create_hashtable(){
    ht_t* hash_table = (ht_t*)malloc(sizeof(ht_t));
    hash_table->capacity = CAPACITY;
    hash_table->length = 0;
    hash_table->entries = (ht_entry_t**)malloc(sizeof(ht_entry_t*) * hash_table->capacity);
    //initialize all entries to NULL
    for (size_t i = 0; i < hash_table->capacity; i++){
        hash_table->entries[i] = NULL;
    }
    return hash_table;
}

unsigned long hash_key(const char *s){
    
    unsigned long h;
    unsigned const char *us;

    /* cast s to unsigned const char * */
    /* this ensures that elements of s will be treated as having values >= 0 */
    us = (unsigned const char *) s;

    h = 0;
    while(*us != '\0') {
        h = h * MULTIPLIER + *us;
        us++;
    } 

    return h;
}

int insert(ht_t* table, const char *key, void* value){
    //check that we have space in the table
    if (table->length >= table->capacity){
        return 0; //false
    }
    long index = hash_key(key) % table->capacity;
    ht_entry_t* entry = malloc(sizeof(ht_entry_t));
    entry->key = key;
    entry->value = value;
    //use linear probing
    while (table->entries[index] != NULL){
        
        //wrap around
        if (index >= table->capacity){
            index = 0;
        }
        else{
            index += 1;
        }
    }
    //insert into hash table
    table->entries[index] = entry;
    table->length += 1;
    return 1;//true
}

ht_entry_t* get(ht_t* table, const char *key){
    long index = hash_key(key) % table->capacity;
    if (table->entries[index] == NULL){
        return NULL; //there's nothing there yet
    }
    //check for value. If not match, use linear probing until NULL or value found, whichever comes first
    while (table->entries[index] != NULL){
        if (strcmp(table->entries[index]->key, key ) == 0){
            return table->entries[index];
        }
        else{
            if (index >= table->capacity){
                index = 0;
            }
            else{
                index += 1;
            }
        }
    }
    return NULL;
}