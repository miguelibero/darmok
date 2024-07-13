cmake_minimum_required(VERSION 3.26)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

set(VCPKG_BINARY_TARGET_DIR vcpkg_installed/${VCPKG_TARGET_TRIPLET})

project(
  darmok
  VERSION 0.1.1
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
option(DARMOK_BUILD_LUA "build darmok with lua support" OFF)
option(DARMOK_BUILD_LUA_EXE "build the darmok lua executable" OFF)
option(DARMOK_BUILD_FREETYPE "build darmok with freetype support" OFF)
option(DARMOK_BUILD_IMGUI "build darmok with imGUI support" OFF)
option(DARMOK_BUILD_RMLUI "build darmok with RmlUI support" OFF)
option(DARMOK_BUILD_ASSIMP "build darmok with assimp support" OFF)
option(DARMOK_BUILD_OZZ "build darmok with ozz animation support" OFF)
option(DARMOK_BUILD_JOLT "build darmok with jolt physics support" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_BASE_DIR ${PROJECT_SOURCE_DIR}/include)
set(INCLUDE_DIR ${INCLUDE_BASE_DIR}/darmok)
set(LOCAL_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

list(APPEND CMAKE_MODULE_PATH 
  ${LOCAL_MODULE_PATH} ${LOCAL_MODULE_PATH}/lib)

find_package(bgfx CONFIG REQUIRED)

include(darmokProcessAssets)

set(SHADERS_TARGET darmok-shaders)
set(SHADERS_DIR ${PROJECT_SOURCE_DIR}/assets/shaders)
set(SHADERS
  ${SHADERS_DIR}/darmok.include.sc
  ${SHADERS_DIR}/darmok_compute.include.sc
  ${SHADERS_DIR}/darmok_pbr_lighting.include.sc
  ${SHADERS_DIR}/darmok_phong_lighting.include.sc
  ${SHADERS_DIR}/darmok_skinning.include.sc
  ${SHADERS_DIR}/darmok-import.json
  ${SHADERS_DIR}/forward_pbr.fragment.sc
  ${SHADERS_DIR}/forward_pbr.varyingdef
  ${SHADERS_DIR}/forward_pbr.vertex.sc
  ${SHADERS_DIR}/forward_pbr.vlayout.json
  ${SHADERS_DIR}/forward_phong.fragment.sc
  ${SHADERS_DIR}/forward_phong.varyingdef
  ${SHADERS_DIR}/forward_phong.vertex.sc
  ${SHADERS_DIR}/forward_phong.vlayout.json
  ${SHADERS_DIR}/gui.fragment.sc
  ${SHADERS_DIR}/gui.varyingdef
  ${SHADERS_DIR}/gui.vertex.sc
  ${SHADERS_DIR}/gui.vlayout.json
  ${SHADERS_DIR}/pbr.include.sc
  ${SHADERS_DIR}/unlit.fragment.sc
  ${SHADERS_DIR}/unlit.varyingdef
  ${SHADERS_DIR}/unlit.vertex.sc
  ${SHADERS_DIR}/unlit.vlayout.json
)
darmok_process_assets(
  TARGET ${SHADERS_TARGET} CORE
  INPUT ${SHADERS_DIR}
  OUTPUT include/private/generated/shaders
  ASSETS ${SHADERS}
)
set_target_properties(${SHADERS_TARGET} PROPERTIES FOLDER "darmok")
set(ASSET_TARGETS ${SHADERS_TARGET})

set(CORE_SOURCES
  ${SRC_DIR}/string.cpp
  ${SRC_DIR}/utf8.cpp
  ${SRC_DIR}/stream.cpp
  ${SRC_DIR}/data.cpp
  ${SRC_DIR}/data_stream.cpp
  ${SRC_DIR}/utils.cpp
  ${SRC_DIR}/math.cpp
  ${SRC_DIR}/color.cpp
  ${SRC_DIR}/image.cpp
  ${SRC_DIR}/shape.cpp
  ${SRC_DIR}/texture.cpp
  ${SRC_DIR}/mesh.cpp
  ${SRC_DIR}/vertex.cpp
  ${SRC_DIR}/vertex_layout.cpp
  ${SRC_DIR}/viewport.cpp
  ${SRC_DIR}/program.cpp
  ${SRC_DIR}/asset_core.cpp
  ${SRC_DIR}/render_graph.cpp
)

set(CORE_HEADERS
  ${INCLUDE_DIR}/glm.hpp
  ${INCLUDE_DIR}/utils.hpp
  ${INCLUDE_DIR}/string.hpp
  ${INCLUDE_DIR}/utf8.hpp
  ${INCLUDE_DIR}/stream.hpp
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
  ${INCLUDE_DIR}/image_fwd.hpp
  ${INCLUDE_DIR}/image.hpp
  ${INCLUDE_DIR}/texture_fwd.hpp
  ${INCLUDE_DIR}/texture.hpp
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/vertex_layout.hpp
  ${INCLUDE_DIR}/collection.hpp
  ${INCLUDE_DIR}/viewport.hpp
  ${INCLUDE_DIR}/program_fwd.hpp
  ${INCLUDE_DIR}/program.hpp
  ${INCLUDE_DIR}/asset_core.hpp
  ${INCLUDE_DIR}/loader.hpp
  ${INCLUDE_DIR}/render_graph.hpp
)

set(SOURCES
  ${SRC_DIR}/platform.cpp
  ${SRC_DIR}/asset.cpp

  ${SRC_DIR}/app.cpp
  ${SRC_DIR}/window.cpp
  ${SRC_DIR}/input.cpp
  ${SRC_DIR}/scene.cpp

  ${SRC_DIR}/texture_atlas.cpp
  ${SRC_DIR}/material.cpp
  ${SRC_DIR}/model.cpp
  ${SRC_DIR}/anim.cpp

  ${SRC_DIR}/transform.cpp
  ${SRC_DIR}/camera.cpp
  ${SRC_DIR}/light.cpp
  ${SRC_DIR}/render.cpp
  ${SRC_DIR}/render_forward.cpp
  ${SRC_DIR}/render_deferred.cpp
  ${SRC_DIR}/freelook.cpp
  ${SRC_DIR}/text.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/asset.hpp

  ${INCLUDE_DIR}/app_fwd.hpp
  ${INCLUDE_DIR}/app.hpp
  ${INCLUDE_DIR}/input_fwd.hpp
  ${INCLUDE_DIR}/input.hpp
  ${INCLUDE_DIR}/window.hpp
  ${INCLUDE_DIR}/scene_fwd.hpp
  ${INCLUDE_DIR}/scene.hpp

  ${INCLUDE_DIR}/texture_atlas.hpp
  ${INCLUDE_DIR}/anim.hpp
  ${INCLUDE_DIR}/material_fwd.hpp
  ${INCLUDE_DIR}/material.hpp
  ${INCLUDE_DIR}/model_fwd.hpp
  ${INCLUDE_DIR}/model.hpp

  ${INCLUDE_DIR}/entity_filter.hpp
  ${INCLUDE_DIR}/transform.hpp
  ${INCLUDE_DIR}/camera.hpp
  ${INCLUDE_DIR}/light.hpp
  ${INCLUDE_DIR}/render.hpp
  ${INCLUDE_DIR}/render_forward.hpp
  ${INCLUDE_DIR}/render_deferred.hpp
  ${INCLUDE_DIR}/freelook.hpp
  ${INCLUDE_DIR}/text_fwd.hpp
  ${INCLUDE_DIR}/text.hpp
)

set(LUA_SOURCES)

if(NOT DARMOK_PLATFORM)
    set(DARMOK_PLATFORM glfw)
endif()
  if(UNIX AND NOT APPLE)
      set(DARMOK_PLATFORM_SUPPORT_WAYLAND 1)
  else()
      set(DARMOK_PLATFORM_SUPPORT_WAYLAND 0)
  endif()

if(DARMOK_PLATFORM STREQUAL "glfw")
  list(APPEND SOURCES ${SRC_DIR}/platform_glfw.cpp)
endif()

if(DARMOK_BUILD_FREETYPE)
  list(APPEND HEADERS
    ${INCLUDE_DIR}/text_freetype.hpp
  )
  list(APPEND SOURCES
    ${SRC_DIR}/text_freetype.cpp
  )
  if(DARMOK_BUILD_LUA)
    list(APPEND LUA_SOURCES
      ${SRC_DIR}/lua/text.cpp
    )
  endif()
endif()

if(DARMOK_BUILD_IMGUI)
  list(APPEND SOURCES ${SRC_DIR}/imgui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/imgui.hpp)

  set(IMGUI_SHADERS_TARGET darmok-imgui-shaders)
  set(IMGUI_SHADERS_DIR ${PROJECT_SOURCE_DIR}/assets/imgui/shaders)
  set(IMGUI_SHADERS
    ${IMGUI_SHADERS_DIR}/basic.fragment.sc
    ${IMGUI_SHADERS_DIR}/basic.varyingdef
    ${IMGUI_SHADERS_DIR}/basic.vertex.sc
    ${IMGUI_SHADERS_DIR}/darmok-import.json
    ${IMGUI_SHADERS_DIR}/lod.fragment.sc
    ${IMGUI_SHADERS_DIR}/lod.varyingdef
    ${IMGUI_SHADERS_DIR}/lod.vertex.sc
  )
  darmok_process_assets(
    TARGET ${IMGUI_SHADERS_TARGET} CORE
    INPUT ${IMGUI_SHADERS_DIR}
    OUTPUT include/private/generated/imgui/shaders
    ASSETS ${IMGUI_SHADERS}
  )
  set_target_properties(${IMGUI_SHADERS_TARGET} PROPERTIES FOLDER "darmok")
  list(APPEND ASSET_TARGETS ${IMGUI_SHADERS_TARGET})
endif()

if(DARMOK_BUILD_RMLUI)
  list(APPEND SOURCES ${SRC_DIR}/rmlui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/rmlui.hpp)

  if(DARMOK_BUILD_LUA)
    list(APPEND LUA_SOURCES ${SRC_DIR}/lua/rmlui.cpp)
  endif()
  set(RMLUI_SHADERS_TARGET darmok-rmlui-shaders)
  set(RMLUI_SHADERS_DIR ${PROJECT_SOURCE_DIR}/assets/rmlui/shaders)
  set(RMLUI_SHADERS
    ${RMLUI_SHADERS_DIR}/basic.fragment.sc
    ${RMLUI_SHADERS_DIR}/basic.varyingdef
    ${RMLUI_SHADERS_DIR}/basic.vertex.sc
    ${RMLUI_SHADERS_DIR}/basic.vlayout.json
    ${RMLUI_SHADERS_DIR}/darmok-import.json
    ${RMLUI_SHADERS_DIR}/solid.fragment.sc
    ${RMLUI_SHADERS_DIR}/solid.varyingdef
    ${RMLUI_SHADERS_DIR}/solid.vertex.sc
  )
  darmok_process_assets(
    TARGET ${RMLUI_SHADERS_TARGET} CORE
    INPUT ${RMLUI_SHADERS_DIR}
    OUTPUT include/private/generated/rmlui/shaders
    ASSETS ${RMLUI_SHADERS}
  )
  set_target_properties(${RMLUI_SHADERS_TARGET} PROPERTIES FOLDER "darmok")
  list(APPEND ASSET_TARGETS ${RMLUI_SHADERS_TARGET})
endif()

if(DARMOK_BUILD_OZZ)
  list(APPEND HEADERS
    ${INCLUDE_DIR}/skeleton.hpp
    ${INCLUDE_DIR}/skeleton_ozz.hpp
  )
  list(APPEND SOURCES
    ${SRC_DIR}/skeleton.cpp  
    ${SRC_DIR}/skeleton_ozz.cpp
  )

  if(DARMOK_BUILD_LUA)
    list(APPEND LUA_SOURCES ${SRC_DIR}/lua/skeleton.cpp)
  endif()
endif()

if(DARMOK_BUILD_ASSIMP)
  list(APPEND HEADERS
    ${INCLUDE_DIR}/model_assimp.hpp
  )
  list(APPEND SOURCES ${SRC_DIR}/model_assimp.cpp)
  if(DARMOK_BUILD_OZZ)
    list(APPEND HEADERS ${INCLUDE_DIR}/skeleton_assimp.hpp)
    list(APPEND SOURCES ${SRC_DIR}/skeleton_assimp.cpp)
  endif()  
endif()

if(DARMOK_BUILD_JOLT)
  list(APPEND HEADERS
    ${INCLUDE_DIR}/physics3d_fwd.hpp
    ${INCLUDE_DIR}/physics3d.hpp
    ${INCLUDE_DIR}/physics3d_debug.hpp
    ${INCLUDE_DIR}/character_fwd.hpp
    ${INCLUDE_DIR}/character.hpp
  )
  list(APPEND SOURCES
    ${SRC_DIR}/physics3d_jolt.cpp
    ${SRC_DIR}/physics3d_debug_jolt.cpp
    ${SRC_DIR}/character_jolt.cpp
  )
  if(DARMOK_BUILD_LUA)
    list(APPEND LUA_SOURCES
      ${SRC_DIR}/lua/physics3d.cpp
      ${SRC_DIR}/lua/physics3d_debug.cpp
      ${SRC_DIR}/lua/character.cpp
    )
  endif()
endif()

if(DARMOK_BUILD_LUA_EXE)
  set(DARMOK_BUILD_LUA ON CACHE BOOL "")
endif()

if(DARMOK_BUILD_LUA)
  list(APPEND LUA_SOURCES
    ${SRC_DIR}/lua/app.cpp
    ${SRC_DIR}/lua/asset.cpp
    ${SRC_DIR}/lua/image.cpp
    ${SRC_DIR}/lua/material.cpp
    ${SRC_DIR}/lua/render.cpp
    ${SRC_DIR}/lua/render_forward.cpp
    ${SRC_DIR}/lua/mesh.cpp
    ${SRC_DIR}/lua/program.cpp
    ${SRC_DIR}/lua/texture.cpp
    ${SRC_DIR}/lua/input.cpp
    ${SRC_DIR}/lua/math1.cpp
    ${SRC_DIR}/lua/math2.cpp
    ${SRC_DIR}/lua/math3.cpp
    ${SRC_DIR}/lua/shape.cpp
    ${SRC_DIR}/lua/scene.cpp
    ${SRC_DIR}/lua/transform.cpp
    ${SRC_DIR}/lua/light.cpp
    ${SRC_DIR}/lua/viewport.cpp
    ${SRC_DIR}/lua/camera.cpp
    ${SRC_DIR}/lua/window.cpp
    ${SRC_DIR}/lua/model.cpp
    ${SRC_DIR}/lua/utils.cpp
    ${SRC_DIR}/lua/skeleton.cpp
    ${SRC_DIR}/lua/freelook.cpp    
  )
  list(APPEND HEADERS
    ${INCLUDE_DIR}/lua.hpp
  )
  source_group(TREE ${SRC_DIR}/lua PREFIX "Lua Binding Source Files" FILES ${LUA_SOURCES})
endif()

set(CORE_LIB_NAME "darmok-core")
add_library(
  ${CORE_LIB_NAME}
  STATIC
  ${CORE_SOURCES}
)
target_sources(${CORE_LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${CORE_HEADERS})
source_group(TREE ${SRC_DIR} PREFIX "Source Files" FILES ${CORE_SOURCES})
source_group(TREE ${INCLUDE_BASE_DIR} PREFIX "Header Files" FILES ${CORE_HEADERS})
add_library(darmok::darmok-core ALIAS ${CORE_LIB_NAME})

set(LIB_NAME "darmok")
add_library(
  ${LIB_NAME}
  ${SOURCES}
  ${LUA_SOURCES}
)
target_sources(${LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${HEADERS})
source_group(TREE ${SRC_DIR} PREFIX "Source Files" FILES ${SOURCES})
source_group(TREE ${INCLUDE_BASE_DIR} PREFIX "Header Files" FILES ${HEADERS})
add_dependencies(${LIB_NAME} ${ASSET_TARGETS})
add_library(darmok::darmok ALIAS ${LIB_NAME})
target_link_libraries(${LIB_NAME} PUBLIC ${CORE_LIB_NAME})

set(EXPORT_TARGETS "")
macro(_darmok_lib_setup TARGET)
  list(APPEND EXPORT_TARGETS ${TARGET})
  set_target_properties(${TARGET} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
  if(MSVC)
    # disable https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251
    # because public methods return glm and glm is only statically linked
    target_compile_options(${TARGET} PRIVATE /wd4251)
  endif()
  target_include_directories(
    ${TARGET}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/public>
    PRIVATE
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/private>
  )
  set_target_properties(${TARGET} PROPERTIES VERSION ${PROJECT_VERSION})
  set_target_properties(${TARGET} PROPERTIES FOLDER ${PROJECT_NAME})
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET} PUBLIC darmok_EXPORTS)
  endif()
endmacro()

_darmok_lib_setup(${LIB_NAME})
_darmok_lib_setup(${CORE_LIB_NAME})

# tools
set(TOOLS_FOLDER ${PROJECT_NAME}/tools)
include(darmokCopyDynamicLibs)

set(COREASSETC_NAME "darmok-coreassetc")
set(COREASSETC_SRC_DIR "tools/coreassetc")
add_executable(${COREASSETC_NAME}
  ${COREASSETC_SRC_DIR}/coreassetc.cpp
)
list(APPEND EXPORT_TARGETS ${COREASSETC_NAME})
target_link_libraries(${COREASSETC_NAME} ${CORE_LIB_NAME})
add_executable(darmok::darmok-coreassetc ALIAS ${COREASSETC_NAME})
set_target_properties(${COREASSETC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})
darmok_copy_dynamic_libs(${COREASSETC_NAME})

set(ASSETC_NAME "darmok-assetc")
set(ASSETC_SRC_DIR "tools/assetc")
add_executable(${ASSETC_NAME}
  ${ASSETC_SRC_DIR}/assetc.cpp
)
list(APPEND EXPORT_TARGETS ${ASSETC_NAME})
target_link_libraries(${ASSETC_NAME} ${LIB_NAME})
add_executable(darmok::darmok-modelc ALIAS ${ASSETC_NAME})
set_target_properties(${ASSETC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})
darmok_copy_dynamic_libs(${ASSETC_NAME})

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
  set_target_properties(darmok-tests PROPERTIES FOLDER ${PROJECT_NAME})
endif()

# code analysis & clang tidy
if(DARMOK_RUN_CODE_ANALYSIS)
  set(CLANG_TIDY_CHECKS "-checks=-*,readability-*,modernize-*,-modernize-use-trailing-return-type")
  if(MSVC)
    set_target_properties(${LIB_NAME} PROPERTIES
      VS_GLOBAL_RunCodeAnalysis true
      VS_GLOBAL_EnableMicrosoftCodeAnalysis true
      VS_GLOBAL_CodeAnalysisRuleSet ${PROJECT_SOURCE_DIR}/darmok.ruleset
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

# freetype
if(DARMOK_BUILD_FREETYPE)
  find_package(Freetype REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE Freetype::Freetype)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_FREETYPE)
endif()

# imgui
if(DARMOK_BUILD_IMGUI)
  find_package(imgui CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC imgui::imgui)
endif()

# rmlgui
if(DARMOK_BUILD_RMLUI)
  find_package(RmlUi CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC RmlCore)
  target_link_libraries(${LIB_NAME} PRIVATE debug RmlDebugger)
  if(DARMOK_BUILD_LUA)
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
endif()

# jolt physics
if(DARMOK_BUILD_JOLT)
  find_package(unofficial-joltphysics CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE unofficial::joltphysics::Jolt)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_JOLT)
endif()

# lua sol2
if(DARMOK_BUILD_LUA)
  find_package(Lua REQUIRED)
  target_include_directories(${LIB_NAME} PRIVATE ${LUA_INCLUDE_DIR})
  target_link_libraries(${LIB_NAME} PRIVATE ${LUA_LIBRARIES})

  # luasocket
  find_library(LUASOCKET_MIME_LIB mime.core)
  find_library(LUASOCKET_SOCKET_LIB socket.core)
  target_link_libraries(${LIB_NAME} PRIVATE ${LUASOCKET_MIME_LIB} ${LUASOCKET_SOCKET_LIB})

  set(VCPKG_LUA_PATH
    "${VCPKG_BINARY_TARGET_DIR}/share/lua/?.lua"
  )
  if(MSVC)
    set(LUA_CPATH_EXT ".dll")
  else()
    set(LUA_CPATH_EXT ".so")
  endif()
  set(VCPKG_LUA_CPATH
    "${VCPKG_BINARY_TARGET_DIR}/$<$<CONFIG:Debug>:debug/>bin/?${LUA_CPATH_EXT}"
  )

  string(REPLACE ";" "," LUA_PATH_DEFINE "${VCPKG_LUA_PATH}")
  string(REPLACE ";" "," LUA_CPATH_DEFINE "${VCPKG_LUA_CPATH}")
  target_compile_definitions(${LIB_NAME} PRIVATE
    $<$<CONFIG:Debug>:LUA_PATH=${LUA_PATH_DEFINE}>
    $<$<CONFIG:Debug>:LUA_CPATH=${LUA_CPATH_DEFINE}>
  )

  # TODO: check how to use luajit
  #find_package(LuaJit REQUIRED)
  #target_include_directories(main PRIVATE ${LUAJIT_INCLUDE_DIR})
  #target_link_libraries(main PRIVATE ${LUAJIT_LIBRARIES})

  find_package(sol2 CONFIG REQUIRED)
  set(SOL_DEFINES SOL_ALL_SAFETIES_ON SOL_EXCEPTIONS_SAFE_PROPAGATION)
  target_compile_definitions(sol2 INTERFACE ${SOL_DEFINES})
  target_link_libraries(${LIB_NAME} PRIVATE sol2)
  list(APPEND VCPKG_MANIFEST_FEATURES "lua")
endif()

if(DARMOK_BUILD_LUA_EXE)
  set(LUA_EXE_NAME "darmok-lua")
  add_executable(
    ${LUA_EXE_NAME}
    src/lua/main.cpp
  )
  list(APPEND EXPORT_TARGETS ${LUA_EXE_NAME})
  darmok_copy_dynamic_libs(${LUA_EXE_NAME})
  add_executable(darmok::darmok-lua ALIAS ${LUA_EXE_NAME})
  target_compile_definitions(
    ${LUA_EXE_NAME}
    PRIVATE DARMOK_IMPLEMENT_MAIN=1
  )
  set_target_properties(${LUA_EXE_NAME} PROPERTIES FOLDER ${PROJECT_NAME})
  target_link_libraries(${LUA_EXE_NAME} ${LIB_NAME})
  install(TARGETS ${LUA_EXE_NAME}
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
  set(ozzAnimation_DIR ${LOCAL_MODULE_PATH})
  find_package(ozzAnimation CONFIG REQUIRED)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_OZZ)
  target_link_libraries(${LIB_NAME} PRIVATE
    ozz_geometry
    ozz_animation
    ozz_animation_offline
  )
endif()

# export
include(GenerateExportHeader)
set(EXPORT_HEADER_PATH include/public/darmok/export.h)
generate_export_header(${LIB_NAME}
  EXPORT_FILE_NAME ${EXPORT_HEADER_PATH}
)
list(APPEND HEADERS ${PROJECT_BINARY_DIR}/${EXPORT_HEADER_PATH})

include(GNUInstallDirs)

set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/darmok/cmake")
set(GENERATED_CONFIG_PATH "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}Config.cmake")
include(CMakePackageConfigHelpers)
configure_package_config_file(
    "${LOCAL_MODULE_PATH}/darmokConfig.cmake.in"
    ${GENERATED_CONFIG_PATH}
    INSTALL_DESTINATION ${CONFIG_INSTALL_DIR}
)
file(GLOB CMAKE_LIB_FILES ${LOCAL_MODULE_PATH}/lib/*.cmake)
install(FILES
    ${GENERATED_CONFIG_PATH}
    ${CMAKE_LIB_FILES}
    DESTINATION ${CONFIG_INSTALL_DIR}
)
set(EXPORT_NAME "${PROJECT_NAME}Targets")
install(TARGETS ${EXPORT_TARGETS}
    EXPORT ${EXPORT_NAME}
    RUNTIME
      COMPONENT Runtime
      DESTINATION ${CMAKE_INSTALL_BINDIR}
    ARCHIVE
      COMPONENT Runtime
      NAMELINK_COMPONENT Development
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY
      COMPONENT Development
      DESTINATION ${CMAKE_INSTALL_LIBDIR}
    FILE_SET HEADERS
      COMPONENT Development
)
install(EXPORT ${EXPORT_NAME}
    NAMESPACE darmok::
    DESTINATION ${CONFIG_INSTALL_DIR}
)
install(FILES
  ${PROJECT_SOURCE_DIR}/README.md
  ${PROJECT_SOURCE_DIR}/CHANGES.md
  ${PROJECT_SOURCE_DIR}/LICENSE
  DESTINATION "${CMAKE_INSTALL_DATADIR}"
)