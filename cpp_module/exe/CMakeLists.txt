cmake_minimum_required(VERSION 3.19)

add_executable(main
	src/main.cpp
)

target_link_libraries(main PRIVATE flecs::flecs_static conveyors_2d_static)
target_include_directories(main PRIVATE src/)

install (TARGETS main EXPORT ${PROJECT_NAME}-export)
