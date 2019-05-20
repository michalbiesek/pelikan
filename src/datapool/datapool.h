#pragma once

#include <stddef.h>

struct datapool;

struct datapool *datapool_open(const char *path, size_t size);
void datapool_close(struct datapool *pool);

void *datapool_addr(struct datapool *pool);
size_t datapool_size(struct datapool *pool);
int datapool_get_fresh_state(struct datapool *pool);
ptrdiff_t datapool_get_offset(struct datapool *pool);
