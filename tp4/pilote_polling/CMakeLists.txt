cmake_minimum_required(VERSION 3.6)
project(pilote_polling)

set(POLLING_SRC setr_driver.c)
add_executable(${PROJECT_NAME} ${POLLING_SRC})