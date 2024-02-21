cmake_minimum_required(VERSION 3.19)

add_executable(main src/main.cpp)

target_link_libraries(main PRIVATE conveyors_2d)

install (TARGETS main EXPORT ${PROJECT_NAME}-export)
