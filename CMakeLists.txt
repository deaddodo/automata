cmake_minimum_required(VERSION 3.6)
project(automata)

set(CMAKE_C_STANDARD 11)

set(SDL2_LIBRARY /usr/lib/libSDL2.so)

set(SOURCE_FILES main.c)
add_executable(automata ${SOURCE_FILES})
target_link_libraries(automata ${SDL2_LIBRARY})