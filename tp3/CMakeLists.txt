cmake_minimum_required(VERSION 3.6)
project(tp3 C CXX)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_INCLUDE_DIR ./)
set(PROJECT_SOURCE_DIR ./)

set(GENERIC_C_FLAGS
        -Wall
        -ggdb3
)

add_definitions(-D__cpluscplus)

set(RTOS_MEMORY_SRC
        allocateurMemoire.c
        commMemoirePartagee.c
)

set(DECODER_SRC
        decodeur.c
        utils.c
        jpgd.cpp
)
set_source_files_properties(${DECODER_SRC} ${RTOS_MEMORY_SRC} PROPERTIES LANGUAGE CXX)

add_library(rtos_malloc ${RTOS_MEMORY_SRC})
add_executable(decodeur ${DECODER_SRC})
set_target_properties(decodeur PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(decodeur rtos_malloc rt pthread)
