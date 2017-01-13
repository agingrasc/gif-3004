include(CMakeForceCompiler)
# info generic, version non necessaire
set(CMAKE_SYSTEM_NAME  Linux)
set(CMAKE_SYSTEM_VERSION 1)

# binaire pour la compilation
set(CMAKE_C_COMPILER /usr/bin/arm-linux-gnueabihf-gcc)
set(CMAKE_CXX_COMPILER /usr/bin/arm-linux-gnueabihf-g++)

# sysroot de compilation croisee
set(CMAKE_FIND_ROOT_PATH /usr/arm-linux-gnueabihf)

# parametre pour la recherche
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

