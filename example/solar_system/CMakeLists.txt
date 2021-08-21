cmake_minimum_required(VERSION 3.16.3)
project(solar_system)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_FILES src/main.cpp)

add_executable(solar_system ${SOURCE_FILES})
if (WIN32)
target_link_options(solar_system PRIVATE "-static-libgcc" "-static-libstdc++")
endif()

target_link_libraries(solar_system glfw ${GLFW_LIBRARIES})
target_link_libraries(solar_system glad)
target_link_libraries(solar_system glm)