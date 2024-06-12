cmake_minimum_required(VERSION 3.26)

if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE
    "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
    CACHE STRING "")
endif()

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
option(DARMOK_BUILD_IMGUI "build darmok with imGUI support" OFF)
option(DARMOK_BUILD_RMLUI "build darmok with RmlUI support" OFF)
option(DARMOK_BUILD_ASSIMP "build darmok with assimp support" OFF)
option(DARMOK_BUILD_OZZ "build darmok with ozz animation support" OFF)
option(DARMOK_BUILD_JOLT "build darmok with jolt physics support" OFF)
option(BUILD_SHARED_LIBS "Build using shared libraries" OFF)

set(SRC_DIR src)
set(INCLUDE_BASE_DIR include)
set(INCLUDE_DIR ${INCLUDE_BASE_DIR}/darmok)
set(LOCAL_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMake)

list(APPEND CMAKE_MODULE_PATH 
  ${LOCAL_MODULE_PATH} ${LOCAL_MODULE_PATH}/lib)

find_package(bgfx CONFIG REQUIRED)

include(darmokProcessAssets)

darmok_process_assets(
  ASSETS "assets/shaders"
  HEADER_OUTPUT_DIR "include/private/generated/shaders"
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
  ${SRC_DIR}/mesh.cpp
  ${SRC_DIR}/vertex.cpp
  ${SRC_DIR}/vertex_layout.cpp
  ${SRC_DIR}/viewport.cpp
  ${SRC_DIR}/program.cpp
)

set(CORE_HEADERS
  ${INCLUDE_DIR}/glm.hpp
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
  ${INCLUDE_DIR}/vertex.hpp
  ${INCLUDE_DIR}/vertex_layout.hpp
  ${INCLUDE_DIR}/ext_loader.hpp
  ${INCLUDE_DIR}/collection.hpp
  ${INCLUDE_DIR}/viewport.hpp
  ${INCLUDE_DIR}/program_fwd.hpp
  ${INCLUDE_DIR}/program.hpp
)

set(SOURCES
  ${SRC_DIR}/platform.cpp
  ${SRC_DIR}/program_standard.cpp
  ${SRC_DIR}/asset.cpp

  ${SRC_DIR}/app.cpp
  ${SRC_DIR}/window.cpp
  ${SRC_DIR}/input.cpp
  ${SRC_DIR}/scene.cpp

  ${SRC_DIR}/texture_atlas.cpp
  ${SRC_DIR}/material.cpp
  ${SRC_DIR}/model.cpp
  ${SRC_DIR}/anim.cpp
  ${SRC_DIR}/skeleton.cpp

  ${SRC_DIR}/transform.cpp
  ${SRC_DIR}/camera.cpp
  ${SRC_DIR}/light.cpp
  ${SRC_DIR}/render.cpp
  ${SRC_DIR}/render_forward.cpp
  ${SRC_DIR}/render_deferred.cpp
)

