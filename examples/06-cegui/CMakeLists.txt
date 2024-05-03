cmake_minimum_required(VERSION 3.15)

project(
  darmok-example-cegui
  LANGUAGES CXX
  DESCRIPTION "CEGUI example darmok project"
)

set(DARMOK_BUILD_CEGUI ON CACHE BOOL "build darmok with cegui support")
get_filename_component(BINARY_DIR_NAME ${CMAKE_BINARY_DIR} NAME)
add_subdirectory(../../ ../../${BINARY_DIR_NAME})

set(CEGUI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../lib/cegui)
set(CEGUI_ASSETS_DIR ${CEGUI_DIR}/datafiles)
file(GLOB_RECURSE CEGUI_ASSETS ${CEGUI_ASSETS_DIR}/*)
foreach(ASSET ${CEGUI_ASSETS})
  file(RELATIVE_PATH ASSET_SRC ${CMAKE_CURRENT_SOURCE_DIR} ${ASSET})  
  file(RELATIVE_PATH ASSET_DST ${CEGUI_ASSETS_DIR} ${ASSET})  
  addCopySource(${ASSET_SRC} assets/cegui/${ASSET_DST})
endforeach()
source_group(TREE ${CEGUI_ASSETS_DIR} PREFIX "CEGUI Asset Files" FILES ${CEGUI_ASSETS})

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  ${CEGUI_DIR}/samples/GameMenu/GameMenu.cpp
  ${CEGUI_ASSETS}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CEGUI_DIR}/cegui/include
  ${CEGUI_DIR}/samples/common/include
  ${CEGUI_DIR}/samples/GameMenu
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)

set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok)


