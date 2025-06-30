#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.20)
project(sweetfish CXX)

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Qt6 REQUIRED COMPONENTS Core DBus Network LinguistTools Widgets)


add_subdirectory(src/Network)
add_subdirectory(src/Setting)
add_subdirectory(src/Mastodon)
add_subdirectory(src/UI)

add_executable(${CMAKE_PROJECT_NAME} src/Sweetfish.cpp src/Resources/icon/icon.qrc src/Resources/ui/ui-icon.qrc)

target_link_libraries(${CMAKE_PROJECT_NAME} PRIVATE Qt6::Core Qt6::Network Qt6::Widgets UI)

qt_standard_project_setup(I18N_TRANSLATED_LANGUAGES ja_JP I18N_SOURCE_LANGUAGE en)
qt_add_translations(${CMAKE_PROJECT_NAME} TS_FILE_BASE src/Resources/translation/lang QM_FILES_OUTPUT_VARIABLE QM_FILES)

#add_dependencies(${CMAKE_PROJECT_NAME} update_translations)

install(TARGETS ${CMAKE_PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR})
install(PROGRAMS sweetfish.desktop DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications)
install(FILES src/Resources/icon/icon-normal.png DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps RENAME ${CMAKE_PROJECT_NAME}.png)
install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/translations)
