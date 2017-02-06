cmake_minimum_required(VERSION 2.6)
project(serveurCurl)

set(PROJECT_INCLUDE_DIR ./)
set(PROJECT_SOURCE_DIR ./)

set(GENERIC_C_FLAGS
        -Wall)

set(CURL_SRC
        actions.c
        communications.c
        requete.c
        servermain.c
        telechargeur.c)



add_executable(serveurCurl ${CURL_SRC})
target_compile_options(serveurCurl PRIVATE ${GENERIC_C_FLAGS})
target_link_libraries(serveurCurl curl)
