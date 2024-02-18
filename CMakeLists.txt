cmake_minimum_required(VERSION 3.15)

project(
  darmok
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "hobby game engine"
)

set(SRC_DIR src)
set(INCLUDE_DIR include/darmok)
set(ASSETS_DIR assets)
set(SHADERS_DIR assets/shaders)
set(SHADERS_BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include)
set(SHADERS_OUTPUT_DIR generated/shaders)
set(LIBS_DIR lib)

add_subdirectory(${LIBS_DIR}/bgfx.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CMake/compileShader.cmake)

darmok_compile_shader(
  TYPE VERTEX HEADER
  SHADERS ${SHADERS_DIR}/sprite_vertex.sc
  VARYING_DEF ${SHADERS_DIR}/sprite_varyingdef.sc
  BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
  OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
)
darmok_compile_shader(
  TYPE FRAGMENT HEADER
  SHADERS ${SHADERS_DIR}/sprite_fragment.sc
  VARYING_DEF ${SHADERS_DIR}/sprite_varyingdef.sc
  BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
  OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
)
darmok_compile_shader(
  TYPE VERTEX HEADER
  SHADERS ${SHADERS_DIR}/debug_vertex.sc
  VARYING_DEF ${SHADERS_DIR}/debug_varyingdef.sc
  BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
  OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
)
darmok_compile_shader(
  TYPE FRAGMENT HEADER
  SHADERS ${SHADERS_DIR}/debug_fragment.sc
  VARYING_DEF ${SHADERS_DIR}/debug_varyingdef.sc
  BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
  OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
)
darmok_compile_shader(
  TYPE VERTEX HEADER
  SHADERS ${SHADERS_DIR}/basic_vertex.sc
  VARYING_DEF ${SHADERS_DIR}/basic_varyingdef.sc
  BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
  OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
)
darmok_compile_shader(
  TYPE FRAGMENT HEADER
  SHADERS ${SHADERS_DIR}/basic_fragment.sc
  VARYING_DEF ${SHADERS_DIR}/basic_varyingdef.sc
  BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
  OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
)

set(shaders
  ${SHADERS_DIR}/debug_vertex.sc
  ${SHADERS_DIR}/debug_varyingdef.sc
  ${SHADERS_DIR}/debug_fragment.sc

  ${SHADERS_DIR}/sprite_vertex.sc
  ${SHADERS_DIR}/sprite_varyingdef.sc
  ${SHADERS_DIR}/sprite_fragment.sc

  ${SHADERS_DIR}/basic_vertex.sc
  ${SHADERS_DIR}/basic_varyingdef.sc
  ${SHADERS_DIR}/basic_fragment.sc
)

set(sources
  ${SRC_DIR}/window.cpp
  ${SRC_DIR}/platform.cpp
  ${SRC_DIR}/app.cpp
  ${SRC_DIR}/glfw.cpp
  ${SRC_DIR}/input.cpp
  ${SRC_DIR}/imgui.cpp
  ${SRC_DIR}/asset.cpp
  ${SRC_DIR}/scene.cpp
  ${SRC_DIR}/utils.cpp
  ${SRC_DIR}/sprite.cpp
  ${SRC_DIR}/anim.cpp
  ${SRC_DIR}/physics2d.cpp
  ${SRC_DIR}/model.cpp
  ${SRC_DIR}/data.cpp
  ${SRC_DIR}/mesh.cpp
  ${SRC_DIR}/color.cpp
  ${SRC_DIR}/material.cpp
)

