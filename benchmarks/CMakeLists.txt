add_executable(bench_micro bench_micro.c)

add_executable(bench_slab bench_slab.c)

target_include_directories(bench_micro PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(bench_micro cuckoo time)
target_link_libraries(bench_micro ccommon-static)
target_link_libraries(bench_micro pthread)

target_include_directories(bench_slab PRIVATE
	${CMAKE_SOURCE_DIR}/src
	${CMAKE_SOURCE_DIR}/tests)

target_link_libraries(bench_slab slab time)
target_link_libraries(bench_slab ccommon-static)
target_link_libraries(bench_slab pthread)
