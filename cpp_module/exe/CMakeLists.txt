cmake_minimum_required(VERSION 3.19)

add_executable(main src/main.cpp src/Line.cpp)

target_link_libraries(main PRIVATE flecs::flecs_static)

install (TARGETS main EXPORT ${PROJECT_NAME}-export)
