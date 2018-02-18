cmake_minimum_required(VERSION 2.8)
project(JsonParser_test)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

add_library(JsonParser Json.cpp)
add_executable(JsonParser_test test.cpp)
target_link_libraries(JsonParser_test JsonParser)