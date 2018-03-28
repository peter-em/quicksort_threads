cmake_minimum_required(VERSION 3.7)
project(quicksort_threads)

set(CMAKE_CXX_STANDARD 11)

set(SOURCE_FILES
        .gitignore
        ctpl_stl.h
        LICENSE
        quicksort_threads.cpp
        README.md
        timer_milisec.h)

add_executable(quicksort_threads ${SOURCE_FILES})