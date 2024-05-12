cmake_minimum_required(VERSION 3.26)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

project(
  darmok
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "hobby C++ game engine"
)

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

option(DARMOK_BUILD_TESTS "build darmok unit tests" OFF)
option(DARMOK_BUILD_EXAMPLES "build darmok examples" OFF)
option(DARMOK_BUILD_SCRIPTING "build darmok with scripting support" OFF)
option(DARMOK_BUILD_SCRIPTING_EXE "build the darmok scripting executable" OFF)
option(DARMOK_BUILD_IMGUI "build darmok with imGUI support" OFF)
option(DARMOK_BUILD_RMLUI "build darmok with RmlUI support" OFF)
option(DARMOK_BUILD_ASSIMP "build darmok with assimp support" OFF)
option(DARMOK_BUILD_OZZ "build darmok with ozz animation support" OFF)
option(DARMOK_BUILD_JOLT "build darmok with jolt physics support" OFF)

set(SRC_DIR src)
set(INCLUDE_DIR include/darmok)
set(LIBS_DIR lib)

find_package(bgfx CONFIG REQUIRED)
include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/processAssets.cmake)

darmok_process_assets(
  HEADER_SHADER_OUTPUT_DIR "include/generated/shaders"
  HEADER_SHADER_INCLUDE_DIR "generated/shaders"
  ASSETS_EXCLUDES "shaders/cegui_*"
)

set(SOURCES
  ${SRC_DIR}/window.cpp
  ${SRC_DIR}/platform.cpp
  ${SRC_DIR}/app.cpp
  ${SRC_DIR}/glfw.cpp
  ${SRC_DIR}/input.cpp
  ${SRC_DIR}/asset.cpp
  ${SRC_DIR}/scene.cpp
  ${SRC_DIR}/utils.cpp
  ${SRC_DIR}/anim.cpp
  ${SRC_DIR}/data.cpp
  ${SRC_DIR}/shape.cpp
  ${SRC_DIR}/mesh.cpp
  ${SRC_DIR}/find.cpp
  ${SRC_DIR}/color.cpp
  ${SRC_DIR}/material.cpp
  ${SRC_DIR}/texture.cpp
  ${SRC_DIR}/texture_atlas.cpp
  ${SRC_DIR}/image.cpp
  ${SRC_DIR}/program.cpp
  ${SRC_DIR}/model.cpp
  ${SRC_DIR}/transform.cpp
  ${SRC_DIR}/camera.cpp
  ${SRC_DIR}/light.cpp
  ${SRC_DIR}/vertex.cpp
  ${SRC_DIR}/render_forward.cpp
  ${SRC_DIR}/render_deferred.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/app.hpp
  ${INCLUDE_DIR}/input_fwd.hpp
  ${INCLUDE_DIR}/input.hpp
  ${INCLUDE_DIR}/window.hpp
  ${INCLUDE_DIR}/utils.hpp
  ${INCLUDE_DIR}/collection.hpp
  ${INCLUDE_DIR}/asset.hpp
  ${INCLUDE_DIR}/scene_fwd.hpp
  ${INCLUDE_DIR}/scene.hpp
  ${INCLUDE_DIR}/entity_filter.hpp
  ${INCLUDE_DIR}/anim.hpp
  ${INCLUDE_DIR}/data.hpp
  ${INCLUDE_DIR}/shape.hpp
  ${INCLUDE_DIR}/mesh.hpp
  ${INCLUDE_DIR}/find.hpp
  ${INCLUDE_DIR}/color_fwd.hpp
  ${INCLUDE_DIR}/color.hpp
  ${INCLUDE_DIR}/material_fwd.hpp
  ${INCLUDE_DIR}/material.hpp
  ${INCLUDE_DIR}/optional_ref.hpp
  ${INCLUDE_DIR}/texture_fwd.hpp
  ${INCLUDE_DIR}/texture.hpp
  ${INCLUDE_DIR}/texture_atlas.hpp
  ${INCLUDE_DIR}/image.hpp
  ${INCLUDE_DIR}/program_fwd.hpp
  ${INCLUDE_DIR}/program.hpp
  ${INCLUDE_DIR}/transform.hpp
  ${INCLUDE_DIR}/skeleton.hpp
  ${INCLUDE_DIR}/camera.hpp
  ${INCLUDE_DIR}/light.hpp
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/render_forward.hpp
  ${INCLUDE_DIR}/render_deferred.hpp
  ${INCLUDE_DIR}/model.hpp
)

set(SCRIPTING_SOURCES)

if(DARMOK_BUILD_IMGUI)
  list(APPEND SOURCES ${SRC_DIR}/imgui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/imgui.hpp)
endif()

if(DARMOK_BUILD_ASSIMP)
  list(APPEND SOURCES ${SRC_DIR}/assimp.cpp)
  list(APPEND SOURCES ${SRC_DIR}/model_assimp.cpp)
endif()

if(DARMOK_BUILD_OZZ)
  list(APPEND SOURCES ${SRC_DIR}/skeleton_ozz.cpp)
endif()

if(DARMOK_BUILD_SCRIPTING)
  list(APPEND SCRIPTING_SOURCES
    ${SRC_DIR}/scripting/app.cpp
    ${SRC_DIR}/scripting/asset.cpp
    ${SRC_DIR}/scripting/image.cpp
    ${SRC_DIR}/scripting/material.cpp
    ${SRC_DIR}/scripting/mesh.cpp
    ${SRC_DIR}/scripting/program.cpp
    ${SRC_DIR}/scripting/texture.cpp
    ${SRC_DIR}/scripting/input.cpp
    ${SRC_DIR}/scripting/math1.cpp
    ${SRC_DIR}/scripting/math2.cpp
    ${SRC_DIR}/scripting/math3.cpp
    ${SRC_DIR}/scripting/shape.cpp
    ${SRC_DIR}/scripting/scene.cpp
    ${SRC_DIR}/scripting/transform.cpp
    ${SRC_DIR}/scripting/light.cpp
    ${SRC_DIR}/scripting/camera.cpp
    ${SRC_DIR}/scripting/window.cpp
    ${SRC_DIR}/scripting/model.cpp
  )
  list(APPEND HEADERS ${INCLUDE_DIR}/scripting.hpp)
