#
# This software is Licensed under the Apache License Version 2.0
# See LICENSE
#
cmake_minimum_required(VERSION 3.1)

add_library(Mastodon STATIC  Mastodon.cpp TootData.cpp Streamer.cpp MediaUpload.cpp)
target_link_libraries(Mastodon Network Setting)
