cmake_minimum_required(VERSION 3.28)
project(untitled1)

set(CMAKE_CXX_STANDARD 23)

find_package(Boost 1.85 REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})

add_executable(main main.cpp)
target_link_libraries(main ${Boost_LIBRARIES})