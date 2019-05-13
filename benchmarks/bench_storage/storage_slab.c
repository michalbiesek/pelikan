#include <storage/slab/item.h>
#include <storage/slab/slab.h>

static rstatus_i
benchmark_deinit(void)
{
    slab_teardown();
    return CC_OK;
}
