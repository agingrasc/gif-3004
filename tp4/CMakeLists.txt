project("keyboard")
cmake_minimum_required(VERSION 3.6)

include_directories(/opt/cross/raspberry-pi/dependancesLab4/include)
include_directories(/opt/cross/raspberry-pi/dependancesLab4/arch/arm/include)
include_directories(/opt/cross/raspberry-pi/dependancesLab4/arch/arm/include/generated)
include_directories(/opt/cross/raspberry-pi/dependancesLab4/arch/arm/mach-bcm2709/include)
add_definitions(-D__KERNEL__ -D__LINUX_ARM_ARCH__=7 -DMODULE)

set(CMAKE_EXE_LINKER_FLAGS ${CMAKE_EXE_LINKER_FLAGS} "-v")
set(SETR_LAB4_FLAGS "-march=armv7-a -c -fmessage-length=0 -include /opt/cross/raspberry-pi/dependancesLab4/include/linux/kconfig.h -Wa,-mimplicit-it=thumb -DKBUILD_MODNAME=\"setr_driver\"")
set(CMAKE_C_FLAGS "${SETR_LAB4_FLAGS}")

add_subdirectory(pilote_interruption)
add_subdirectory(pilote_polling)