endif()

if(DARMOK_BUILD_SCRIPTING)
  list(APPEND SOURCES ${SCRIPTING_SOURCES})
  source_group("Scripting Source Files" FILES ${SCRIPTING_SOURCES})
endif()

set(LIB_NAME darmok)

add_library(
  ${LIB_NAME}
  ${HEADERS}
  ${SOURCES}
  ${ASSETS}
)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    ${CMAKE_CURRENT_BINARY_DIR}/include
    ${BGFX_INCLUDE_DIR}/3rdparty/
    ${BGFX_INCLUDE_DIR}/bgfx/examples/common
)

set_target_properties(${LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

# testing
if(DARMOK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
endif()

if(MSVC)
    SET(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(BUILD_SHARED_LIBS OFF)

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;Distribution")

set(CMAKE_CXX_FLAGS_DISTRIBUTION "/GS- /Gy /O2 /Oi /Ot")
set(CMAKE_EXE_LINKER_FLAGS_DISTRIBUTION "${CMAKE_EXE_LINKER_FLAGS_RELEASE}")

function(SET_WARNING_LEVEL)
  if(MSVC)
    target_compile_options(${ARGV0} PRIVATE /W4)
  else()
    target_compile_options(${ARGV0} PRIVATE -Wall -Wextra -Wpedantic -Wno-missing-field-initializers)
  endif()
endfunction()

function(SET_DEBUGGER_WORKING_DIRECTORY)
  set_target_properties(${ARGV0} PROPERTIES
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/")
endfunction()

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
    find_package(glfw3 CONFIG REQUIRED)
    target_link_libraries(${LIB_NAME} PRIVATE glfw)
    target_compile_definitions(darmok PRIVATE DARMOK_PLATFORM_GLFW)
    if (DARMOK_PLATFORM_SUPPORT_WAYLAND)
        target_compile_definitions(darmok PRIVATE DARMOK_PLATFORM_SUPPORT_WAYLAND)
        target_link_libraries(${LIB_NAME} PRIVATE wayland-egl)
        target_compile_definitions(bgfx PRIVATE WL_EGL_PLATFORM)
    endif()
endif()

# glm
find_package(glm CONFIG REQUIRED)
set(GLM_DEFINES GLM_ENABLE_EXPERIMENTAL GLM_FORCE_LEFT_HANDED)
target_compile_definitions(glm::glm-header-only INTERFACE ${GLM_DEFINES})
target_link_libraries(${LIB_NAME} PUBLIC glm::glm-header-only)

# bgfx
target_link_libraries(${LIB_NAME} PUBLIC bgfx::bx bgfx::bgfx bgfx::bimg bgfx::bimg_decode)

# imgui
if(DARMOK_BUILD_IMGUI)
  find_package(imgui CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC imgui::imgui)
endif()

# entt
find_package(entt CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PUBLIC EnTT::EnTT)

# assimp
if(DARMOK_BUILD_ASSIMP)
  find_package(assimp CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC assimp::assimp)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_ASSIMP)
endif()

# pugixml
find_package(pugixml CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PRIVATE pugixml::pugixml)
target_compile_definitions(pugixml INTERFACE PUGIXML_HEADER_ONLY) # also included in assimp

# json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PRIVATE nlohmann_json::nlohmann_json)

# cereal
find_package(cereal CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PRIVATE cereal::cereal)

if(DARMOK_BUILD_JOLT)
  find_package(unofficial-joltphysics CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE unofficial::joltphysics::Jolt)
endif()

# lua sol2
if(DARMOK_BUILD_SCRIPTING)
  find_package(Lua REQUIRED)
  target_include_directories(${LIB_NAME} PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(${LIB_NAME} PRIVATE ${LUA_LIBRARIES})

  # TODO: check how to use luajit
  #find_package(LuaJit REQUIRED)
  #target_include_directories(main PRIVATE ${LUAJIT_INCLUDE_DIR})
  #target_link_libraries(main PRIVATE ${LUAJIT_LIBRARIES})

  find_package(sol2 CONFIG REQUIRED)
  set(SOL_DEFINES SOL_ALL_SAFETIES_ON SOL_EXCEPTIONS_SAFE_PROPAGATION)
  target_compile_definitions(sol2 INTERFACE ${SOL_DEFINES})
  target_link_libraries(${LIB_NAME} PRIVATE sol2)


endif()
if(DARMOK_BUILD_SCRIPTING_EXE)
  set(SCRIPTING_EXE_NAME ${LIB_NAME}-scripting)
  add_executable(
    ${SCRIPTING_EXE_NAME}
    src/scripting/main.cpp
  )
  target_compile_definitions(
    ${SCRIPTING_EXE_NAME}
    PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
  )
  target_link_libraries(${SCRIPTING_EXE_NAME} LINK_PUBLIC ${LIB_NAME})
endif()

# ozz
if(DARMOK_BUILD_OZZ)
  include(${CMAKE_CURRENT_SOURCE_DIR}/CMake/configureOzzAnimation.cmake)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_OZZ)
  target_link_libraries(${LIB_NAME} PRIVATE
    ozz_geometry
    ozz_animation
    ozz_animation_offline
    ozz_options)
endif()