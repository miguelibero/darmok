cmake_minimum_required(VERSION 3.26)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

set(VCPKG_BINARY_TARGET_DIR vcpkg_installed/${VCPKG_TARGET_TRIPLET})

project(
  darmok
  VERSION 0.1.2
  LANGUAGES CXX
  DESCRIPTION "hobby C++ game engine"
)
set(DARMOK_VERSION_MAJOR ${PROJECT_VERSION_MAJOR})
set(DARMOK_VERSION_MINOR ${PROJECT_VERSION_MINOR})
set(DARMOK_VERSION_PATCH ${PROJECT_VERSION_PATCH})

if(PROJECT_BINARY_DIR STREQUAL PROJECT_SOURCE_DIR)
  message(
    FATAL_ERROR
    "In-source builds not allowed. Please make a new directory (called a build directory) and run CMake from there."
  )
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(ADD_OPTIM_FLAGS "")
if(MSVC)
  set(ADD_OPTIM_FLAGS "/GS- /Gy /O2 /Oi /Ot")
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /bigobj")
  add_compile_options("/utf-8")
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

if(APPLE)
  macro(target_link_framework TARGET NAME)
    find_library(FRAMEWORK_${NAME}
      NAMES ${NAME}
      PATHS ${CMAKE_OSX_SYSROOT}/System/Library
      PATH_SUFFIXES Frameworks PrivateFrameworks
      CMAKE_FIND_FRAMEWORK only
      NO_DEFAULT_PATH )
    if(${FRAMEWORK_${NAME}} STREQUAL FRAMEWORK_${NAME}-NOTFOUND)
      message(ERROR ": Framework ${NAME} not found")
    else()
      target_link_libraries(${TARGET} PUBLIC "${FRAMEWORK_${NAME}}")
      message(STATUS "Framework ${NAME} found at ${FRAMEWORK_${NAME}}")
    endif()
  endmacro(target_link_framework)
endif()

