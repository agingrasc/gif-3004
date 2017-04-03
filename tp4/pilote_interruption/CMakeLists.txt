cmake_minimum_required(VERSION 3.6)
project(pilote_interruption)

set(INTERRUPTION_SRC setr_driver_irq.c)

add_executable(${PROJECT_NAME} ${INTERRUPTION_SRC})