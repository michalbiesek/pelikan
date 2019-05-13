#add_library(bench_storage_slab storage_slab.c)
#target_sources(bench_storage_slab PRIVATE storage_slab.c)

#add_library(bench_storage_cuckoo storage.h)
#target_sources(bench_storage_cuckoo PRIVATE storage_cuckoo.c)


set(SOURCE
    storage_slab.c)

add_library(bench_storage_slab ${SOURCE})
target_link_libraries(bench_storage_slab slab)
