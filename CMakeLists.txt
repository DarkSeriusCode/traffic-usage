cmake_minimum_required(VERSION 3.15)
project(traffic_usage VERSION 0.1.0 LANGUAGES C)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

configure_file("${PROJECT_SOURCE_DIR}/cmake/config.h.in" "${PROJECT_SOURCE_DIR}/src/config.h")

file(GLOB src "src/*.c")

add_executable(${PROJECT_NAME} main.c ${src})
target_link_libraries(${PROJECT_NAME} PUBLIC "ncurses")
target_compile_options(${PROJECT_NAME} PUBLIC "-Wall")
