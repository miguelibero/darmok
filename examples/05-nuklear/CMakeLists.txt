cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-nuklear
  LANGUAGES CXX
  DESCRIPTION "nuklear UI example darmok project"
)

set(DARMOK_BUILD_NUKLEAR ON CACHE BOOL "build darmok with nuklear UI support")
get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
add_subdirectory(../../ ../../${BINARY_DIR_NAME})

darmok_process_assets()

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  ${ASSETS}
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok)


