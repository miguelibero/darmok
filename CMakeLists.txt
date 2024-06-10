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

if(MSVC)
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
endif()

if(MSVC AND BUILD_SHARED_LIBS)
  set(DLLEXPORT "__declspec(dllexport)")
else()
  set(DLLEXPORT "")
endif()

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(ADD_OPTIM_FLAGS "")
if(MSVC)
  set(ADD_OPTIM_FLAGS "/GS- /Gy /O2 /Oi /Ot")
endif()

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} ${ADD_OPTIM_FLAGS}")
set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} ${ADD_OPTIM_FLAGS}")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} ${ADD_OPTIM_FLAGS}")
set(CMAKE_C_FLAGS_RELWITHDEBINFO "${CMAKE_C_FLAGS_RELWITHDEBINFO} ${ADD_OPTIM_FLAGS}")

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

option(DARMOK_BUILD_SAMPLES "build darmok samples" OFF)
option(DARMOK_BUILD_TESTS "build darmok unit tests" OFF)
option(DARMOK_BUILD_SCRIPTING "build darmok with scripting support" OFF)
option(DARMOK_BUILD_SCRIPTING_EXE "build the darmok scripting executable" OFF)
option(DARMOK_BUILD_IMGUI "build darmok with imGUI support" OFF)
option(DARMOK_BUILD_RMLUI "build darmok with RmlUI support" OFF)
option(DARMOK_BUILD_ASSIMP "build darmok with assimp support" OFF)
option(DARMOK_BUILD_OZZ "build darmok with ozz animation support" OFF)
option(DARMOK_BUILD_JOLT "build darmok with jolt physics support" OFF)

set(SRC_DIR src)
set(INCLUDE_BASE_DIR include)
set(INCLUDE_DIR ${INCLUDE_BASE_DIR}/darmok)
set(LOCAL_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

list(APPEND CMAKE_MODULE_PATH ${LOCAL_MODULE_PATH})

find_package(bgfx CONFIG REQUIRED)

include(darmokProcessAssets)

darmok_process_assets(
  ASSETS "assets/shaders"
  HEADER_OUTPUT_DIR "include/generated/shaders"
  HEADER_SHADER_INCLUDE_DIR "generated/shaders"
  SOURCE_GROUP_NAME "Embedded Shader Files"
)

set(CORE_SOURCES
  ${SRC_DIR}/string.cpp
  ${SRC_DIR}/data.cpp
  ${SRC_DIR}/data_stream.cpp
  ${SRC_DIR}/utils.cpp
  ${SRC_DIR}/math.cpp
  ${SRC_DIR}/color.cpp
  ${SRC_DIR}/image.cpp
  ${SRC_DIR}/shape.cpp
  ${SRC_DIR}/texture.cpp
  ${SRC_DIR}/texture_atlas.cpp
  ${SRC_DIR}/mesh.cpp
  ${SRC_DIR}/vertex.cpp
  ${SRC_DIR}/vertex_layout.cpp

  ${SRC_DIR}/app.cpp
  ${SRC_DIR}/window.cpp
  ${SRC_DIR}/input.cpp
  ${SRC_DIR}/scene.cpp
  ${SRC_DIR}/anim.cpp
  ${SRC_DIR}/material.cpp
  ${SRC_DIR}/program.cpp
  ${SRC_DIR}/model.cpp
  ${SRC_DIR}/transform.cpp
  ${SRC_DIR}/skeleton.cpp
  ${SRC_DIR}/viewport.cpp
  ${SRC_DIR}/camera.cpp
  ${SRC_DIR}/light.cpp
  ${SRC_DIR}/render.cpp
  ${SRC_DIR}/render_forward.cpp
  ${SRC_DIR}/render_deferred.cpp
)

set(CORE_HEADERS
  ${INCLUDE_DIR}/utils.hpp
  ${INCLUDE_DIR}/string.hpp
  ${INCLUDE_DIR}/optional_ref.hpp
  ${INCLUDE_DIR}/collection.hpp
  ${INCLUDE_DIR}/data.hpp
  ${INCLUDE_DIR}/data_stream.hpp
  ${INCLUDE_DIR}/math.hpp
  ${INCLUDE_DIR}/shape.hpp
  ${INCLUDE_DIR}/mesh_fwd.hpp
  ${INCLUDE_DIR}/mesh.hpp
  ${INCLUDE_DIR}/color_fwd.hpp
  ${INCLUDE_DIR}/color.hpp
  ${INCLUDE_DIR}/image.hpp
  ${INCLUDE_DIR}/texture_fwd.hpp
  ${INCLUDE_DIR}/texture.hpp
  ${INCLUDE_DIR}/texture_atlas.hpp
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/vertex_layout.hpp
  ${INCLUDE_DIR}/ext_loader.hpp

  ${INCLUDE_DIR}/app_fwd.hpp
  ${INCLUDE_DIR}/app.hpp
  ${INCLUDE_DIR}/input_fwd.hpp
  ${INCLUDE_DIR}/input.hpp
  ${INCLUDE_DIR}/window.hpp
  ${INCLUDE_DIR}/collection.hpp
  ${INCLUDE_DIR}/scene_fwd.hpp
  ${INCLUDE_DIR}/scene.hpp
  ${INCLUDE_DIR}/entity_filter.hpp
  ${INCLUDE_DIR}/anim.hpp
  ${INCLUDE_DIR}/material_fwd.hpp
  ${INCLUDE_DIR}/material.hpp
  ${INCLUDE_DIR}/program_fwd.hpp
  ${INCLUDE_DIR}/program.hpp
  ${INCLUDE_DIR}/transform.hpp
  ${INCLUDE_DIR}/skeleton.hpp
  ${INCLUDE_DIR}/viewport.hpp
  ${INCLUDE_DIR}/camera.hpp
  ${INCLUDE_DIR}/light.hpp
  ${INCLUDE_DIR}/render.hpp
  ${INCLUDE_DIR}/render_forward.hpp
  ${INCLUDE_DIR}/render_deferred.hpp
  ${INCLUDE_DIR}/model.hpp
)

set(SOURCES
  ${SRC_DIR}/platform.cpp
  ${SRC_DIR}/program_standard.cpp
  ${SRC_DIR}/asset.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/program_standard.hpp
  ${INCLUDE_DIR}/asset.hpp
)

set(SCRIPTING_SOURCES)

if(NOT DARMOK_PLATFORM)
    set(DARMOK_PLATFORM glfw)
endif()
  if(UNIX AND NOT APPLE)
      set(DARMOK_PLATFORM_SUPPORT_WAYLAND 1)
  else()
      set(DARMOK_PLATFORM_SUPPORT_WAYLAND 0)
  endif()

if(DARMOK_PLATFORM STREQUAL "glfw")
  list(APPEND SOURCES ${SRC_DIR}/glfw.cpp)
endif()

if(DARMOK_BUILD_IMGUI)
  list(APPEND SOURCES ${SRC_DIR}/imgui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/imgui.hpp)

  darmok_process_assets(
    ASSETS "assets/imgui"
    HEADER_VAR_PREFIX "imgui_"
    HEADER_OUTPUT_DIR "include/generated/imgui/shaders"
    HEADER_SHADER_INCLUDE_DIR "generated/imgui/shaders"
    SOURCE_GROUP_NAME "Imgui Asset Files"
    SOURCES_VAR IMGUI_ASSETS
  )
  list(APPEND ASSETS ${IMGUI_ASSETS})
endif()

if(DARMOK_BUILD_RMLUI)
  list(APPEND SOURCES ${SRC_DIR}/rmlui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/rmlui.hpp)

  if(DARMOK_BUILD_SCRIPTING)
    list(APPEND SCRIPTING_SOURCES ${SRC_DIR}/scripting/rmlui.cpp)
  endif()

  darmok_process_assets(
    ASSETS "assets/rmlui"
    HEADER_VAR_PREFIX "rmlui_"
    HEADER_OUTPUT_DIR "include/generated/rmlui/shaders"
    HEADER_SHADER_INCLUDE_DIR "generated/rmlui/shaders"
    SOURCE_GROUP_NAME "RmlUI Asset Files"
    SOURCES_VAR RMLUI_ASSETS
  )
  list(APPEND ASSETS ${RMLUI_ASSETS})

endif()

if(DARMOK_BUILD_OZZ)
  list(APPEND HEADERS ${INCLUDE_DIR}/skeleton_ozz.hpp)
  list(APPEND SOURCES ${SRC_DIR}/skeleton_ozz.cpp)
endif()

if(DARMOK_BUILD_ASSIMP)
  list(APPEND HEADERS ${INCLUDE_DIR}/model_assimp.hpp)
  list(APPEND SOURCES ${SRC_DIR}/model_assimp.cpp)
endif()

if(DARMOK_BUILD_SCRIPTING_EXE)
  set(DARMOK_BUILD_SCRIPTING ON CACHE BOOL "")
endif()

if(DARMOK_BUILD_SCRIPTING)
  list(APPEND SCRIPTING_SOURCES
    ${SRC_DIR}/scripting/app.cpp
    ${SRC_DIR}/scripting/asset.cpp
    ${SRC_DIR}/scripting/image.cpp
    ${SRC_DIR}/scripting/material.cpp
    ${SRC_DIR}/scripting/render.cpp
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
    ${SRC_DIR}/scripting/viewport.cpp
    ${SRC_DIR}/scripting/camera.cpp
    ${SRC_DIR}/scripting/window.cpp
    ${SRC_DIR}/scripting/model.cpp
    ${SRC_DIR}/scripting/utils.cpp
    ${SRC_DIR}/scripting/skeleton.cpp
  )
  list(APPEND HEADERS
    ${INCLUDE_DIR}/scripting.hpp
  )
  source_group("Scripting Source Files" FILES ${SCRIPTING_SOURCES})
endif()

set(CORE_LIB_NAME "darmok-core")
add_library(
  ${CORE_LIB_NAME}
  ${CORE_SOURCES}
)
target_sources(${CORE_LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${CORE_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} PREFIX "Source Files" FILES ${CORE_SOURCES})
add_library(darmok::darmok-core ALIAS ${CORE_LIB_NAME})

set(LIB_NAME "darmok")
add_library(
  ${LIB_NAME}
  ${SOURCES}
  ${SCRIPTING_SOURCES}
  ${ASSETS}
)
target_sources(${LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} PREFIX "Source Files" FILES ${SOURCES})
add_library(darmok::darmok ALIAS ${LIB_NAME})
target_link_libraries(${LIB_NAME} PUBLIC ${CORE_LIB_NAME})

set(EXPORT_NAME "${PROJECT_NAME}Targets")
set(EXPORT_INSTALL_DIR "share/${PROJECT_NAME}")
install(TARGETS ${LIB_NAME} ${CORE_LIB_NAME}
    EXPORT ${EXPORT_NAME}
    RUNTIME_DEPENDENCY_SET ${LIB_NAME}
    FILE_SET HEADERS
)
install(EXPORT ${EXPORT_NAME}
  NAMESPACE darmok
  DESTINATION ${EXPORT_INSTALL_DIR}
)
install(FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/README.md
  ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md
  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  DESTINATION share/doc/darmok
)

macro(_darmok_lib_setup TARGET)
  target_include_directories(
    ${TARGET}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
  )
  set_target_properties(${TARGET} PROPERTIES VERSION ${PROJECT_VERSION})
  target_compile_definitions(${TARGET} PUBLIC "DLLEXPORT=${DLLEXPORT}")
  set_target_properties(${TARGET} PROPERTIES FOLDER ${PROJECT_NAME})
endmacro()

_darmok_lib_setup(${LIB_NAME})
_darmok_lib_setup(${CORE_LIB_NAME})

# tools
set(TOOLS_FOLDER ${PROJECT_NAME}/tools)

# vlayoutc: only depends on darmok-core to avoid dependency cycle
# since it is used to generate standard darmok shaders
set(VLAYOUTC_NAME "darmok-vlayoutc")
set(VLAYOUTC_SRC_DIR "tools/vlayoutc")
add_executable(${VLAYOUTC_NAME}
  ${VLAYOUTC_SRC_DIR}/vlayoutc.cpp
)
target_link_libraries(${VLAYOUTC_NAME} ${CORE_LIB_NAME})
add_executable(darmok::vlayoutc ALIAS ${VLAYOUTC_NAME})
set_target_properties(${VLAYOUTC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})

# samples
if(DARMOK_BUILD_SAMPLES)
  list(APPEND VCPKG_MANIFEST_FEATURES "samples")
  add_subdirectory(samples)
endif()

# testing
if(DARMOK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
endif()

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

# bgfx
target_link_libraries(${CORE_LIB_NAME} PUBLIC bgfx::bx bgfx::bgfx bgfx::bimg bgfx::bimg_decode)

# pugixml
find_package(pugixml CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC pugixml::pugixml)

# json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC nlohmann_json::nlohmann_json)

# tween
find_package(Tweeny CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC tweeny)

# cereal
find_package(cereal CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC cereal::cereal)

# glm
find_package(glm CONFIG REQUIRED)
set(GLM_DEFINES GLM_ENABLE_EXPERIMENTAL GLM_FORCE_LEFT_HANDED)
target_compile_definitions(glm::glm-header-only INTERFACE ${GLM_DEFINES})
target_link_libraries(${CORE_LIB_NAME} PUBLIC glm::glm-header-only)

# glfw
if(DARMOK_PLATFORM STREQUAL "glfw")
  find_package(glfw3 CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE glfw)
  target_compile_definitions(${LIB_NAME} PRIVATE DARMOK_PLATFORM_GLFW)
  if (DARMOK_PLATFORM_SUPPORT_WAYLAND)
    target_compile_definitions(${LIB_NAME} PRIVATE DARMOK_PLATFORM_SUPPORT_WAYLAND)
    target_link_libraries(${LIB_NAME} PRIVATE wayland-egl)
    target_compile_definitions(bgfx::bgfx INTERFACE WL_EGL_PLATFORM)
  endif()
endif()

# imgui
if(DARMOK_BUILD_IMGUI)
  find_package(imgui CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC imgui::imgui)
endif()

# imgui
if(DARMOK_BUILD_RMLUI)
  find_package(RmlUi CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC RmlCore)
  target_link_libraries(${LIB_NAME} PRIVATE debug RmlDebugger)
  if(DARMOK_BUILD_SCRIPTING)
    target_link_libraries(${LIB_NAME} PRIVATE RmlLua)
  endif()
endif()

# entt
find_package(EnTT CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PUBLIC EnTT::EnTT)

# assimp
if(DARMOK_BUILD_ASSIMP)
  find_package(assimp CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE assimp::assimp)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_ASSIMP)

  set(MODELC_NAME "darmok-modelc")
  set(MODELC_SRC_DIR "tools/modelc")
  add_executable(${MODELC_NAME}
    ${MODELC_SRC_DIR}/modelc.cpp
  )

  target_link_libraries(${MODELC_NAME} ${LIB_NAME})
  add_executable(darmok::modelc ALIAS ${MODELC_NAME})
  set_target_properties(${MODELC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})
endif()

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
  list(APPEND VCPKG_MANIFEST_FEATURES "scripting")
  add_library(sol2::sol2 ALIAS sol2)
endif()

if(DARMOK_BUILD_SCRIPTING_EXE)
  set(SCRIPTING_EXE_NAME darmok-scripting)
  add_executable(
    ${SCRIPTING_EXE_NAME}
    src/scripting/main.cpp
  )
  add_executable(darmok::scripting ALIAS ${SCRIPTING_EXE_NAME})
  target_compile_definitions(
    ${SCRIPTING_EXE_NAME}
    PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
  )
  set_target_properties(${SCRIPTING_EXE_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
  target_link_libraries(${SCRIPTING_EXE_NAME} ${LIB_NAME})
  install(TARGETS ${SCRIPTING_EXE_NAME}
    EXPORT ${EXPORT_NAME}
    RUNTIME_DEPENDENCY_SET ${LIB_NAME}
  )
  install(RUNTIME_DEPENDENCY_SET ${LIB_NAME}
    PRE_EXCLUDE_REGEXES "api-ms-" "ext-ms-" "ozz_.*\\.dll"
    POST_EXCLUDE_REGEXES ".*system32/.*\\.dll"    
  )
endif()

# ozz
if(DARMOK_BUILD_OZZ)
  set(OzzAnimation_DIR ${LOCAL_MODULE_PATH})
  find_package(OzzAnimation CONFIG REQUIRED)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_OZZ)
  target_link_libraries(${LIB_NAME} PRIVATE
    ozz_geometry
    ozz_animation
    ozz_animation_offline
    ozz_options)
endif()