set(HEADERS
  ${INCLUDE_DIR}/program_standard.hpp
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
  ${INCLUDE_DIR}/skeleton.hpp
  ${INCLUDE_DIR}/model.hpp

  ${INCLUDE_DIR}/entity_filter.hpp
  ${INCLUDE_DIR}/transform.hpp
  ${INCLUDE_DIR}/camera.hpp
  ${INCLUDE_DIR}/light.hpp
  ${INCLUDE_DIR}/render.hpp
  ${INCLUDE_DIR}/render_forward.hpp
  ${INCLUDE_DIR}/render_deferred.hpp
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
  list(APPEND SOURCES ${SRC_DIR}/glfw.cpp)
endif()

if(DARMOK_BUILD_IMGUI)
  list(APPEND SOURCES ${SRC_DIR}/imgui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/imgui.hpp)

  darmok_process_assets(
    ASSETS "assets/imgui"
    HEADER_VAR_PREFIX "imgui_"
    HEADER_OUTPUT_DIR "include/private/generated/imgui/shaders"
    HEADER_SHADER_INCLUDE_DIR "generated/imgui/shaders"
    SOURCE_GROUP_NAME "Imgui Asset Files"
    SOURCES_VAR IMGUI_ASSETS
  )
  list(APPEND ASSETS ${IMGUI_ASSETS})
endif()

if(DARMOK_BUILD_RMLUI)
  list(APPEND SOURCES ${SRC_DIR}/rmlui.cpp)
  list(APPEND HEADERS ${INCLUDE_DIR}/rmlui.hpp)

  if(DARMOK_BUILD_LUA)
    list(APPEND LUA_SOURCES ${SRC_DIR}/lua/rmlui.cpp)
  endif()

  darmok_process_assets(
    ASSETS "assets/rmlui"
    HEADER_VAR_PREFIX "rmlui_"
    HEADER_OUTPUT_DIR "include/private/generated/rmlui/shaders"
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
  )
  list(APPEND HEADERS
    ${INCLUDE_DIR}/lua.hpp
  )
  source_group("Lua Binding Source Files" FILES ${LUA_SOURCES})
endif()

set(CORE_LIB_NAME "darmok-core")
add_library(
  ${CORE_LIB_NAME}
  STATIC
  ${CORE_SOURCES}
)
target_sources(${CORE_LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${CORE_HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} PREFIX "Source Files" FILES ${CORE_SOURCES})
add_library(darmok::darmok-core ALIAS ${CORE_LIB_NAME})

set(LIB_NAME "darmok")
add_library(
  ${LIB_NAME}
  ${SOURCES}
  ${LUA_SOURCES}
  ${ASSETS}
)
target_sources(${LIB_NAME} PUBLIC FILE_SET HEADERS BASE_DIRS ${INCLUDE_BASE_DIR} FILES ${HEADERS})
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR}/${SRC_DIR} PREFIX "Source Files" FILES ${SOURCES})
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
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
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

# vlayoutc: only depends on darmok-core to avoid dependency cycle
# since it is used to generate standard darmok shaders
set(VLAYOUTC_NAME "darmok-vlayoutc")
set(VLAYOUTC_SRC_DIR "tools/vlayoutc")
add_executable(${VLAYOUTC_NAME}
  ${VLAYOUTC_SRC_DIR}/vlayoutc.cpp
)
list(APPEND EXPORT_TARGETS ${VLAYOUTC_NAME})
target_link_libraries(${VLAYOUTC_NAME} ${CORE_LIB_NAME})
add_executable(darmok::darmok-vlayoutc ALIAS ${VLAYOUTC_NAME})
set_target_properties(${VLAYOUTC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})
darmok_copy_dynamic_libs(${VLAYOUTC_NAME})

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

  set(MODELC_NAME "darmok-modelc")
  set(MODELC_SRC_DIR "tools/modelc")
  add_executable(${MODELC_NAME}
    ${MODELC_SRC_DIR}/modelc.cpp
  )
  list(APPEND EXPORT_TARGETS ${MODELC_NAME})
  target_link_libraries(${MODELC_NAME} ${LIB_NAME})
  add_executable(darmok::darmok-modelc ALIAS ${MODELC_NAME})
  set_target_properties(${MODELC_NAME} PROPERTIES FOLDER ${TOOLS_FOLDER})
  darmok_copy_dynamic_libs(${MODELC_NAME})
endif()

if(DARMOK_BUILD_JOLT)
  find_package(unofficial-joltphysics CONFIG REQUIRED)
  target_link_libraries(${LIB_NAME} PRIVATE unofficial::joltphysics::Jolt)
endif()

# lua sol2
if(DARMOK_BUILD_LUA)
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
    PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
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
    ozz_options)
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
  ${CMAKE_CURRENT_SOURCE_DIR}/README.md
  ${CMAKE_CURRENT_SOURCE_DIR}/CHANGES.md
  ${CMAKE_CURRENT_SOURCE_DIR}/LICENSE
  DESTINATION "${CMAKE_INSTALL_DATADIR}"
)