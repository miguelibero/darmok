cmake_minimum_required(VERSION 3.26)

set(VCPKG_INSTALLED_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../../vcpkg_installed)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

project(
  darmok-example-empty
  LANGUAGES CXX
  DESCRIPTION "empty example darmok project"
)

set(DARMOK_BUILD_IMGUI ON CACHE BOOL "build darmok with imGUI support")
get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
add_subdirectory(../../ ../../${BINARY_DIR_NAME})

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok)