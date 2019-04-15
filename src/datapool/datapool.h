#pragma once

#include <stddef.h>
#include <inttypes.h>
struct datapool;

struct datapool *datapool_open(const char *path, size_t size,
	int *fresh);
void datapool_close(struct datapool *pool);

void *datapool_addr(struct datapool *pool);
size_t datapool_size(struct datapool *pool);
void * datapool_alloc(struct datapool *pool, size_t size);
void datapool_set_nslab(struct datapool *pool, uint64_t nslab);
uint64_t datapool_get_nslab(struct datapool *pool);
