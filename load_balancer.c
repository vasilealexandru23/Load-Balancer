/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

#define MAX_HASH 100000
#define MAX_SERVERS 99999
#define MAX_HASH_RING_SIZE 1000
#define MAX_COPIES 3
#define DUPLICATE_GENERATOR 100000

load_balancer *init_load_balancer()
{
	load_balancer *main = malloc(sizeof(load_balancer));
	main->number_servers = 0;
	main->ring_size = 0;
	main->hash_ring = calloc(MAX_HASH_RING_SIZE, sizeof(hash_ring_elem));
	return main;
}

int find_new_position(load_balancer *main, int server_id)
{
	int low = 0;
	int high = main->ring_size - 1;
	unsigned int hash_server = hash_function_servers(&server_id);
	while (low < high) {
		int middle = (low + high) / 2;
		if (hash_function_servers(&(main->hash_ring[middle].new_id)) < hash_server)
			low = middle + 1;
		else
			high = middle;
	}

	if (hash_server > hash_function_servers(&(main->hash_ring[low].new_id)))
		low = (low + 1) % (main->ring_size + 1);

	return low;
}

void loader_add_server(load_balancer *main, int server_id)
{
	// Creez noul server.
	for (unsigned int copies = 0; copies < MAX_COPIES; ++copies) {
		int new_id = copies * DUPLICATE_GENERATOR + server_id;

		int position = find_new_position(main, new_id);

		main->ring_size++;

		for (int i = main->ring_size - 1; i > position; --i)
			main->hash_ring[i] = main->hash_ring[i - 1];

		main->hash_ring[position].new_id = new_id;
		main->hash_ring[position].origin_server_id = server_id;
		main->hash_ring[position].server = init_server_memory();

		if (main->ring_size != 1) {
			int next_server = (position + 1) % main->ring_size;
			// Din serverul urmator elimin fiecare chestie din server si adaug iar eventual.
			hashtable_t *server_data = main->hash_ring[next_server].server->storage;
			for (int i = 0; i < HMAX; ++i) {
				unsigned int index_to_erase = 0;
				while (index_to_erase < server_data->buckets[i]->size) {
					ll_node_t *to_remove = ll_get_nth_node(server_data->buckets[i], index_to_erase);
					char *copie = calloc(strlen((char *)((info *)to_remove->data)->key) + 2, sizeof(char));
					strcpy(copie, (char *)((info *)to_remove->data)->key);
					char *copie2 = calloc(strlen((char *)((info *)to_remove->data)->value) + 2, sizeof(char));
					strcpy(copie2, (char *)((info *)to_remove->data)->value);
					server_data = ht_remove_entry(server_data, ((info *)to_remove->data)->key);
					int trash = 0;
					loader_store(main, copie, copie2, &trash);
					free(copie);
					free(copie2);
					if (trash == next_server)
						index_to_erase++;
				}
			}
		}
	}
}

void loader_remove_server(load_balancer *main, int server_id)
{
	for (unsigned int copies = 0; copies < MAX_COPIES; ++copies) {
		int new_id = copies * 100000 + server_id;

		int position = find_new_position(main, new_id);

		if (main->hash_ring[position].origin_server_id != (unsigned int)server_id)
			continue;

		hash_ring_elem aux = main->hash_ring[position];

		// Le mut pe toate la dreapta.
		for (int i = position; i < (int)main->ring_size - 1; ++i) {
			main->hash_ring[i] = main->hash_ring[i + 1];
		}

		main->hash_ring[main->ring_size - 1].server = NULL;

		// get next valid server.
		main->ring_size--;
		if (main->ring_size != 0) {
			hashtable_t *to_erase_server = aux.server->storage;
			for (int i = 0; i < HMAX; ++i) {
				while (to_erase_server->buckets[i]->size) {
					ll_node_t *removed = ll_remove_nth_node(to_erase_server->buckets[i], 0);
					int trash = 0;
					loader_store(main, ((info *)removed->data)->key, ((info *)removed->data)->value, &trash);
					free(((info *)removed->data)->key);
					free(((info *)removed->data)->value);
					free(removed->data);
					free(removed);
					removed = NULL;
				}
			}	
			ht_free(to_erase_server);	
			free(aux.server);
			aux.server = NULL;
		}
	}
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	int lower_bound = 0;
	int upper_bound = main->ring_size - 1;
	while (lower_bound < upper_bound) {
		int middle = (lower_bound + upper_bound) / 2;
		if (hash_function_servers(&(main->hash_ring[middle].new_id)) < hash_function_key(key))
			lower_bound = middle + 1;
		else
			upper_bound = middle;
	}
	if (hash_function_servers(&(main->hash_ring[lower_bound].new_id)) < hash_function_key(key))
		lower_bound = (lower_bound + 1) % main->ring_size;
	*server_id = lower_bound;
	server_store(main->hash_ring[lower_bound].server, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	int lower_bound = 0;
	int upper_bound = main->ring_size - 1;
	while (lower_bound < upper_bound) {
		int middle = (lower_bound + upper_bound) / 2;
		if (hash_function_servers(&(main->hash_ring[middle].new_id)) < hash_function_key(key))
			lower_bound = middle + 1;
		else
			upper_bound = middle;
	}
	if (hash_function_servers(&(main->hash_ring[lower_bound].new_id)) < hash_function_key(key))
		lower_bound = (lower_bound + 1) % main->ring_size;
	*server_id = main->hash_ring[lower_bound].origin_server_id;
	return ht_get(main->hash_ring[lower_bound].server->storage, key);
}

void free_load_balancer(load_balancer *main) {
	for (int i = 0; i < MAX_HASH_RING_SIZE; ++i) {
		if (main->hash_ring[i].server != NULL) {
			ht_free(main->hash_ring[i].server->storage);
			free(main->hash_ring[i].server);
			main->hash_ring[i].server = NULL;
		}
	}
	free(main->hash_ring);
	free(main);
	main = NULL;
}