option(DARMOK_BUILD_SAMPLES "build darmok samples" OFF)
option(DARMOK_BUILD_TESTS "build darmok unit tests" OFF)
option(DARMOK_BUILD_LUA "build darmok with lua support" OFF)
option(DARMOK_BUILD_LUAJIT "build darmok with luajit support" OFF)
option(DARMOK_BUILD_LUA_EXE "build the darmok lua executable" OFF)
option(DARMOK_BUILD_FREETYPE "build darmok with freetype support" OFF)
option(DARMOK_BUILD_IMGUI "build darmok with imGUI support" OFF)
option(DARMOK_BUILD_RMLUI "build darmok with RmlUI support" OFF)
option(DARMOK_BUILD_ASSIMP "build darmok with assimp support" OFF)
option(DARMOK_BUILD_OZZ "build darmok with ozz animation support" OFF)
option(DARMOK_BUILD_JOLT "build darmok with jolt physics support" OFF)
option(DARMOK_BUILD_MINIAUDIO "build darmok with miniaudio support" OFF)
option(DARMOK_BUILD_EDITOR "build the darmok editor" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(INCLUDE_BASE_DIR ${PROJECT_SOURCE_DIR}/include)
set(LIB_BASE_DIR ${PROJECT_SOURCE_DIR}/lib)
set(INCLUDE_DIR ${INCLUDE_BASE_DIR}/darmok)
set(LOCAL_MODULE_PATH ${PROJECT_SOURCE_DIR}/CMake)

list(APPEND CMAKE_MODULE_PATH 
  ${LOCAL_MODULE_PATH} ${LOCAL_MODULE_PATH}/lib)

find_package(bgfx CONFIG REQUIRED)
find_package(lodepng CONFIG REQUIRED)

include(darmokProcessAssets)

set(CORE_ASSETS_TARGET darmok-core-assets)
darmok_process_assets(
  TARGET ${CORE_ASSETS_TARGET} CORE
  INPUT ${PROJECT_SOURCE_DIR}/assets/shaders/core
  OUTPUT include/private/generated/shaders
  ASSETS
    darmok_light.sc
    darmok_material.sc
    darmok_material_basic.sc
    darmok_sampler.sc
    darmok_skinning.sc
    darmok_util.sc
    darmok-import.json
    forward.fragment.sc
    forward.vertex.sc
    forward.program.json
    forward_basic.fragment.sc
    forward_basic.vertex.sc
    forward_basic.program.json
    tonemap.fragment.sc
    tonemap.vertex.sc
    tonemap.program.json
    gui.fragment.sc
    gui.vertex.sc
    gui.program.json
    unlit.fragment.sc
    unlit.vertex.sc
    unlit.program.json
)
set(ASSET_TARGETS ${CORE_ASSETS_TARGET})

set(CORE_SOURCES
  string.cpp
  stream.cpp
  data.cpp
  data_stream.cpp
  utils.cpp
  uniform.cpp
  math.cpp
  color.cpp
  image.cpp
  shape.cpp
  texture.cpp
  mesh_core.cpp
  vertex.cpp
  varying.cpp
  viewport.cpp
  program_core.cpp
  asset_core.cpp
  easing.cpp
  protobuf.cpp
  glm_serialize.cpp
)

set(CORE_HEADERS
  convert.hpp
  glm.hpp
  glm_serialize.hpp
  utils.hpp
  string.hpp
  stream.hpp
  optional_ref.hpp
  collection.hpp
  data.hpp
  data_stream.hpp
  uniform.hpp
  math.hpp
  shape.hpp
  mesh_core.hpp
  color_fwd.hpp
  color.hpp
  image_fwd.hpp
  image.hpp
  texture.hpp
  vertex.hpp
  varying.hpp
  collection.hpp
  viewport.hpp
  program_core.hpp
  asset_core.hpp
  loader.hpp
  easing.hpp
  protobuf.hpp
  expected.hpp
)

set(SOURCES
  platform.cpp
  asset.cpp

  app.cpp
  window.cpp
  input.cpp
  scene.cpp
  scene_filter.cpp
  scene_serialize.cpp
  asset_pack.cpp
  
  texture_atlas.cpp
  material.cpp
  anim.cpp
  program.cpp
  mesh.cpp

  transform.cpp
  camera.cpp
  culling.cpp
  light.cpp
  render_scene.cpp
  render_forward.cpp
  render_deferred.cpp
  render_chain.cpp
  render_debug.cpp
  shadow.cpp
  freelook.cpp
  text.cpp
  environment.cpp
)

set(HEADERS
  asset.hpp
  app_fwd.hpp
  app.hpp
  input.hpp
  window.hpp
  scene_fwd.hpp
  scene.hpp
  scene_filter.hpp
  scene_serialize.hpp
  asset_pack.hpp

  texture_atlas.hpp
  anim.hpp
  material.hpp
  program.hpp
  mesh.hpp

  transform.hpp
  camera.hpp
  culling.hpp
  light.hpp
  render_scene.hpp
  render_forward.hpp
  render_deferred.hpp
  render_chain.hpp
  render_debug.hpp
  shadow_fwd.hpp
  shadow.hpp
  freelook.hpp
  text_fwd.hpp
  text.hpp
  environment.hpp
)

set(LUA_SOURCES
  app.cpp
  asset.cpp
  asset_pack.cpp
  image.cpp
  material.cpp
  render_scene.cpp
  render_forward.cpp
  render_chain.cpp
  culling.cpp
  mesh.cpp
  program.cpp
  texture.cpp
  input1.cpp
  input2.cpp
  math_base.cpp
  math_color.cpp
  math_mat.cpp
  math_quat.cpp
  math_uvec.cpp
  math_vec.cpp
  math_protobuf.cpp
  shape1.cpp
  shape2.cpp
  scene.cpp
  scene_serialize.cpp
  scene_filter.cpp
  transform.cpp
  light.cpp
  viewport.cpp
  camera.cpp
  window.cpp
  utils.cpp
  protobuf.cpp
  skeleton.cpp
  freelook.cpp
  coroutine.cpp 
)

if(NOT DARMOK_PLATFORM)
  set(DARMOK_PLATFORM glfw)
endif()
if(UNIX AND NOT APPLE)
  set(DARMOK_PLATFORM_SUPPORT_WAYLAND 1)
else()
  set(DARMOK_PLATFORM_SUPPORT_WAYLAND 0)
endif()

if(DARMOK_PLATFORM STREQUAL "glfw")
  list(APPEND SOURCES platform_glfw.cpp)
endif()

if(DARMOK_BUILD_FREETYPE)
  list(APPEND HEADERS
    text_freetype.hpp
  )
  list(APPEND SOURCES
    text_freetype.cpp
  )
  list(APPEND LUA_SOURCES
    text.cpp
  )
endif()

if(DARMOK_BUILD_IMGUI)
  list(APPEND SOURCES imgui.cpp)
  list(APPEND HEADERS imgui.hpp)

  set(IMGUI_ASSETS_TARGET darmok-imgui-assets)
  darmok_process_assets(
    TARGET ${IMGUI_ASSETS_TARGET} CORE
    INPUT ${PROJECT_SOURCE_DIR}/assets/shaders/imgui
    OUTPUT include/private/generated/shaders/imgui
    ASSETS
      imgui.fragment.sc
      imgui.vertex.sc
      imgui.program.json
      darmok-import.json
  )
  list(APPEND ASSET_TARGETS ${IMGUI_ASSETS_TARGET})
endif()

if(DARMOK_BUILD_RMLUI)
  list(APPEND SOURCES
    rmlui.cpp
    rmlui_debug.cpp
  )
  list(APPEND HEADERS
    rmlui.hpp
    rmlui_fwd.hpp
    rmlui_debug.hpp
  )
  list(APPEND LUA_SOURCES
    rmlui_comp.cpp
    rmlui_canvas.cpp
    rmlui_event.cpp
    rmlui_element.cpp
    rmlui_debug.cpp
  )

  set(RMLUI_ASSETS_TARGET darmok-rmlui-assets)
  darmok_process_assets(
    TARGET ${RMLUI_ASSETS_TARGET} CORE
    INPUT ${PROJECT_SOURCE_DIR}/assets/shaders/rmlui
    OUTPUT include/private/generated/shaders/rmlui
    ASSETS
      rmlui.fragment.sc
      rmlui.vertex.sc
      rmlui.program.json
      darmok-import.json
  )
  list(APPEND ASSET_TARGETS ${RMLUI_ASSETS_TARGET})
endif()

if(DARMOK_BUILD_OZZ)
  list(APPEND HEADERS
    skeleton.hpp
    skeleton_ozz.hpp
  )
  list(APPEND SOURCES
    skeleton.cpp  
    skeleton_ozz.cpp
  )

  list(APPEND LUA_SOURCES skeleton.cpp)
endif()

if(DARMOK_BUILD_ASSIMP)
  list(APPEND HEADERS
    assimp.hpp
    scene_assimp.hpp
    mesh_assimp.hpp
  )
  list(APPEND SOURCES
    assimp.cpp
    scene_assimp.cpp
    mesh_assimp.cpp
  )
  if(DARMOK_BUILD_OZZ)
    list(APPEND HEADERS skeleton_assimp.hpp)
    list(APPEND SOURCES skeleton_assimp.cpp)
  endif()  
endif()

if(DARMOK_BUILD_JOLT)
  list(APPEND HEADERS
    physics3d.hpp
    physics3d_character.hpp
    physics3d_debug.hpp
  )
  list(APPEND SOURCES
    physics3d_jolt.cpp
    physics3d_character_jolt.cpp
    physics3d_debug_jolt.cpp
  )
  list(APPEND LUA_SOURCES
    physics3d_system.cpp
    physics3d_body.cpp
    physics3d_character.cpp
    physics3d_debug.cpp
  )
endif()

if(DARMOK_BUILD_MINIAUDIO)
  list(APPEND HEADERS
    audio_fwd.hpp  
    audio.hpp
    audio_mini.hpp
  )
  list(APPEND SOURCES
    audio_mini.cpp
  )
  list(APPEND LUA_SOURCES audio.cpp)
endif()

if(DARMOK_BUILD_LUA_EXE)
  set(DARMOK_BUILD_LUA ON CACHE BOOL "")
endif()

list(TRANSFORM LUA_SOURCES PREPEND "${SRC_DIR}/lua/")
list(TRANSFORM SOURCES PREPEND "${SRC_DIR}/")

if(DARMOK_BUILD_LUA)
  list(APPEND HEADERS lua.hpp)

  set(LUA_ASSETS_TARGET darmok-lua-assets)
  darmok_process_assets(
    TARGET ${LUA_ASSETS_TARGET} CORE
    INPUT ${PROJECT_SOURCE_DIR}/assets/lua
    OUTPUT include/private/generated/lua
    ASSETS
      middleclass.lua
      class.lua
      string.lua
      table.lua
      base.lua
  )
  list(APPEND ASSET_TARGETS ${LUA_ASSETS_TARGET})
  list(APPEND SOURCES ${LUA_SOURCES})
endif()

list(TRANSFORM CORE_HEADERS PREPEND "${INCLUDE_DIR}/")
list(TRANSFORM CORE_SOURCES PREPEND "${SRC_DIR}/")
list(TRANSFORM HEADERS PREPEND "${INCLUDE_DIR}/")

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
)
target_sources(${LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${HEADERS})
source_group(TREE ${SRC_DIR} PREFIX "Source Files" FILES ${SOURCES})
source_group(TREE ${INCLUDE_BASE_DIR} PREFIX "Header Files" FILES ${HEADERS})
source_group(TREE ${SRC_DIR}/lua PREFIX "Lua Binding Source Files" FILES ${LUA_SOURCES})

add_dependencies(${LIB_NAME} ${ASSET_TARGETS})
foreach(TARGET ${ASSET_TARGETS})
  set_target_properties(${TARGET} PROPERTIES FOLDER "darmok/assets")
endforeach()

add_library(darmok::darmok ALIAS ${LIB_NAME})
target_link_libraries(${LIB_NAME} PUBLIC ${CORE_LIB_NAME})

# code analysis & clang tidy
if(DARMOK_RUN_CODE_ANALYSIS)
  find_program(CLANG_TIDY_EXE NAMES "clang-tidy" NO_CACHE)
endif()

macro(_darmok_analysis_setup TARGET)
  if(DARMOK_RUN_CODE_ANALYSIS)
    set(CLANG_TIDY_CMD "${CLANG_TIDY_EXE}")
    if(MSVC)
      # clang-tidy header filter not working with vs
      # set_target_properties(${LIB_NAME} PROPERTIES
      #   VS_GLOBAL_RunCodeAnalysis false
      #   VS_GLOBAL_EnableMicrosoftCodeAnalysis true
      #   VS_GLOBAL_EnableClangTidyCodeAnalysis true
      # )
      # # https://gitlab.kitware.com/cmake/cmake/-/issues/20512
      # set(CLANG_TIDY_CMD "${CLANG_TIDY_CMD};--extra-arg=/EHsc")
    endif()
    set_target_properties(${LIB_NAME} PROPERTIES CXX_CLANG_TIDY "${CLANG_TIDY_CMD}")
  endif()
endmacro()

set(EXPORT_TARGETS "")
macro(_darmok_lib_setup TARGET)
  list(APPEND EXPORT_TARGETS ${TARGET})
  set_target_properties(${TARGET} PROPERTIES COMPILE_WARNING_AS_ERROR ON)
  if(MSVC)
    # disable https://learn.microsoft.com/en-us/cpp/error-messages/compiler-warnings/compiler-warning-level-1-c4251
    # because public methods return glm and glm is only statically linked
    target_compile_options(${TARGET} PRIVATE /wd4251)
    target_compile_options(${TARGET} PRIVATE /EHs)
  else()
    target_compile_options(${TARGET} PRIVATE -fexceptions)
  endif()
  target_include_directories(
    ${TARGET}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/public>
    PRIVATE
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/private>
  )
  set_target_properties(${TARGET} PROPERTIES VERSION ${PROJECT_VERSION})
  set_target_properties(${TARGET} PROPERTIES FOLDER ${PROJECT_NAME})
  if(BUILD_SHARED_LIBS)
    target_compile_definitions(${TARGET} PUBLIC darmok_EXPORTS)
  endif()
  _darmok_analysis_setup(${TARGET})
  # target_link_libraries(${LIB_NAME} PRIVATE -fsanitize=address)
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
add_executable(darmok::darmok-assetc ALIAS ${ASSETC_NAME})
set_target_properties(${ASSETC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})
darmok_copy_dynamic_libs(${ASSETC_NAME})

# samples
if(DARMOK_BUILD_SAMPLES)
  list(APPEND VCPKG_MANIFEST_FEATURES "samples")
  add_subdirectory(samples)
endif()

# editor
if(DARMOK_BUILD_EDITOR)
  list(APPEND VCPKG_MANIFEST_FEATURES "editor")
  add_subdirectory(editor)
endif()

# testing
if(DARMOK_BUILD_TESTS)
  enable_testing()
  add_subdirectory(test)
  list(APPEND VCPKG_MANIFEST_FEATURES "tests")
  set_target_properties(darmok-tests PROPERTIES FOLDER ${PROJECT_NAME})
  set_target_properties(darmok-tests-proto-objects PROPERTIES FOLDER ${PROJECT_NAME})
endif()

# openGL
find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIRS})

