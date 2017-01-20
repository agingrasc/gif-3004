cmake_minimum_required(VERSION 3.6)
project(tp1)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_EXE_LINKER_FLAGS "-pg")

set(SOURCE_FILES src/setr_labo1.cpp)
set_source_files_properties(${SOURCE_FILES} PROPERTIES COMPILE_FLAGS "-Wall -pg")
add_executable(tp1 ${SOURCE_FILES})