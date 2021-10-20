#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hash.h"

int main(int argc, char* argv[]){
    //read plates in plates.txt into hash table
    ht* table = create_hashtable();
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
        ht_entry* entry = get(table, plate);
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