#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(Sierra2020 C)

# include libraries
include_directories(libraries/ccs811)
add_subdirectory(libraries/ccs811)

# Create executable
# add_library (ccs811 ccs811.c)
ADD_EXECUTABLE(${PROJECT_NAME} main.c)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} applibs pthread gcc_s c)

# Add MakeImage post-build command
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")

target_link_libraries (${PROJECT_NAME} ccs811 lib_ccs811)