# bgfx
target_link_libraries(${CORE_LIB_NAME} PUBLIC bgfx::bx bgfx::bgfx bgfx::bimg bgfx::bimg_decode)
set(BGFX_SHADERC_TARGET bgfx::shaderc)

# fmt
find_package(fmt CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC fmt::fmt)
# target_compile_definitions(${CORE_LIB_NAME} PUBLIC FMT_UNICODE=0)
# target_compile_definitions(${LIB_NAME} PUBLIC FMT_UNICODE=0)

# pugixml
find_package(pugixml CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC pugixml::pugixml)

# json
find_package(nlohmann_json CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC nlohmann_json::nlohmann_json)

# magic_enum
find_package(magic_enum CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE magic_enum::magic_enum)

# tl-expected
find_package(tl-expected CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PUBLIC tl::expected)

# protobuf
find_package(Protobuf CONFIG REQUIRED)
set(PROTO_FILES
  math.proto
  easing.proto
  shape.proto
  varying.proto
  program.proto
  texture.proto
  texture_atlas.proto
  material.proto
  mesh.proto
  registry.proto
  scene.proto
  camera.proto
  asset.proto
  light.proto
  skeleton.proto
  assimp.proto
  text.proto
  physics3d.proto
  rmlui.proto
  input.proto
  freelook.proto
)
set(PROTO_DIR ${PROJECT_SOURCE_DIR}/assets/protobuf)
set(PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/public/darmok/protobuf")
set(PROTO_LIB_NAME ${LIB_NAME}-proto-objects)

list(TRANSFORM PROTO_FILES PREPEND "${PROTO_DIR}/")
add_library(${PROTO_LIB_NAME} OBJECT ${PROTO_FILES})
target_link_libraries(${PROTO_LIB_NAME} PUBLIC protobuf::libprotobuf)
target_include_directories(${PROTO_LIB_NAME} PUBLIC "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
protobuf_generate(
    TARGET ${PROTO_LIB_NAME}
    IMPORT_DIRS "${PROTO_DIR}"
    PROTOC_OUT_DIR "${PROTO_BINARY_DIR}")
target_link_libraries(${CORE_LIB_NAME} PRIVATE ${PROTO_LIB_NAME})
add_dependencies(${CORE_LIB_NAME} ${PROTO_LIB_NAME})
list(APPEND EXPORT_TARGETS ${PROTO_LIB_NAME})
set_target_properties(${PROTO_LIB_NAME} PROPERTIES FOLDER ${PROJECT_NAME})

# glm
find_package(glm CONFIG REQUIRED)
set(GLM_DEFINES GLM_ENABLE_EXPERIMENTAL GLM_FORCE_LEFT_HANDED GLM_FORCE_DEPTH_ZERO_TO_ONE)
target_compile_definitions(glm::glm-header-only INTERFACE ${GLM_DEFINES})
target_link_libraries(${CORE_LIB_NAME} PUBLIC glm::glm-header-only)

# cli11
find_package(CLI11 CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE CLI11::CLI11)

# tiny-process
find_package(tiny-process-library CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE tiny-process-library::tiny-process-library)

# tinyfiledialogs
find_package(tinyfiledialogs CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PRIVATE tinyfiledialogs::tinyfiledialogs)

# mikktspace
find_package(mikktspace CONFIG REQUIRED)
target_link_libraries(${CORE_LIB_NAME} PRIVATE mikktspace::mikktspace)

# glfw
if(DARMOK_PLATFORM STREQUAL "glfw")
  find_package(glfw3 CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE glfw)
  target_compile_definitions(${LIB_NAME} PRIVATE DARMOK_PLATFORM_GLFW)
  if (DARMOK_PLATFORM_SUPPORT_WAYLAND)
    target_link_libraries(${LIB_NAME} PRIVATE wayland-egl)
    target_compile_definitions(bgfx::bgfx INTERFACE WL_EGL_PLATFORM)
  endif()
endif()

# taskflow
find_package(Taskflow CONFIG REQUIRED)
target_link_libraries(${LIB_NAME} PRIVATE Taskflow::Taskflow)

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
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_IMGUI)
endif()

# rmlgui
if(DARMOK_BUILD_RMLUI)
  find_package(RmlUi CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PUBLIC RmlUi::RmlUi)
  target_link_libraries(${LIB_NAME} PRIVATE debug RmlUi::Debugger)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_RMLUI)
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
  find_package(Jolt CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE Jolt::Jolt)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_JOLT)
