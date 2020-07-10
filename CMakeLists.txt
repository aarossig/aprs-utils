################################################################################
#
# aprs-file-copy
#
################################################################################

# Project Settings #############################################################

# Setup CMake and project requirements.
cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 17)

# Project name.
project(aprs-file-copy)

# Dependencies #################################################################

include(FindProtobuf)

find_package(PkgConfig REQUIRED)
find_package(Protobuf REQUIRED)

pkg_check_modules(tclap REQUIRED tclap)
pkg_check_modules(SDL_net REQUIRED SDL_net)

# Subdirectories ###############################################################

add_subdirectory(aprs_file_copy)
add_subdirectory(proto)
add_subdirectory(util)
