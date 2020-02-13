#  Copyright (c) Microsoft Corporation. All rights reserved.
#  Licensed under the MIT License.

CMAKE_MINIMUM_REQUIRED(VERSION 3.8)
PROJECT(Sierra2020 C)

# include libraries
include_directories(libraries/ccs811)
add_subdirectory(libraries/ccs811)
include_directories(libraries/iothub)
add_subdirectory(libraries/iothub)

include_directories(libraries/avnet_onboard)
add_subdirectory(libraries/avnet_onboard)

# Create executable
ADD_EXECUTABLE(${PROJECT_NAME} main.c sensors.c sensors.h messages.c messages.h)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} applibs pthread gcc_s c)
#
TARGET_INCLUDE_DIRECTORIES(${PROJECT_NAME} PUBLIC ${AZURE_SPHERE_API_SET_DIR}/usr/include/azureiot)
TARGET_COMPILE_DEFINITIONS(${PROJECT_NAME} PUBLIC AZURE_IOT_HUB_CONFIGURED)
TARGET_LINK_LIBRARIES(${PROJECT_NAME} m azureiot applibs pthread gcc_s c)

target_link_libraries (${PROJECT_NAME} ccs811 lib_ccs811 avnet_onboard parson epoll_timerfd_utilities)

find_program(POWERSHELL powershell.exe)

if (POWERSHELL)
    # Run validate_manifest script during build
    ADD_CUSTOM_TARGET(ValidateManifest ALL
        COMMAND ${POWERSHELL} -ExecutionPolicy Bypass -NoProfile -NonInteractive -File ${CMAKE_SOURCE_DIR}/script/validate_manifest.ps1
    DEPENDS ${CMAKE_SOURCE_DIR}/app_manifest.json)
else()
    # Warn users without PowerShell to update their manifest
    ADD_CUSTOM_TARGET(ValidateManifest ALL
        COMMAND echo "Please ensure that you have updated app_manifest.json as described in IoTCentral.md or IoTHub.md, as appropriate."
    DEPENDS ${CMAKE_SOURCE_DIR}/app_manifest.json)
endif()

ADD_DEPENDENCIES(ValidateManifest ${PROJECT_NAME})

# Add MakeImage post-build command
INCLUDE("${AZURE_SPHERE_MAKE_IMAGE_FILE}")