
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <omp.h>
typedef struct licence licence_t;
struct licence
{
    char *licence;
    licence_t *next;
};
typedef struct licence_ht licence_ht_t;
struct licence_ht
{
    licence_t **buckets;
    size_t size;
};
licence_ht_t licence_ht;


size_t bernstein_hash_function(char *s)
{
    size_t hash = 5381;
    int c;
    while ((c = *s++) != '\0')
    {
        // hash = hash * 33 + c
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}


void print_hash_table(licence_ht_t *h)
{
    printf("hash table with %ld buckets\n", h->size);
    for (size_t i = 0; i < h->size; ++i)
    {
        printf("bucket %ld: ", i);
        if (h->buckets[i] == NULL)
        {
            printf("empty\n");
        }
        else
        {
            for (licence_t *j = h->buckets[i]; j != NULL; j = j->next)
            {
                printf("licence = %s", j->licence);
                if (j->next != NULL)
                {
                    printf(" -> ");
                }
            }
            printf("\n");
        }
    }
}


size_t index_hash_table(licence_ht_t *h, char *licence)
{
    return bernstein_hash_function(licence) % h->size;
}


bool initialise_new_hash_table(licence_ht_t *h, size_t n)
{
    h->size = n;
    h->buckets = (licence_t **)calloc(n, sizeof(licence_t *));
    for (int i=0; i<h->size; i++)
    {
        h->buckets[i] = NULL;
    }
    return true;
}

licence_t *add_to_licence_hash_table_bucket(licence_ht_t *h, char *key)
{
    
    size_t index = index_hash_table(h, key);
    return h->buckets[index];
}


bool add_to_licence_hash_table(licence_ht_t *h, char *licence)
{
    
    licence_t *head = add_to_licence_hash_table_bucket(h,licence);
    if (head != NULL){
        while (head->next != NULL) {
            head = head->next;
        }
        licence_t *new = (licence_t *)malloc(sizeof(licence_t));
        //int index = index_hash_table(h, licence);
        new->licence = licence;
        new->next = NULL;
        head->next = new;
        return true;
    }
    licence_t *new = (licence_t *)malloc(sizeof(licence_t));
    int index = index_hash_table(h, licence);
    new->licence = licence;
    new->next = NULL;
    h->buckets[index] = new;
    return true;
}


FILE *create_billing_file(char *filename)
{
    FILE *file_ptr = fopen(filename, "a");
    return file_ptr;
}

bool write_to_billing_file(FILE *file_ptr, char *licence, double price)
{
    fprintf(file_ptr, "%s $%.2f\n", licence, price);
    return true;
}


bool import_car_plates(char *fn, licence_ht_t *ht)
{
    // initialise hashtable
    size_t buckets = 10;
    if (!initialise_new_hash_table(ht, buckets)){
        return false;
    }

    int licence_length = 7;
    char *buffer = (char *)malloc(sizeof(char)*licence_length);;
    FILE *file = fopen(fn, "r");
    int i;
    if (file)
    {
        i = fscanf(file, "%s", buffer);
        while (i>0)
        {
            add_to_licence_hash_table(ht, buffer);
            buffer = (char *)malloc(sizeof(char)*licence_length);
            i = fscanf(file, "%s", buffer);
        }
        fclose(file);
    }
    return true;
}


int main(void)
{
    // Import licence plates
    if(!import_car_plates("plates.txt",&licence_ht)){
        printf("Licence import failure exiting safely");
        return 1;
    } else{
        printf("Licence imported safely");
        print_hash_table(&licence_ht);
    }

}