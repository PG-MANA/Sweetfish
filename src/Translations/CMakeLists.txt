#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.20)

set(TS_FILES en_AU.ts en_US.ts en_GB.ts)
set_directory_properties(PROPERTIES CLEAN_NO_CUSTOM 1)# For Clean
qt6_create_translation(QM_FILES ${CMAKE_SOURCE_DIR}/src/UI/ ${TS_FILES})
add_custom_target(translations DEPENDS ${QM_FILES})

install(FILES ${QM_FILES} DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/${CMAKE_PROJECT_NAME}/translations)
