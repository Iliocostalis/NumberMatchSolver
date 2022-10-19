cmake_minimum_required(VERSION 3.16)

project(programm)

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")

add_executable(programm "")

add_subdirectory("src")

#target_link_libraries(programm fann)