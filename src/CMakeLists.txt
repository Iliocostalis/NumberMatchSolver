cmake_minimum_required(VERSION 3.16)

target_include_directories(programm PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include)

file(GLOB srcFiles CONFIGURE_DEPENDS "*.cpp")
target_sources(programm PRIVATE ${srcFiles})