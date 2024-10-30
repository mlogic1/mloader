cmake_minimum_required(VERSION 3.10)

project(cli)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(mloader-cli main.cpp)

target_link_libraries(mloader-cli PRIVATE mloader)