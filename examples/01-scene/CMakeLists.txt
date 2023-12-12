cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-scene
  LANGUAGES CXX
  DESCRIPTION "scene darmok project"
)

add_subdirectory(../../ ../../build)

set(ASSETS_DIR ${CMAKE_BINARY_DIR}/assets)
FILE(COPY assets/darmok.jpg DESTINATION ${ASSETS_DIR})

darmok_compile_shader(
  TYPE VERTEX
  SHADERS assets/sprite_vertex.sc
  VARYING_DEF assets/varying.def.sc
  OUTPUT_DIR ${ASSETS_DIR}
)
darmok_compile_shader(
  TYPE FRAGMENT
  SHADERS assets/sprite_fragment.sc
  VARYING_DEF assets/varying.def.sc
  OUTPUT_DIR ${ASSETS_DIR}
)

set(sources
  src/main.cpp
  assets/sprite_vertex.sc
  assets/sprite_fragment.sc
  assets/varying.def.sc
)

add_executable(
  ${PROJECT_NAME}
  ${sources}
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_CONFIG_IMPLEMENT_MAIN=1"
)
set(BGFX_SHADER_INCLUDE_PATH lala)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
target_link_libraries (${PROJECT_NAME} LINK_PUBLIC darmok)


