cmake_minimum_required(VERSION 3.15)

project(
  darmok
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "hobby C++ game engine"
)

option(DARMOK_BUILD_SCRIPTING "build darmok with scripting support" OFF)
option(DARMOK_BUILD_SCRIPTING_EXE "build the darmok scripting executable" OFF)
option(DARMOK_BUILD_NUKLEAR "build darmok with nuklear UI support" OFF)
option(DARMOK_BUILD_IMGUI "build darmok with imGUI support" OFF)
option(DARMOK_BUILD_CEGUI "build darmok with cegui support" OFF)
option(DARMOK_BUILD_ASSIMP "build darmok with assimp support" OFF)
option(DARMOK_BUILD_OZZ "build darmok with ozz animation support" OFF)

set(SRC_DIR src)
set(INCLUDE_DIR include/darmok)
set(LIBS_DIR lib)

set(BGFX_BUILD_EXAMPLES OFF CACHE BOOL "build bgfx examples.")
add_subdirectory(${LIBS_DIR}/bgfx.cmake)
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
  ${INCLUDE_DIR}/asset.hpp
  ${INCLUDE_DIR}/scene_fwd.hpp
  ${INCLUDE_DIR}/scene.hpp
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
  ${INCLUDE_DIR}/program.hpp
  ${INCLUDE_DIR}/transform.hpp
  ${INCLUDE_DIR}/camera.hpp
  ${INCLUDE_DIR}/light.hpp
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/render_forward.hpp
  ${INCLUDE_DIR}/render_deferred.hpp
)

set(SCRIPTING_SOURCES)

if(DARMOK_BUILD_ASSIMP)
  list(APPEND SOURCES ${SRC_DIR}/assimp.cpp)
  list(APPEND SCRIPTING_SOURCES ${SRC_DIR}/scripting/assimp.cpp)
  list(APPEND HEADERS
    ${INCLUDE_DIR}/assimp_fwd.hpp
    ${INCLUDE_DIR}/assimp.hpp)
endif()

if(DARMOK_BUILD_IMGUI)
  list(APPEND SOURCES ${SRC_DIR}/imgui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/imgui.hpp)
endif()

if(DARMOK_BUILD_NUKLEAR)
  list(APPEND SOURCES ${SRC_DIR}/nuklear.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/nuklear.hpp)
endif()

if(DARMOK_BUILD_CEGUI)
  set(CEGUI_SOURCES 
    ${SRC_DIR}/cegui/geometry.cpp
    ${SRC_DIR}/cegui/render_target.cpp
    ${SRC_DIR}/cegui/renderer.cpp
    ${SRC_DIR}/cegui/resource.cpp
    ${SRC_DIR}/cegui/shader.cpp
    ${SRC_DIR}/cegui/texture_target.cpp
    ${SRC_DIR}/cegui/texture.cpp
    ${SRC_DIR}/cegui/utils.cpp
    ${SRC_DIR}/cegui/target_transform.cpp
    ${SRC_DIR}/cegui/image.cpp
  )
  list(APPEND SOURCES ${CEGUI_SOURCES} ${SRC_DIR}/cegui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/cegui.hpp)
  source_group("CEGUI Source Files" FILES ${CEGUI_SOURCES})

  darmok_process_assets(
    ASSETS_INCLUDES "shaders/cegui_*"
    HEADER_SHADER_OUTPUT_DIR "include/generated/shaders"
    HEADER_SHADER_INCLUDE_DIR "generated/shaders"
  )

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
    ${SRC_DIR}/scripting/shape.cpp
    ${SRC_DIR}/scripting/scene.cpp
    ${SRC_DIR}/scripting/transform.cpp
    ${SRC_DIR}/scripting/light.cpp
    ${SRC_DIR}/scripting/camera.cpp
    ${SRC_DIR}/scripting/window.cpp
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
    ${LIBS_DIR}/rapidjson/include
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_BINARY_DIR}/include
  )

  set_target_properties(${LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

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
    set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "Build the GLFW example programs")
    set(GLFW_BUILD_TESTS OFF CACHE BOOL "Build the GLFW test programs")
    set(GLFW_BUILD_DOCS OFF CACHE BOOL "Build the GLFW documentation")
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
set(GLM_DEFINES GLM_ENABLE_EXPERIMENTAL GLM_FORCE_LEFT_HANDED)
target_compile_definitions(glm INTERFACE ${GLM_DEFINES})
target_link_libraries(${LIB_NAME} PUBLIC glm)

# bgfx
target_link_libraries(${LIB_NAME} PUBLIC bgfx bimg bimg_decode bx)

# imgui
if(DARMOK_BUILD_IMGUI)
  include(${CMAKE_CURRENT_LIST_DIR}/CMake/imgui.cmake)
  target_link_libraries(${LIB_NAME} PUBLIC imgui)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_IMGUI)
