#include <stdio.h>
#include <stdbool.h>

#include "PVI.h"





//This function is called when a car tries to enter the carpark. Will be used to check 
// if the car is allowed inside (i.e. if it's on the plates.txt list)
//Input: A pointer to the license plate
//Returns: A bool for whether or not the car has access
bool check_access(char* plate) {
    bool access = false;

    //Check if the plate supplied is in the table

    //If the car/plate is in the hash table, make access=true
    //Else, if the car/plate isn't in the hash table, make access=false

    return access;
}

//This function will be called whenever the car moves up or down a level, and is used 
// to keep track of what level each car is on
//Input: A pointer to the license plate
void update_level(char* plate, int level) {
    //Update the level that the car is on in the hash table 
}

//This function gets the level that a car is currently on
//Input: A pointer to the license plate
//Returns: The level of the car
int get_level(char* plate) {

}


















int main(int argc, char* argv[]){
    //read plates in plates.txt into hash table
    ht_t* table = create_hashtable();
    FILE* file = fopen("plates.txt", "r");
    if (!file){
        printf("Could not open plates.txt for reading.\n");
        return 0;
    }
    int count = 0;
    char plate[50];
    int inserted;
    while (count < CAPACITY && fscanf(file, "%s", plate) != EOF){
        printf("Read plate %s.\n", plate);
        char* thePlate = (char*)(malloc(sizeof(char) * (strlen(plate) + 1)));
        strcpy(thePlate, plate);
        inserted = insert(table, thePlate, thePlate);
        if (!inserted){
            printf("An error occured while reading plates.txt.\n");
            return 0;
        }
        count += 1;
    }
    fclose(file);
    printf("Finished reading %d plates in plates.txt.\n", count);

    //search for plate
    while (1){//loop until user enters no
        printf("Enter plate number or -1 to stop: ");
        scanf("%s", plate);
        if (!strcmp(plate, "-1")){
            break;
        }
        ht_entry_t* entry = get(table, plate);
        if (entry == NULL){
            printf("%s not found.\n", plate);
        }
        else{
            printf("%s found.\n", plate);
        }
    }
    //free memory
    destroy_hash_table(table);
}
//function definitions


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
