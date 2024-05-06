cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-scripting
  LANGUAGES CXX
  DESCRIPTION "scripting example darmok project"
)

set(DARMOK_BUILD_SCRIPTING ON CACHE BOOL "enable scripting support")
set(DARMOK_BUILD_SCRIPTING_EXE ON CACHE BOOL "compile scripting executable")

get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
add_subdirectory(../../ ../../${BINARY_DIR_NAME})

darmok_process_assets()

add_custom_target(${PROJECT_NAME}
  COMMAND $<TARGET_FILE:darmok-scripting>
  DEPENDS darmok-scripting
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  SOURCES ${ASSETS}
  COMMENT "running darmok scripting executable"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

