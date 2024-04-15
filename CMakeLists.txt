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

option(BGFX_BUILD_EXAMPLES "Build bgfx examples." ON)
set(BGFX_BUILD_EXAMPLES OFF)
add_subdirectory(${LIBS_DIR}/bgfx.cmake)
include(${CMAKE_CURRENT_LIST_DIR}/CMake/compileShader.cmake)

macro(addExportTargets NAME TARGETS)
  install(TARGETS ${TARGETS} EXPORT ${NAME}Targets)
  install(EXPORT ${NAME}Targets FILE ${NAME}Targets.cmake
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${NAME})
endmacro()

macro(addCopySource SRC DST)
  add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/${DST}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/${SRC} ${CMAKE_BINARY_DIR}/${DST}
    MAIN_DEPENDENCY ${SRC})
endmacro()

set(SHADERS "")
set(SHADER_INCLUDES "")

macro(addShaderVertexLayout NAME)
  set(VERTEX_LAYOUT_PATH "${SHADERS_DIR}/${NAME}_vertex_layout.json")
  bgfx_compile_binary_to_header(
    INPUT_FILE ${CMAKE_CURRENT_SOURCE_DIR}/${VERTEX_LAYOUT_PATH}
    OUTPUT_FILE include/${SHADERS_OUTPUT_DIR}/${NAME}_vertex_layout.h
    ARRAY_NAME ${NAME}_vertex_layout
  )
  list(APPEND SHADERS ${VERTEX_LAYOUT_PATH})
endmacro()

macro(addShader NAME)
  set(VERTEX_PATH "${SHADERS_DIR}/${NAME}_vertex.sc")
  set(FRAGMENT_PATH "${SHADERS_DIR}/${NAME}_fragment.sc")
  set(VARYING_DEF_PATH "${SHADERS_DIR}/${NAME}_varyingdef.sc")

  darmok_compile_shader(
    TYPE VERTEX HEADER
    SHADERS ${VERTEX_PATH}
    VARYING_DEF ${VARYING_DEF_PATH}
    BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
    OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
  )
  darmok_compile_shader(
    TYPE FRAGMENT HEADER
    SHADERS ${FRAGMENT_PATH}
    VARYING_DEF ${VARYING_DEF_PATH}
    BASE_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/include
    OUTPUT_DIR ${SHADERS_OUTPUT_DIR}
  )
  list(APPEND SHADERS ${VERTEX_PATH} ${FRAGMENT_PATH} ${VARYING_DEF_PATH})

  addShaderVertexLayout(${NAME})

endmacro()

macro(addShaderInclude NAME)
  list(APPEND SHADER_INCLUDES "${SHADERS_DIR}/${NAME}.sh")
endmacro()

addShader("unlit")
addShader("forward_phong")
addShader("forward_pbr")
addShaderInclude("phong_lighting")
addShaderInclude("pbr_lighting")

set(SOURCES
  ${SRC_DIR}/window.cpp
  ${SRC_DIR}/platform.cpp
  ${SRC_DIR}/app.cpp
  ${SRC_DIR}/glfw.cpp
  ${SRC_DIR}/input.cpp
  ${SRC_DIR}/imgui.cpp
  ${SRC_DIR}/asset.cpp
  ${SRC_DIR}/scene.cpp
  ${SRC_DIR}/utils.cpp
  ${SRC_DIR}/anim.cpp
  ${SRC_DIR}/model.cpp
  ${SRC_DIR}/data.cpp
  ${SRC_DIR}/mesh.cpp
  ${SRC_DIR}/color.cpp
  ${SRC_DIR}/material.cpp
  ${SRC_DIR}/texture.cpp
  ${SRC_DIR}/image.cpp
  ${SRC_DIR}/program.cpp
  ${SRC_DIR}/transform.cpp
  ${SRC_DIR}/camera.cpp
  ${SRC_DIR}/light.cpp
  ${SRC_DIR}/vertex.cpp
  ${SRC_DIR}/render_forward.cpp
  ${SRC_DIR}/render_deferred.cpp
  ${SRC_DIR}/scripting.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/app.hpp
  ${INCLUDE_DIR}/input.hpp
  ${INCLUDE_DIR}/window.hpp
  ${INCLUDE_DIR}/utils.hpp
  ${INCLUDE_DIR}/asset.hpp
  ${INCLUDE_DIR}/imgui.hpp
  ${INCLUDE_DIR}/scene.hpp
  ${INCLUDE_DIR}/anim.hpp
  ${INCLUDE_DIR}/model.hpp
  ${INCLUDE_DIR}/data.hpp
  ${INCLUDE_DIR}/mesh.hpp
  ${INCLUDE_DIR}/color.hpp
  ${INCLUDE_DIR}/material.hpp
  ${INCLUDE_DIR}/optional_ref.hpp
  ${INCLUDE_DIR}/texture.hpp
  ${INCLUDE_DIR}/image.hpp
  ${INCLUDE_DIR}/program.hpp
  ${INCLUDE_DIR}/transform.hpp
  ${INCLUDE_DIR}/camera.hpp
  ${INCLUDE_DIR}/light.hpp
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/render_forward.hpp
  ${INCLUDE_DIR}/render_deferred.hpp
  ${INCLUDE_DIR}/scripting.hpp
)

set(LIB_NAME darmok)

add_library(
  ${LIB_NAME}
  ${HEADERS}
  ${SOURCES}
  ${SHADERS}
  ${SHADER_INCLUDES}
)

