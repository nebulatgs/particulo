cmake_minimum_required(VERSION 3.16.3)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
project(examples)

include_directories("../lib/emsdk/upstream/emscripten/system/include")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("../lib/glfw" "../lib/glfw/build")
add_subdirectory("../lib/glad" "../lib/glad/build")
add_subdirectory("../lib/glm" "../lib/glm/build")

include_directories("../lib/glfw/include")
include_directories("../lib/glad/include")
include_directories("../lib/Polyline2D/include")
include_directories("../lib/glm")
include_directories("../src")

add_subdirectory(bouncing_balls)
add_subdirectory(solar_system)
add_subdirectory(interactive_solar_system)
add_subdirectory(interactive_rigidbody)
add_subdirectory(verlet)