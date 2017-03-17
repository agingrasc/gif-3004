cmake_minimum_required(VERSION 3.6)
project(tp3 C CXX)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_INCLUDE_DIR ./)
set(PROJECT_SOURCE_DIR ./)

set(GENERIC_C_FLAGS "-Wall -ggdb3")

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

set(COMPOSITER_SRC
        compositeur.c
        utils.c)

set(FILTER_SRC
        filtreur.c
        utils.c)

set(REDIMENSIONNEUR_SRC
        redimensionneur.c
        utils.c)

set_source_files_properties(${REDIMENSIONNEUR_SRC} ${FILTER_SRC} ${COMPOSITER_SRC} ${DECODER_SRC} ${RTOS_MEMORY_SRC} PROPERTIES LANGUAGE CXX COMPILE_FLAGS "${GENERIC_C_FLAGS} -g")


add_library(rtos_malloc ${RTOS_MEMORY_SRC})
add_executable(decodeur ${DECODER_SRC})
add_executable(compositeur ${COMPOSITER_SRC})
add_executable(filtreur ${FILTER_SRC})
add_executable(redimensionneur ${REDIMENSIONNEUR_SRC})
set_target_properties(decodeur PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(decodeur rtos_malloc rt pthread)
target_link_libraries(compositeur rtos_malloc rt pthread)
target_link_libraries(filtreur rtos_malloc rt pthread)
target_link_libraries(redimensionneur rtos_malloc rt pthread)
