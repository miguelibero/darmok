cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-model
  LANGUAGES CXX
  DESCRIPTION "model example darmok project"
)

get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
add_subdirectory(../../ ../../${BINARY_DIR_NAME})

addCopySource(assets/human.fbx assets/human.fbx)
addCopySource(assets/human.png assets/human.png)

set(sources
  src/main.cpp
  assets/human.fbx
  assets/human.png
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
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok)


