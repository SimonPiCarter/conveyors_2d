cmake_minimum_required(VERSION 3.19)

add_executable(main src/main.cpp)

install (TARGETS main EXPORT ${PROJECT_NAME}-export)
