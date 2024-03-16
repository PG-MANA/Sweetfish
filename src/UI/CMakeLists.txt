#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.20)

add_library(UI STATIC MainWindow.cpp  ImageLabel.cpp TextLabel.cpp TootContent.cpp TootInfo.cpp ImageViewer.cpp VideoPlayer.cpp UserInfoBox.cpp)
find_package(Phonon4Qt6 QUIET)
if(Phonon4Qt6_FOUND)
    target_link_libraries(UI Qt6::Widgets ${PHONON_LIBRARY}  Mastodon)
    message( STATUS "Using Phonon4Qt6")
else(Phonon4Qt6_FOUND)
    find_package(Qt6 CONFIG REQUIRED Multimedia MultimediaWidgets)
    target_link_libraries(UI Qt6::Widgets Qt6::Multimedia Qt6::MultimediaWidgets Mastodon)
    add_definitions("-DUSE_MULTIMEDIA")
    message( STATUS "Using Qt Multimedia")
endif(Phonon4Qt6_FOUND)
