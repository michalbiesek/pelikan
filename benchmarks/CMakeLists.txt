add_subdirectory(bench_storage)

set(MODULES
    bench_storage_slab
    slab
    time)

set(LIBS
    ccommon-static
    ${CMAKE_THREAD_LIBS_INIT})

add_executable(bench_slab bench_micro.c)

target_include_directories(bench_slab PRIVATE
        ${CMAKE_SOURCE_DIR}/src
        ${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(bench_slab ${MODULES} ${LIBS})
