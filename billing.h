#ifndef BILLING
#define BILLING

#define BILL_CENTS_PER_MILLISECOND (5)

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
// licence_ht_t licence_ht;

void start_billing(ht_t *licence_ht, char* plate);
float end_billing(ht_t *licence_ht, char* plate);
size_t bernstein_hash_function(char *s);
void print_hash_table(licence_ht_t *h);
size_t index_hash_table(licence_ht_t *h, char *licence);
bool initialise_new_hash_table(licence_ht_t *h, size_t n);
licence_t *add_to_licence_hash_table_bucket(licence_ht_t *h, char *key);
bool add_to_licence_hash_table(licence_ht_t *h, char *licence);
FILE *create_billing_file(char *filename);
bool write_to_billing_file(FILE *file_ptr, char *licence, float bill_total);
bool import_car_plates(char *fn, licence_ht_t *ht);


#endif