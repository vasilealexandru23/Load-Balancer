#include "HashTable.h"

/*
 * Functii de comparare a cheilor:
 */


int compare_function_strings(void *a, void *b)
{
	char *str_a = (char *)a;
	char *str_b = (char *)b;

	return strcmp(str_a, str_b);
}

unsigned int hash_function_servers(void *a) {
    unsigned int uint_a = *((unsigned int *)a);

    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = ((uint_a >> 16u) ^ uint_a) * 0x45d9f3b;
    uint_a = (uint_a >> 16u) ^ uint_a;
    return uint_a;
}

unsigned int hash_function_key(void *a) {
    unsigned char *puchar_a = (unsigned char *)a;
    unsigned int hash = 5381;
    int c;

    while ((c = *puchar_a++))
        hash = ((hash << 5u) + hash) + c;

    return hash;
}

/*
 * Functii de hashing:
 */
void key_val_free_function(void *data)
{
	free(((info *)data)->key);
	free(((info *)data)->value);
	free(data);
	data = NULL;
}

hashtable_t *ht_create(unsigned int hmax, unsigned int (*hash_function)(void*),
		int (*compare_function)(void*, void*),
		void (*key_val_free_function)(void*))
{
	hashtable_t *ht = malloc(sizeof(hashtable_t));
	ht->buckets = malloc(hmax * sizeof(linked_list_t *));
	for (unsigned int i = 0; i < hmax; ++i)
		ht->buckets[i] = ll_create(sizeof(info));
	ht->size = 0;
	ht->hmax = hmax;
	ht->hash_function = hash_function;
	ht->compare_function = compare_function;
	ht->key_val_free_function = key_val_free_function;
	return ht;
}

void *ht_get(hashtable_t *ht, void *key)
{
	if (ht == NULL || ht->size == 0) return NULL;
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *current = ht->buckets[index]->head;
	while (current != NULL) {
		void *current_key = ((info *)current->data)->key;
		if (!ht->compare_function(current_key, key))
			return ((info *)current->data)->value;
		current = current->next;
	}

	return NULL;
}

int ht_has_key(hashtable_t *ht, void *key)
{
	void *value = ht_get(ht, key);
	if (value) return 1;
	return 0;
}

hashtable_t *ht_put(hashtable_t *ht, void *key, unsigned int key_size,
	void *value, unsigned int value_size)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *current = ht->buckets[index]->head;
	while (current != NULL) {
		void *current_key = ((info *)current->data)->key;
		if (!ht->compare_function(current_key, key)) {
			memcpy(((info *)current->data)->value, value, value_size);
			return ht;
		}
		current = current->next;
	}
	info new_node;
	new_node.key = malloc(key_size);
	memcpy(new_node.key, key, key_size);
	new_node.value = malloc(value_size);
	memcpy(new_node.value, value, value_size);
	ll_add_nth_node(ht->buckets[index], 0, &new_node);
	ht->size++;
	// Verific daca am ajuns la 75% din capacitatea dictionarului.
	if (4 * ht->size >= 3 * ht->hmax) {
		hashtable_t *new_ht = ht_create(2 * ht->hmax, ht->hash_function,
										ht->compare_function,
										ht->key_val_free_function);
		for (unsigned int i = 0; i < ht->hmax; ++i) {
			ll_node_t *prev = NULL;
			ll_node_t *current = ht->buckets[i]->head;
			while (current != NULL) {
				info *data = ((info *)current->data);
				new_ht = ht_put(new_ht, data->key, strlen((char *)data->key) + 1,
								data->value, strlen((char *)data->value) + 1);
				free(((info *)(current->data))->key);
				free(((info *)(current->data))->value);
				prev = current;
				current = current->next;
				free(prev->data);
				free(prev);
			}
			free(ht->buckets[i]);
		}
		free(ht->buckets);
		free(ht);
		return new_ht;
	}
	return ht;
}

hashtable_t *ht_remove_entry(hashtable_t *ht, void *key)
{
	unsigned int index = ht->hash_function(key) % ht->hmax;
	ll_node_t *current = ht->buckets[index]->head;
	int location = 0;
	while (current != NULL) {
		void *current_key = ((info *)current->data)->key;
		if (!ht->compare_function(current_key, key)) {
			ll_node_t *remove = ll_remove_nth_node(ht->buckets[index], location);
			ht->key_val_free_function(remove->data);
			free(remove);
			remove = NULL;
			if (ht->buckets[index]->size == 0)
				ht->size--;
		}
		location++;
		current = current->next;
	}
	// Verific daca am ajuns la 25% din capacitatea dictionarului.
	if (ht->hmax >= 4 * ht->size) {
		hashtable_t *new_ht = ht_create(ht->hmax / 2, ht->hash_function,
						ht->compare_function,
						ht->key_val_free_function);
		for (unsigned int i = 0; i < ht->hmax; ++i) {
			ll_node_t *prev = NULL;
			ll_node_t *current = ht->buckets[i]->head;
			while (current != NULL) {
				info *data = ((info *)current->data);
				new_ht = ht_put(new_ht, data->key, strlen((char *)data->key) + 1,
						data->value, strlen((char *)data->value) + 1);
				free(((info *)(current->data))->key);
				free(((info *)(current->data))->value);
				prev = current;
				current = current->next;
				free(prev->data);
				free(prev);
			}
			free(ht->buckets[i]);
		}
		free(ht->buckets);
		free(ht);
		return new_ht;
	}
	return ht;
}

void ht_free(hashtable_t *ht)
{	
	if (!ht)
		return;
	for (unsigned int i = 0; i < ht->hmax; ++i) {
		while (ht->buckets[i]->size) {
			ll_node_t *removed = ll_remove_nth_node(ht->buckets[i], 0);
			ht->key_val_free_function(removed->data);
			free(removed);
			removed = NULL;
		}
		free(ht->buckets[i]);
	}
	free(ht->buckets);
	free(ht);
	ht = NULL;
}

