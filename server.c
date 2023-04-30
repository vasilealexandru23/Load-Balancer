/* Copyright 2023 <> */
#include <stdlib.h>
#include <string.h>
#include "server.h"

server_memory *init_server_memory()
{
	server_memory *server = malloc(sizeof(server_memory));
	DIE(!server, "Malloc server_memory failed!");
	server->storage = ht_create(HMAX, hash_function_key,
		compare_function_strings, key_val_free_function);	
	return server;
}

void server_store(server_memory *server, char *key, char *value) {
	server->storage = ht_put(server->storage, key, strlen(key) + 1, value, strlen(value) + 1);
}

char *server_retrieve(server_memory *server, char *key) {
	return (char *)ht_get(server->storage, key);
}

void server_remove(server_memory *server, char *key) {
	server->storage = ht_remove_entry(server->storage, key);
}

void free_server_memory(server_memory *server) {
	ht_free(server->storage);
	free(server);
}

