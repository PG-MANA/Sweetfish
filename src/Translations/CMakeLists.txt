#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.1)

set(TS_FILES en_AU.ts)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)# For Clean
qt5_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src/UI/ ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})

add_custom_command(
  TARGET translations POST_BUILD
  COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/locales
  COMMAND ${CMAKE_COMMAND} -E copy ${QM_FILES}  ${CMAKE_BINARY_DIR}/locales/
)
