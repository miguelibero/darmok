cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-empty
  LANGUAGES CXX
  DESCRIPTION "empty example darmok project"
)

get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
add_subdirectory(../../ ../../${BINARY_DIR_NAME})

set(sources
  src/main.cpp
)

add_executable(
  ${PROJECT_NAME}
  ${sources}
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC darmok)