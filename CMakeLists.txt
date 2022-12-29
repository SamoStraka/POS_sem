cmake_minimum_required(VERSION 3.18)
project(pos_sem C)

set(CMAKE_C_STANDARD 99)

add_executable(pos_sem main.c k_a_t_client.h k_a_t_server.h k_a_t_definitions.h k_a_t_client.c k_a_t_server.c k_a_t_definitions.c)

target_link_libraries(pos_sem pthread m)