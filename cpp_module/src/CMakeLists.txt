cmake_minimum_required(VERSION 3.19)
project(gdextension)

include(FetchContent)

FetchContent_Declare(
		GDExtension
		GIT_REPOSITORY https://github.com/godotengine/godot-cpp.git
		GIT_TAG godot-4.1.2-stable
)

FetchContent_MakeAvailable(GDExtension)

include(FetchContent)

FetchContent_Declare(
        flecs
        GIT_REPOSITORY https://github.com/SanderMertens/flecs
        GIT_TAG v3.2.11
)

FetchContent_MakeAvailable(flecs)

# Automatically pick up files added to src
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
	"${CMAKE_CURRENT_SOURCE_DIR}/entity_drawer/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/entity_drawer/*.cpp"
	"${CMAKE_CURRENT_SOURCE_DIR}/lib/*.h"
	"${CMAKE_CURRENT_SOURCE_DIR}/lib/*.cpp")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/" PREFIX src FILES ${SOURCES})

add_library(conveyors_2d SHARED ${SOURCES})
target_link_libraries(conveyors_2d PRIVATE godot::cpp flecs::flecs_static)
target_include_directories(conveyors_2d PRIVATE ./)

install (TARGETS conveyors_2d EXPORT ${PROJECT_NAME}-export)
