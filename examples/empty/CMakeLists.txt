cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-empty
  LANGUAGES CXX
  DESCRIPTION "empty darmok project"
)

add_subdirectory (../../ ../../build)

set(sources
  src/main.cpp
)

add_executable(
  ${PROJECT_NAME}
  ${sources}
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_CONFIG_IMPLEMENT_MAIN=1"
)
set_property(TARGET ${PROJECT_NAME} PROPERTY CXX_STANDARD 20)
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC darmok)