cmake_minimum_required(VERSION 3.0)
project(Glad)

add_library(glad include/glad/glad.h glad.c)
target_include_directories(glad PUBLIC include/)
target_include_directories(glad PUBLIC include/KHR)