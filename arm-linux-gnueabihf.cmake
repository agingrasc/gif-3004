#include(CMakeForceCompiler)
# info generic, version non necessaire
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(TOOLCHAIN_PREFIX "/opt/cross/raspberry-pi/arm-linux-gnueabihf")

# binaire pour la compilation
set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/bin/arm-linux-gnueabihf-g++")

# sysroot de compilation croisee
set(CMAKE_SYSROOT "/opt/cross/raspberry-pi/rootfs")
set(CMAKE_FIND_ROOT_PATH "/opt/cross/raspberry-pi/rootfs")
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")
set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")
set(CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} --sysroot=${CMAKE_FIND_ROOT_PATH}")

# parametre pour la recherche
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
