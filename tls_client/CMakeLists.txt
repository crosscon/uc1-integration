# SPDX-License-Identifier: Apache-2.0

cmake_minimum_required(VERSION 3.20.0)
find_package(Zephyr REQUIRED HINTS $ENV{ZEPHYR_BASE})
project(dhcpv4_client)

file(GLOB_RECURSE app_sources src/*.c)

target_sources(app PRIVATE ${app_sources})

target_include_directories(app PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pufcaller
    ${CMAKE_CURRENT_SOURCE_DIR}/src/pufcaller/crosscon_hv
)

# Optional: define HAS_LOCAL_WIFI_CONFIG if wifi_config_local.h exists
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/src/wifi_config_local.h")
    add_compile_definitions(HAS_LOCAL_WIFI_CONFIG=1)
endif()
