/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>

#include "load_balancer.h"

#define MAX_COPIES 3
#define DUPLICATE_GENERATOR 100000

load_balancer *init_load_balancer()
{
	/* Alloc and initialize the load balancer. */
	load_balancer *main_server = malloc(sizeof(load_balancer));
	DIE(!main_server, "Malloc load_balancer failed!");

	main_server->number_servers = 0;
	main_server->ring_size = 0;
	main_server->ring_capacity = 1;

	main_server->hash_ring = calloc(1, sizeof(hash_ring_elem));
	DIE(!main_server->hash_ring, "Calloc hashring failed!");

	return main_server;
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
	/* Check if the load_balancer is empty. */
	if (!main->ring_size)
		return 0;

	int low = 0;
	int high = main->ring_size - 1;
	while (low < high) {
		int middle = (low + high) / 2;
		if (compare_function(&(main->hash_ring[middle].new_id), data))
			low = middle + 1;
		else
			high = middle;
	}

	int overflow = (compare_function == compare_with_server);
	if (compare_function(&(main->hash_ring[low].new_id), data))
		low = (low + 1) % (main->ring_size + overflow);

	return low;
}

void check_realloc(load_balancer *main)
{
	/* Find new size for realloc(if it's the case). */
	unsigned int new_size = 0;

	if (4 * main->ring_size >= 3 * main->ring_capacity) {
		main->ring_capacity *= 2;
		new_size = main->ring_capacity;
	} else if (main->ring_capacity >= 4 * main->ring_size) {
		main->ring_capacity /= 2;
		new_size = main->ring_capacity;
	}

	/* Check if we need to resize the hashring. */
	if (!new_size)
		return;

	/* Do the resize. */
	hash_ring_elem *tmp =
	    realloc(main->hash_ring, new_size * sizeof(hash_ring_elem));
	DIE(!tmp, "Realloc hashring failed!");
	main->hash_ring = tmp;
}

void loader_add_server(load_balancer *main, int server_id)
{
	/* Create the server with all the duplicates. */
	for (unsigned int copies = 0; copies < MAX_COPIES; ++copies) {
		int new_id = copies * DUPLICATE_GENERATOR + server_id;

		/* Find the position where to insert the new server in hashring. */
		int position = find_new_position(main, &new_id, compare_with_server);

		/* Increase number of elements on the hashring. */
		main->ring_size++;
		check_realloc(main);

		/* Make space for the new server(duplicate). */
		for (int i = main->ring_size - 1; i > position; --i)
			main->hash_ring[i] = main->hash_ring[i - 1];

		/* Initialize info in hashring. */
		main->hash_ring[position].new_id = new_id;
		main->hash_ring[position].origin_server_id = server_id;
		main->hash_ring[position].server = init_server_memory();

		if (main->ring_size != 1) {
			int next_server = (position + 1) % main->ring_size;
			/* Redistribute all items from the following server. */
			hashtable_t *server_data =
			    main->hash_ring[next_server].server->storage;

			for (unsigned int i = 0; i < server_data->hmax; ++i) {
				/* Redistribute all items from the current bucket. */
				unsigned int index_to_erase = 0;
				while (index_to_erase < server_data->buckets[i]->size) {
					ll_node_t *to_remove = ll_get_nth_node
						(server_data->buckets[i], index_to_erase);

					/* Extract data from current node. */
					char *key = (char *)((info *)to_remove->data)->key;
					char *value = (char *)((info *)to_remove->data)->value;

					/* Make copies, data might be erased from memory. */
					char *copy_key = calloc(strlen(key) + 1, sizeof(char));
					strcpy(copy_key, (char *)((info *)to_remove->data)->key);
					char *copy_value = calloc(strlen(value) + 1, sizeof(char));
					strcpy(copy_value, (char *)((info *)to_remove->data)->value);

					/* Erase node from current server and redistribute it. */
					ht_remove_entry(server_data, copy_key);
					int new_server_parent = 0;
					loader_store(main, copy_key, copy_value, &new_server_parent);

					/* Erase aux variables from memory. */
					free(copy_key);
					free(copy_value);

					/* Element's new server parent is the same as the old. */
					if (new_server_parent == next_server)
						index_to_erase++;
				}
			}
			/* Check for resize. */
			main->hash_ring[next_server].server->storage =
			resize(main->hash_ring[next_server].server->storage);
		}
	}
}

void loader_remove_server(load_balancer *main, int server_id)
{
	/* Remove the server with all the duplicates. */
	for (unsigned int copies = 0; copies < MAX_COPIES; ++copies) {
		int new_id = copies * DUPLICATE_GENERATOR + server_id;

		/* Find the position of the duplicate on hashring. */
		int position = find_new_position(main, &new_id, compare_with_server);

		/* Track the server to be removed. */
		hash_ring_elem to_erase_elem = main->hash_ring[position];

		/* Clear space. */
		for (int i = position; i < (int)main->ring_size - 1; ++i)
			main->hash_ring[i] = main->hash_ring[i + 1];

		/* Erase last element. */
		main->hash_ring[main->ring_size - 1].server = NULL;

		/* Redistribute all items from the server to */
		/* be erased if we still have servers in loader. */
		main->ring_size--;
		check_realloc(main);
		if (main->ring_size != 0) {
			hashtable_t *to_erase_server = to_erase_elem.server->storage;
			for (unsigned int i = 0; i < to_erase_server->hmax; ++i) {
				while (to_erase_server->buckets[i]->size) {
					ll_node_t *removed = ll_remove_nth_node(to_erase_server->buckets[i], 0);
					int get_server = 0;

					/* Get pair <key, value> from node. */
					char *key = ((info *)removed->data)->key;
					char *value = ((info *)removed->data)->value;

					/* Redistribute the pair in loader. */
					loader_store(main, key, value, &get_server);

					/* Free memory. */
					to_erase_server->key_val_free_function(removed->data);
					free(removed);
					removed = NULL;
				}
			}
			/* Erase server from memory. */
			ht_free(to_erase_server);
			free(to_erase_elem.server);
			to_erase_elem.server = NULL;
		}
	}
}

void loader_store(load_balancer *main, char *key, char *value, int *server_id)
{
	/* Add new pair <key, value> to the load balancer. */
	*server_id = find_new_position(main, key, compare_with_key);
	server_store(main->hash_ring[*server_id].server, key, value);
}

char *loader_retrieve(load_balancer *main, char *key, int *server_id)
{
	/* Retrieve value from pair <key, value> from the load balancer. */
	int position = find_new_position(main, key, compare_with_key);
	*server_id = main->hash_ring[position].origin_server_id;
	return ht_get(main->hash_ring[position].server->storage, key);
}

void free_load_balancer(load_balancer *main)
{
	/* Iterate and erase all memory from all servers. */
	for (unsigned int i = 0; i < main->ring_size; ++i) {
		if (main->hash_ring[i].server) {
			ht_free(main->hash_ring[i].server->storage);
			free(main->hash_ring[i].server);
			main->hash_ring[i].server = NULL;
		}
	}
	free(main->hash_ring);
	free(main);
	main = NULL;
}
