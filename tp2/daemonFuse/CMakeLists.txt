cmake_minimum_required(VERSION 2.6)
project(daemonFuse)

set(PROJECT_INCLUDE_DIR ./)
set(PROJECT_SOURCE_DIR ./)

set(GENERIC_C_FLAGS
        -Wall)

set(FUSE_SRC
        communications.c
        fstools.c
        setrfs.c)


add_executable(setrfs ${FUSE_SRC})
target_compile_options(setrfs PRIVATE ${GENERIC_C_FLAGS})
target_compile_definitions(setrfs PRIVATE -D_FILE_OFFSET_BITS=64)
target_link_libraries(setrfs fuse)