set(headers
  ${INCLUDE_DIR}/app.hpp
  ${INCLUDE_DIR}/input.hpp
  ${INCLUDE_DIR}/window.hpp
  ${INCLUDE_DIR}/utils.hpp
  ${INCLUDE_DIR}/asset.hpp
  ${INCLUDE_DIR}/imgui.hpp
  ${INCLUDE_DIR}/scene.hpp
  ${INCLUDE_DIR}/sprite.hpp
  ${INCLUDE_DIR}/anim.hpp
  ${INCLUDE_DIR}/physics2d.hpp
  ${INCLUDE_DIR}/model.hpp
  ${INCLUDE_DIR}/data.hpp
  ${INCLUDE_DIR}/mesh.hpp
  ${INCLUDE_DIR}/color.hpp
  ${INCLUDE_DIR}/material.hpp
  ${INCLUDE_DIR}/optional_ref.hpp
)

set(LIB_NAME darmok)

add_library(
  ${LIB_NAME}
  ${headers}
  ${sources}
  ${shaders}
)

set_target_properties(
  ${LIB_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
)
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

set(BGFX_BASE ${LIBS_DIR}/bgfx.cmake/bgfx)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BGFX_BASE}/3rdparty
    PRIVATE
    ${BGFX_BASE}/examples/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/include
  )

include(GNUInstallDirs)
install(
  TARGETS
  ${LIB_NAME}
  EXPORT
  ${LIB_NAME}Targets
  LIBRARY DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
  RUNTIME DESTINATION
  ${CMAKE_INSTALL_BINDIR}
  ARCHIVE DESTINATION
  ${CMAKE_INSTALL_LIBDIR}
  INCLUDES DESTINATION
  include
  PUBLIC_HEADER DESTINATION
  include
)

install(
  EXPORT
  ${LIB_NAME}Targets
  FILE
  ${PROJECT_NAME}Targets.cmake
  NAMESPACE
  ${LIB_NAME}::
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/${LIB_NAME}
)

set_target_properties(${LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

#
# Install the `include` directory
#

install(
  DIRECTORY
  include/${LIB_NAME}
  DESTINATION
  include
)

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
  ${LIB_NAME}ConfigVersion.cmake
  VERSION
  ${PROJECT_VERSION}
  COMPATIBILITY
  SameMajorVersion
)

include(GenerateExportHeader)
generate_export_header(${LIB_NAME})
install(
  FILES
  ${PROJECT_BINARY_DIR}/${LIB_NAME}.hpp
  DESTINATION
  include
)

# testing
enable_testing()
add_subdirectory(test)

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()
set_property(TARGET ${LIB_NAME} PROPERTY CXX_STANDARD 20)

# openGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# glfw
add_subdirectory(${LIBS_DIR}/glfw)
target_link_libraries(${LIB_NAME} PRIVATE glfw)
add_compile_definitions(DARMOK_CONFIG_USE_GLFW)

# glm
add_compile_definitions(GLM_ENABLE_EXPERIMENTAL)
add_subdirectory(${LIBS_DIR}/glm)
target_link_libraries(${LIB_NAME} PUBLIC glm)
install(
  TARGETS
  glm glm-header-only
  EXPORT
  glmTargets
)
install(
  EXPORT
  glmTargets
  FILE
  glmTargets.cmake
  DESTINATION
  ${CMAKE_INSTALL_LIBDIR}/cmake/glm
)
set_property(TARGET ${LIB_NAME} PROPERTY CXX_STANDARD 20)

# bgfx
target_link_libraries(${LIB_NAME} PUBLIC bgfx bimg bimg_decode bx)

# imgui
include(${CMAKE_CURRENT_LIST_DIR}/CMake/imgui.cmake)
target_link_libraries(${LIB_NAME} PUBLIC imgui)

# entt
add_subdirectory(${LIBS_DIR}/entt)
target_link_libraries(${LIB_NAME} PUBLIC EnTT)

# assimp
add_subdirectory(${LIBS_DIR}/assimp)
target_link_libraries(${LIB_NAME} PRIVATE assimp)

# pugixml
add_subdirectory(${LIBS_DIR}/pugixml)
target_link_libraries(${LIB_NAME} PRIVATE pugixml)
add_compile_definitions(PUGIXML_HEADER_ONLY) # also included in assimp