endif()

# entt
add_subdirectory(${LIBS_DIR}/entt)
target_link_libraries(${LIB_NAME} PUBLIC EnTT)

# assimp
if(DARMOK_BUILD_ASSIMP)
  set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "If the test suite for Assimp is built in addition to the library.")
  set(ASSIMP_BUILD_DRACO ON CACHE BOOL "If the Draco libraries are to be built. Primarily for glTF")
  set(ASSIMP_BUILD_ALL_EXPORTERS_BY_DEFAULT OFF CACHE BOOL "default value of all ASSIMP_BUILD_XXX_EXPORTER values")

  add_subdirectory(${LIBS_DIR}/assimp)
  target_link_libraries(${LIB_NAME} PUBLIC assimp)
  target_compile_definitions(assimp PRIVATE ASSIMP_BUILD_NO_GLTF1_IMPORTER ASSIMP_BUILD_NO_EXPORT)
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_ASSIMP)
endif()

# pugixml
add_subdirectory(${LIBS_DIR}/pugixml)
target_link_libraries(${LIB_NAME} PRIVATE pugixml)
target_compile_definitions(pugixml INTERFACE PUGIXML_HEADER_ONLY) # also included in assimp

# lua sol2
if(DARMOK_BUILD_SCRIPTING)
  set(BUILD_LUAJIT ON)
  set(BUILD_LUA_AS_DLL OFF)
  list(PREPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/${LIBS_DIR}/sol2/cmake/Packages")
  set(LUA_LOCAL_DIR ${CMAKE_CURRENT_SOURCE_DIR}/${LIBS_DIR}/luajit)
  find_package(LuaBuild REQUIRED COMPONENTS "latest")
  set(LUA_LIB_NAME luajit_lib_2.1)
  add_subdirectory(${LIBS_DIR}/sol2)
  target_compile_definitions(sol2 INTERFACE SOL_EXCEPTIONS_SAFE_PROPAGATION)
  target_link_libraries(${LIB_NAME} PRIVATE sol2 ${LUA_LIB_NAME})
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
  # FBX SDK can be downloaded from https://aps.autodesk.com/developer/overview/fbx-sdk
  # set(ozz_build_fbx ON CACHE BOOL "Build Fbx pipeline (Requires Fbx SDK)")
  set(ozz_build_gltf ON CACHE BOOL "Build glTF importer (Requires c++11)")
  set(ozz_build_samples OFF CACHE BOOL "Build samples")
  set(ozz_build_howtos OFF CACHE BOOL "Build howtos")
  set(ozz_build_tests OFF CACHE BOOL "Build unit tests")
  set(ozz_build_msvc_rt_dll ON CACHE BOOL "Select msvc DLL runtime library")
  add_subdirectory(${LIBS_DIR}/ozz)
  target_link_libraries(${LIB_NAME} PRIVATE
    ozz_geometry
    ozz_animation_offline
    ozz_options)
endif()

# nuklear
if(DARMOK_BUILD_NUKLEAR)
  set(NUKLEAR_BASE ${LIBS_DIR}/nuklear)
  set(NUKLEAR_DEFINES 
    NK_INCLUDE_FIXED_TYPES
    NK_INCLUDE_STANDARD_IO
    NK_INCLUDE_STANDARD_VARARGS
    NK_INCLUDE_STANDARD_BOOL
    NK_INCLUDE_DEFAULT_ALLOCATOR
    NK_INCLUDE_VERTEX_BUFFER_OUTPUT
    NK_INCLUDE_FONT_BAKING # adds `stb_truetype` and `stb_rect_pack` implementation to this library and provides font baking and rendering. If you already have font handling or do not want to use this font handler you don't have to define it.
    NK_INCLUDE_DEFAULT_FONT # adds the default font: ProggyClean.ttf into this library which can be loaded into a font atlas and allows using this library without having a truetype font
    NK_KEYSTATE_BASED_INPUT # if your backend uses key state for each frame rather than key press/release events
    NK_INCLUDE_STANDARD_BOOL
    # NK_ZERO_COMMAND_MEMORY # will zero out memory for each drawing command added to a drawing queue (inside nk_command_buffer_push). Zeroing command memory is very useful for fast checking (using memcmp) if command buffers are equal and avoid drawing frames when nothing on screen has changed since previous frame.
    # NK_INCLUDE_COMMAND_USERDATA # adds a userdata pointer into each command. Can be useful for example if you want to provide custom shaders depending on the used widget. Can be combined with the style structures.
    # NK_BUTTON_TRIGGER_ON_RELEASE # require button clicks occurring either on buttons being pressed (up to down) or released (down to up). By default this library will react on buttons being pressed, but if you define this it will only trigger if a button is released.
  )
  if(DARMOK_BUILD_IMGUI)
    list(APPEND NUKLEAR_DEFINES
      NK_NO_STB_TRUETYPE_IMPLEMENTATION # already added in imgui
      NK_NO_STB_RECT_PACK_IMPLEMENTATION # already added in imgui
    )
  endif()
  target_include_directories(${LIB_NAME} PUBLIC ${NUKLEAR_BASE})
  target_compile_definitions(${LIB_NAME} PUBLIC ${NUKLEAR_DEFINES})
  target_compile_definitions(${LIB_NAME} PUBLIC DARMOK_NUKLEAR)
endif()

# freetype
set(FT_DISABLE_BROTLI ON CACHE BOOL "Disable support of compressed WOFF2 fonts.")
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "Disable HarfBuzz (used for improving auto-hinting of OpenType fonts).")
set(FT_ENABLE_ERROR_STRINGS ON CACHE BOOL "Enable support for meaningful error descriptions.")
add_subdirectory(${LIBS_DIR}/freetype)
target_link_libraries(${LIB_NAME} PRIVATE freetype)

