project("bluetooth_sound")
cmake_minimum_required(VERSION 3.6)

set(CMAKE_C_FLAGS "-Wall -Wno-unused-value -Wno-format-extra-args -Wno-format")

add_executable(server server.c fifo.c)
target_link_libraries(server asound vorbis vorbisenc m ogg)

add_executable(client client.c stb_vorbis.c fifo.c)
target_link_libraries(client asound vorbis vorbisenc m)

add_executable(bluetooth_sim bluetooth_sim.c fifo.c)

set_property(TARGET bluetooth_sim server client PROPERTY C_STANDARD 11)
