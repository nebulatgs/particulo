cmake_minimum_required(VERSION 3.16.3)
project(particulo_example)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_FILES src/main.cpp)
include_directories("../lib/emsdk/upstream/emscripten/system/include")

set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

add_subdirectory("../lib/glfw" "../lib/glfw/build")
add_subdirectory("../lib/glad" "../lib/glad/build")
add_subdirectory("../lib/glm" "../lib/glm/build")

include_directories("../lib/glfw/include")
include_directories("../lib/glad/include")
include_directories("../lib/glm")
include_directories("../src")

add_executable(particulo_example ${SOURCE_FILES})
if (WIN32)
target_link_options(particulo_example PRIVATE "-static-libgcc" "-static-libstdc++")
endif()

target_link_libraries(particulo_example glfw ${GLFW_LIBRARIES})
target_link_libraries(particulo_example glad)
target_link_libraries(particulo_example glm)