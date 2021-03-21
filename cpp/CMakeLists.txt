
cmake_minimum_required(VERSION 3.12)

project(flex)

set(CMAKE_CXX_STANDARD 14)

file(GLOB source *.cpp *.cc *.h)

add_executable(flex ${source})
