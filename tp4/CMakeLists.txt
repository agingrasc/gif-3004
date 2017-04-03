project("keyboard")
cmake_minimum_required(VERSION 3.6)

include_directories(/opt/cross/raspberry-pi/dependancesLab4/include)
include_directories(/opt/cross/raspberry-pi/dependancesLab4/arch/arm/include)
include_directories(/opt/cross/raspberry-pi/dependancesLab4/arch/arm/include/generated)
include_directories(/opt/cross/raspberry-pi/dependancesLab4/arch/mach

add_subdirectory(pilote_interruption)
add_subdirectory(pilote_polling)
