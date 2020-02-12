#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(Sierra2020 C)

# include libraries
include_directories(libraries/ccs811)
add_subdirectory(libraries/ccs811)

include_directories(libraries/avnet_onboard)
add_subdirectory(libraries/avnet_onboard)

# Create executable
ADD_EXECUTABLE(${PROJECT_NAME} main.c sensors.c sensors.h)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} applibs pthread gcc_s c)

# Add MakeImage post-build command
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")

target_link_libraries (${PROJECT_NAME} ccs811 lib_ccs811 avnet_onboard)
