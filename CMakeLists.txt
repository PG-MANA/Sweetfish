#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.20)
project(sweetfish CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 CONFIG REQUIRED Core Widgets Network LinguistTools)

add_subdirectory(src/Network)
add_subdirectory(src/Setting)
add_subdirectory(src/Mastodon)
add_subdirectory(src/UI)
add_subdirectory(src/Translations)

add_executable(sweetfish src/Sweetfish.cpp src/Resources/icon/icon.qrc src/Resources/ui/ui-icon.qrc)

target_link_libraries(sweetfish UI)

add_dependencies(sweetfish translations)

install(TARGETS sweetfish DESTINATION bin)
install(DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/locales DESTINATION lib/sweetfish)
