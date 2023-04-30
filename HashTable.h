#ifndef __HASHTABLE_H__
#define __HASHTABLE_H__
#define MAX_STRING_SIZE	256
#define HMAX 1
#include "LinkedList.h"
typedef struct info info;
struct info {
	void *key;
	void *value;
};

typedef struct hashtable_t hashtable_t;
struct hashtable_t {
	linked_list_t **buckets; /* Array de liste simplu-inlantuite. */
	/* Nr. total de noduri existente curent in toate bucket-urile. */
	unsigned int size;
	unsigned int hmax; /* Nr. de bucket-uri. */
	/* (Pointer la) Functie pentru a calcula valoarea hash asociata cheilor. */
	unsigned int (*hash_function)(void*);
	/* (Pointer la) Functie pentru a compara doua chei. */
	int (*compare_function)(void*, void*);
	/* (Pointer la) Functie pentru a elibera memoria ocupata de cheie si valoare. */
	void (*key_val_free_function)(void*);
};

int compare_function_ints(void *a, void *b);

int compare_function_strings(void *a, void *b);

unsigned int hash_function_servers(void *a);

unsigned int hash_function_key(void *a);

void key_val_free_function(void *data);

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		               int (*compare_function)(void*, void*),
		               void (*key_val_free_function)(void*));

void *ht_get(hashtable_t *ht, void *key);

int ht_has_key(hashtable_t *ht, void *key);

hashtable_t *ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	                void *value, unsigned int value_size);

hashtable_t *ht_remove_entry(hashtable_t *ht, void *key);

void ht_free(hashtable_t *ht);

#endif // __HASHTABLE_H__