endif()

if(DARMOK_BUILD_MINIAUDIO)
  find_path(MINIAUDIO_INCLUDE_DIRS "miniaudio.h")
  target_include_directories(${LIB_NAME} PRIVATE ${MINIAUDIO_INCLUDE_DIRS})
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_MINIAUDIO)
  if(APPLE)
    target_link_framework(${LIB_NAME} "CoreAudio")
    target_link_framework(${LIB_NAME} "AudioToolbox")
  endif()
endif()

# lua sol2
if(DARMOK_BUILD_LUA)

  find_path(LUAJIT_INCLUDE_DIR luajit/lua.hpp)
  
  if(DARMOK_BUILD_LUAJIT AND LUAJIT_INCLUDE_DIR)
    set(LUAJIT_INCLUDE_DIR "${LUAJIT_INCLUDE_DIR}/luajit")
    find_library(LUAJIT_LIBRARY NAMES lua51 lua)
    target_include_directories(${LIB_NAME} PRIVATE ${LUAJIT_INCLUDE_DIR})
    target_link_libraries(${LIB_NAME} PRIVATE ${LUAJIT_LIBRARY})
    set(SOL2_LUA_VERSION "LuaJIT")
    target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_LUAJIT)
  else()
    # normal lua
    find_package(Lua REQUIRED)
    target_include_directories(${LIB_NAME} PRIVATE ${LUA_INCLUDE_DIR})
    target_link_libraries(${LIB_NAME} PRIVATE ${LUA_LIBRARIES})
  endif()

  # luasocket
  if(UNIX)
    find_library(LUASOCKET_MIME_LIB mime.core.a)
    find_library(LUASOCKET_SOCKET_LIB socket.core.a)  
    find_library(LUASOCKET_SOCKET_UNIX_LIB socket.unix.a)  
    target_link_libraries(${LIB_NAME} PRIVATE ${LUASOCKET_MIME_LIB} ${LUASOCKET_SOCKET_LIB} ${LUASOCKET_SOCKET_UNIX_LIB})
  elseif(WIN32)
    find_library(LUASOCKET_MIME_LIB mime.core)
    find_library(LUASOCKET_SOCKET_LIB socket.core)  
    target_link_libraries(${LIB_NAME} PRIVATE ${LUASOCKET_MIME_LIB} ${LUASOCKET_SOCKET_LIB})
  endif()

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

  find_package(sol2 CONFIG REQUIRED)
  set(SOL_DEFINES LUA_USE_APICHECK SOL_ALL_SAFETIES_ON=1)
  find_path(SOL2_INCLUDE_DIRS "sol/abort.hpp")
  target_include_directories(${LIB_NAME} PRIVATE ${SOL2_INCLUDE_DIRS})
  target_compile_definitions(${LIB_NAME} PUBLIC ${SOL_DEFINES})
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

  set(LUA_EXE_ARGS
    --bgfx-shaderc $<TARGET_FILE:${BGFX_SHADERC_TARGET}>
    --bgfx-shader-include
      ${BGFX_SHADER_INCLUDE_PATH}
      ${PROJECT_SOURCE_DIR}/assets/shaders/core
  )

  set_target_properties(${LUA_EXE_NAME}
    PROPERTIES FOLDER ${TOOLS_FOLDER}
    VS_DEBUGGER_COMMAND_ARGUMENTS "${LUA_EXE_ARGS}"
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
set(EXPORT_CUSTOM_CONTENT  "
#define DARMOK_VERSION_MAJOR ${DARMOK_VERSION_MAJOR}
#define DARMOK_VERSION_MINOR ${DARMOK_VERSION_MINOR}
#define DARMOK_VERSION_PATCH ${DARMOK_VERSION_PATCH}
#define DARMOK_VERSION \"${PROJECT_VERSION}\"
")
generate_export_header(${LIB_NAME}
  EXPORT_FILE_NAME ${EXPORT_HEADER_PATH}
  CUSTOM_CONTENT_FROM_VARIABLE EXPORT_CUSTOM_CONTENT
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