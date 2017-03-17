cmake_minimum_required(VERSION 3.6)
project(tp3 C CXX)

set(CMAKE_CXX_STANDARD 11)

set(PROJECT_INCLUDE_DIR ./)
set(PROJECT_SOURCE_DIR ./)

set(GENERIC_C_FLAGS "-Wall -ggdb3")
#set(OPTIMISATION_FLAGS "-march=armv8-a -mtune=cortex-a53 -mfpu=crypto-neon-fp-armv8 -mfloat-abi=hard -ftree-vectorize -funsafe-math-optimizations -Ofast -funroll-loops -floop-parallelize-all -floop-block -flto")
set(OPTIMISATION_FLAGS "")

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

set(GRAYSCALER_SRC
        grayscaler.c
        utils.c)

set_source_files_properties(${GRAYSCALER_SRC} ${REDIMENSIONNEUR_SRC} ${FILTER_SRC} ${COMPOSITER_SRC} ${DECODER_SRC} ${RTOS_MEMORY_SRC} PROPERTIES LANGUAGE CXX COMPILE_FLAGS "${GENERIC_C_FLAGS} ${OPTIMISATION_FLAGS}")


add_library(rtos_malloc ${RTOS_MEMORY_SRC})
add_executable(decodeur ${DECODER_SRC})
add_executable(compositeur ${COMPOSITER_SRC})
add_executable(filtreur ${FILTER_SRC})
add_executable(redimensionneur ${REDIMENSIONNEUR_SRC})
add_executable(grayscaler ${GRAYSCALER_SRC})
set_target_properties(decodeur PROPERTIES LINKER_LANGUAGE CXX)
target_link_libraries(decodeur rtos_malloc rt pthread)
target_link_libraries(compositeur rtos_malloc rt pthread)
target_link_libraries(filtreur rtos_malloc rt pthread)
target_link_libraries(redimensionneur rtos_malloc rt pthread)
target_link_libraries(grayscaler rtos_malloc rt pthread)