# cegui
if(DARMOK_BUILD_CEGUI)
  set(GLM_H_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${LIBS_DIR}/glm)
  set(PUGIXML_H_PATH ${LIBS_DIR}/pugixml/src)
  set(FREETYPE_H_PATH ${LIBS_DIR}/freetype/include)

  set(CEGUI_BUILD_APPLICATION_TEMPLATES OFF CACHE BOOL "Specifies whether to build the application templates.")
  set(CEGUI_BUILD_RENDERER_OPENGL OFF CACHE BOOL "Specifies whether to build the old OpenGL 1.2 (fixed pipeline) renderer module.")
  set(CEGUI_BUILD_RENDERER_OPENGL3 OFF CACHE BOOL "Specifies whether to build the renderer module that supports OpenGL 3.2 (core profile) and OpenGL ES 2.0.")
  set(CEGUI_BUILD_RENDERER_DIRECT3D11 OFF CACHE BOOL "Specifies whether to build the Direct3D 11 renderer module")
  set(CEGUI_BUILD_SAMPLES OFF CACHE BOOL "Specifies whether to build the CEGUI sample applications")
  set(CEGUI_BUILD_IMAGECODEC_STB ON CACHE BOOL "Specifies whether to build the STB based ImageCodec module")
  set(CEGUI_BUILD_XMLPARSER_PUGIXML ON CACHE BOOL "Specifies the XMLParser module to use as the default")
  set(CEGUI_BUILD_STATIC_CONFIGURATION ON CACHE BOOL "Specifies whether the static configs will be built. (NB: Shared configs are always built)")
  set(CEGUI_BUILD_DYNAMIC_CONFIGURATION OFF CACHE BOOL "Dynamic libs to be built. Always true except for Android")
  set(CEGUI_BUILD_STATIC_FACTORY_MODULE ON CACHE BOOL "For static CEGUI builds, specifies whether to include the getWindowFactoryModule method.  When false, the developer must provide a custom implementation of the method")
  set(CEGUI_USE_FREETYPE ON CACHE BOOL "Specifies whether to include Freetype font support")

  add_subdirectory(${LIBS_DIR}/cegui)

  set(CEGUI_INCLUDE_DIRS
    ${LIBS_DIR}/cegui/cegui/include
    ${CMAKE_CURRENT_BINARY_DIR}/lib/cegui/cegui/include)

  # TODO: maybe add CEGUIPugiXMLParser directly in darmok
  # TODO: CEGUI recommends dynamic linking
  target_link_libraries(CEGUIPugiXMLParser_Static pugixml)
  target_include_directories(CEGUIPugiXMLParser_Static PRIVATE ${PUGIXML_H_PATH})
  target_include_directories(CEGUIBase-9999_Static PRIVATE ${FREETYPE_H_PATH})
  target_include_directories(${LIB_NAME} PUBLIC ${CEGUI_INCLUDE_DIRS})

  target_link_libraries(${LIB_NAME} PRIVATE
    CEGUICoreWindowRendererSet_Static
    CEGUIPugiXMLParser_Static)

  target_link_libraries(${LIB_NAME} PUBLIC
    CEGUIBase-9999_Static CEGUICommonDialogs-9999_Static)

endif()