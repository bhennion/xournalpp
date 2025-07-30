cmake_minimum_required(VERSION 3.10)


file(GLOB benchmark_rendering_sources stroke-rendering/*.cpp stroke-rendering/*.h)
add_executable(benchmark_rendering EXCLUDE_FROM_ALL ${benchmark_rendering_sources})
# Link the target to the GTK+ libraries
target_link_libraries(benchmark_rendering PRIVATE ${GTK3_LIBRARIES} xoj::core xoj::util)
