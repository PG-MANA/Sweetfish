#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.20)

add_library(Setting STATIC Setting.cpp)

target_link_libraries(Setting PRIVATE Qt6::Core)
