cmake_minimum_required(VERSION 3.16.3)
project(interactive_rigidbody)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(SOURCE_FILES src/main.cpp)

add_executable(interactive_rigidbody ${SOURCE_FILES})
if (MSVC)
target_link_options(interactive_rigidbody PRIVATE "-static-libgcc" "-static-libstdc++")
endif()
if (MINGW)
target_link_options(interactive_rigidbody PRIVATE "-static")
endif()
target_link_libraries(interactive_rigidbody glfw ${GLFW_LIBRARIES})
target_link_libraries(interactive_rigidbody glad)
target_link_libraries(interactive_rigidbody glm)