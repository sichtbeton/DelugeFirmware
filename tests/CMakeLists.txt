cmake_minimum_required(VERSION 3.24)
project(DelugeTests)

enable_testing()
#needs to support -m32 since the memory tests assume a 32 bit architecture
if (UNIX AND NOT APPLE)
    add_compile_options(-m32 -Og -ggdb3)
    add_link_options(-m32)
    add_subdirectory(32bit_unit_tests)
endif ()
add_subdirectory(spec)
add_subdirectory(unit)

