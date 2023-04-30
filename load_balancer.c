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
	// Alloc and initialize the load balancer.
	load_balancer *main = malloc(sizeof(load_balancer));
	DIE(!main, "Malloc load_balancer failed!");
	main->number_servers = 0;
	main->ring_size = 0;
	main->hash_ring = calloc(MAX_HASH_RING_SIZE, sizeof(hash_ring_elem));
	DIE(!main->hash_ring, "Malloc hashring failed!");
	return main;
}

unsigned int compare_with_server(void *a, void *b)
{
	return (hash_function_servers(a) < hash_function_servers(b));
}

unsigned int compare_with_key(void *a, void *b)
{
	return (hash_function_servers(a) < hash_function_key(b));
}

int find_new_position(load_balancer *main, void *data,
		unsigned int (*compare_function)(void *, void *))
{
	int low = 0;
	int high = main->ring_size - 1;
	while (low < high) {
		int middle = (low + high) / 2;
		if (compare_function(&(main->hash_ring[middle].new_id), data))
			low = middle + 1;
		else
			high = middle;
	}

	if (compare_function(&(main->hash_ring[low].new_id), data))
		low = (low + 1) % (main->ring_size + (compare_function == compare_with_server));

	return low;
}

void loader_add_server(load_balancer *main, int server_id)
{
	// Create the server with all the duplicates.
	for (unsigned int copies = 0; copies < MAX_COPIES; ++copies) {
		int new_id = copies * DUPLICATE_GENERATOR + server_id;

		// Find the position where to insert the new server in hashring.
		int position = find_new_position(main, &new_id, compare_with_server);

		main->ring_size++;

		// Make space for the new server(duplicate).
		for (int i = main->ring_size - 1; i > position; --i)
			main->hash_ring[i] = main->hash_ring[i - 1];

		// Initialize info in hashring.
		main->hash_ring[position].new_id = new_id;
		main->hash_ring[position].origin_server_id = server_id;
		main->hash_ring[position].server = init_server_memory();

		if (main->ring_size != 1) {
			int next_server = (position + 1) % main->ring_size;
			// Redistribute all items from the following server.
			hashtable_t *server_data = main->hash_ring[next_server].server->storage;
			for (int i = 0; i < HMAX; ++i) {
				unsigned int index_to_erase = 0;
				while (index_to_erase < server_data->buckets[i]->size) {
					ll_node_t *to_remove = ll_get_nth_node(server_data->buckets[i], index_to_erase);
					if (to_remove == NULL) break;
					char *copie = calloc(strlen((char *)((info *)to_remove->data)->key) + 1, sizeof(char));
					strcpy(copie, (char *)((info *)to_remove->data)->key);
					char *copie2 = calloc(strlen((char *)((info *)to_remove->data)->value) + 1, sizeof(char));
					strcpy(copie2, (char *)((info *)to_remove->data)->value);
					server_data = ht_remove_entry(server_data, copie);
					int trash = 0;
					loader_store(main, copie, copie2, &trash);
					free(to_remove);
					to_remove = NULL;
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
	// Remove the server with all the duplicates.
	for (unsigned int copies = 0; copies < MAX_COPIES; ++copies) {
		int new_id = copies * 100000 + server_id;

		// Find the position of the duplicate on hashring.
		int position = find_new_position(main, &new_id, compare_with_server);

		// Track the server to be removed.
		hash_ring_elem to_erase_elem = main->hash_ring[position];

		// Clear space.
		for (int i = position; i < (int)main->ring_size - 1; ++i)
			main->hash_ring[i] = main->hash_ring[i + 1];

		// Erase last element.
		main->hash_ring[main->ring_size - 1].server = NULL;

		// Redistribute all items from the server to
		// be erased if we still have servers in loader.
		main->ring_size--;
		if (main->ring_size != 0) {
			hashtable_t *to_erase_server = to_erase_elem.server->storage;
			for (int i = 0; i < HMAX; ++i) {
				while (to_erase_server->buckets[i]->size) {
					ll_node_t *removed = ll_remove_nth_node(to_erase_server->buckets[i], 0);
					int trash = 0;
					loader_store(main, ((info *)removed->data)->key, ((info *)removed->data)->value, &trash);
					to_erase_server->key_val_free_function(removed->data);
					free(removed);
					removed = NULL;
				}
			}	
			// Erase server from memory.
			ht_free(to_erase_server);	
			free(to_erase_elem.server);
			to_erase_elem.server = NULL;
		}
	}
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	*server_id = find_new_position(main, key, compare_with_key);
	server_store(main->hash_ring[*server_id].server, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	int position = find_new_position(main, key, compare_with_key);
	*server_id = main->hash_ring[position].origin_server_id;
	return ht_get(main->hash_ring[position].server->storage, key);
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

