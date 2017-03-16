#include(CMakeForceCompiler)
# info generic, version non necessaire
set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_VERSION 1)

set(TOOLCHAIN_PREFIX "/opt/cross/raspberry-pi/arm-linux-gnueabihf")
# binaire pour la compilation

#link_directories("${TOOLCHAIN_PREFIX}/arm-linux-gnueabihf/lib" "/opt/cross/raspberry-pi/rootfs/lib")
#set(CMAKE_LINKER "${TOOLCHAIN_PREFIX}/arm-linux-gnueabihf/bin/ld")
#set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_C_LINK_EXECUTABLE "<CMAKE_LINKER> <FLAGS> <CMAKE_C_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET>")
#set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET> <LINK_LIBRARIES>")
#set(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_LINKER} <FLAGS> <CMAKE_CXX_LINK_FLAGS> <LINK_FLAGS> <OBJECTS> -o <TARGET>")

set(CMAKE_C_COMPILER "${TOOLCHAIN_PREFIX}/bin/arm-linux-gnueabihf-gcc")
set(CMAKE_CXX_COMPILER "${TOOLCHAIN_PREFIX}/bin/arm-linux-gnueabihf-g++")

# sysroot de compilation croisee
set(CMAKE_SYSROOT "/opt/cross/raspberry-pi/rootfs")
set(CMAKE_FIND_ROOT_PATH "/opt/cross/raspberry-pi/rootfs")

# parametre pour la recherche
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

