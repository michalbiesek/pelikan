#pragma once

#include <cc_define.h>
#include <stddef.h>

typedef size_t benchmark_key_u;

struct benchmark_entry {
    char *key;
    benchmark_key_u key_size;
    char *value;
    size_t value_size;
};

rstatus_i benchmark_init(size_t item_size, size_t nentries);
rstatus_i benchmark_deinit(void);
rstatus_i benchmark_put(struct benchmark_entry *e);
rstatus_i benchmark_get(struct benchmark_entry *e);
rstatus_i benchmark_rem(struct benchmark_entry *e);
