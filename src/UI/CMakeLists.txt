#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.20)

add_library(UI STATIC MainWindow.cpp  ImageLabel.cpp TextLabel.cpp TootContent.cpp TootInfo.cpp ImageViewer.cpp VideoPlayer.cpp UserInfoBox.cpp)
find_package(Phonon4Qt6 QUIET)
if(Phonon4Qt6_FOUND AND (NOT (DEFINED USE_MULTIMEDIA)))
    target_link_libraries(UI PRIVATE ${PHONON_LIBRARY} Qt6::Core Qt6::DBus Qt6::Network Qt6::Widgets Network Setting Mastodon)
    message( STATUS "Using Phonon4Qt6")
else(Phonon4Qt6_FOUND AND (NOT (DEFINED USE_MULTIMEDIA)))
    find_package(Qt6 REQUIRED COMPONENTS MultimediaWidgets)
    target_link_libraries(UI PRIVATE Qt6::Core Qt6::DBus Qt6::MultimediaWidgets Qt6::Network Qt6::Widgets Network Setting Mastodon)
    add_definitions("-DUSE_MULTIMEDIA")
    message( STATUS "Using Qt Multimedia")
endif(Phonon4Qt6_FOUND AND (NOT (DEFINED USE_MULTIMEDIA)))