source_group("Shader Files" FILES ${SHADERS} ${SHADER_INCLUDES})

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
    ${LIBS_DIR}/rapidjson/include
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

# code analysis & clang tidy
if(DARMOK_RUN_CODE_ANALYSIS)
  set(CLANG_TIDY_CHECKS "-checks=-*,readability-*,modernize-*,-modernize-use-trailing-return-type")
  if(MSVC)
    set_target_properties(${LIB_NAME} PROPERTIES
        VS_GLOBAL_RunCodeAnalysis true
        VS_GLOBAL_EnableMicrosoftCodeAnalysis true
        VS_GLOBAL_CodeAnalysisRuleSet ${CMAKE_CURRENT_SOURCE_DIR}/darmok.ruleset
        VS_GLOBAL_EnableClangTidyCodeAnalysis true
        VS_GLOBAL_ClangTidyChecks ${CLANG_TIDY_CHECKS}
    )
  else()
    find_program(CLANG_TIDY_EXE
      NAMES "clang-tidy"
      NO_CACHE)
    set(CLANG_TIDY_COMMAND "${CLANG_TIDY_EXE}" "${CLANG_TIDY_CHECKS}")
    set_target_properties(${LIB_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_COMMAND}")
  endif()
endif()

# openGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

if(NOT DARMOK_PLATFORM)
    set(DARMOK_PLATFORM glfw)
endif()
if(NOT DARMOK_PLATFORM_SUPPORT_WAYLAND)
    if(UNIX AND NOT APPLE)
        set(DARMOK_PLATFORM_SUPPORT_WAYLAND 1)
    else()
        set(DARMOK_PLATFORM_SUPPORT_WAYLAND 0)
    endif()
endif()

# glfw
if(DARMOK_PLATFORM STREQUAL "glfw")
    option(GLFW_BUILD_EXAMPLES "Build the GLFW example programs" ${GLFW_STANDALONE})
    option(GLFW_BUILD_TESTS "Build the GLFW test programs" ${GLFW_STANDALONE})
    option(GLFW_BUILD_DOCS "Build the GLFW documentation" ON)
    set(GLFW_BUILD_EXAMPLES OFF)
    set(GLFW_BUILD_TESTS OFF)
    set(GLFW_BUILD_DOCS OFF)
    add_subdirectory(${LIBS_DIR}/glfw)
    target_link_libraries(${LIB_NAME} PRIVATE glfw)
    target_compile_definitions(darmok PRIVATE DARMOK_PLATFORM_GLFW)
    if (DARMOK_PLATFORM_SUPPORT_WAYLAND)
        target_compile_definitions(darmok PRIVATE DARMOK_PLATFORM_SUPPORT_WAYLAND)
        target_link_libraries(${LIB_NAME} PRIVATE wayland-egl)
        target_compile_definitions(bgfx PRIVATE WL_EGL_PLATFORM)
    endif()
endif()

# glm
add_subdirectory(${LIBS_DIR}/glm)
target_link_libraries(${LIB_NAME} PUBLIC glm)
target_compile_definitions(glm INTERFACE GLM_ENABLE_EXPERIMENTAL GLM_FORCE_LEFT_HANDED)
addExportTargets("glm" "glm;glm-header-only")

# bgfx
target_link_libraries(${LIB_NAME} PUBLIC bgfx bimg bimg_decode bx)

# imgui
include(${CMAKE_CURRENT_LIST_DIR}/CMake/imgui.cmake)
target_link_libraries(${LIB_NAME} PUBLIC imgui)

# entt
add_subdirectory(${LIBS_DIR}/entt)
target_link_libraries(${LIB_NAME} PUBLIC EnTT)

# assimp
OPTION(ASSIMP_BUILD_TESTS "If the test suite for Assimp is built in addition to the library." ON)
set(ASSIMP_BUILD_TESTS OFF)
add_subdirectory(${LIBS_DIR}/assimp)
target_link_libraries(${LIB_NAME} PRIVATE assimp)

# pugixml
add_subdirectory(${LIBS_DIR}/pugixml)
target_link_libraries(${LIB_NAME} PRIVATE pugixml)
target_compile_definitions(pugixml INTERFACE PUGIXML_HEADER_ONLY) # also included in assimp

# luajit
add_subdirectory(${LIBS_DIR}/luajit)


# sol2
add_subdirectory(${LIBS_DIR}/sol2)
target_link_libraries(${LIB_NAME} PRIVATE sol2)

# ozz
option(ozz_build_fbx "Build Fbx pipeline (Requires Fbx SDK)" ON)
option(ozz_build_gltf "Build glTF importer (Requires c++11)" ON)
option(ozz_build_samples "Build samples" ON)
option(ozz_build_howtos "Build howtos" ON)
option(ozz_build_tests "Build unit tests" ON)
option(ozz_build_msvc_rt_dll "Select msvc DLL runtime library" OFF)
set(ozz_build_fbx OFF)
set(ozz_build_gltf OFF)
set(ozz_build_samples OFF)
set(ozz_build_howtos OFF)
set(ozz_build_tests OFF)
set(ozz_build_msvc_rt_dll ON)
add_subdirectory(${LIBS_DIR}/ozz)
target_link_libraries(${LIB_NAME} PRIVATE
  ozz_geometry
  ozz_animation_offline
  ozz_options)
addExportTargets("ozz" "ozz_base;ozz_geometry;ozz_animation;ozz_animation_offline;ozz_options")
