if(NOT ${DARMOK_BUILD_IMGUI})
  return()
endif()

set(EDITOR_NAME darmok-editor)
set(EDITOR_BASE_DIR ${PROJECT_SOURCE_DIR}/editor)
set(EDITOR_SRC_DIR ${EDITOR_BASE_DIR}/src)
set(EDITOR_ASSETS_DIR ${EDITOR_BASE_DIR}/assets)
set(EDITOR_INCLUDE_BASE_DIR ${EDITOR_BASE_DIR}/include)
set(EDITOR_LIB_BASE_DIR ${EDITOR_BASE_DIR}/lib)
set(EDITOR_INCLUDE_DIR ${EDITOR_INCLUDE_BASE_DIR}/darmok-editor)

set(EDITOR_HEADERS
  app.hpp
  project.hpp
  utils.hpp
  scene_view.hpp
  editor.hpp
  inspector.hpp
  assets_view.hpp
  inspector/transform.hpp
  inspector/camera.hpp
  inspector/scene.hpp
  inspector/light.hpp
  inspector/render.hpp
  inspector/material.hpp
  inspector/program.hpp
  inspector/shape.hpp
  inspector/mesh.hpp
  inspector/texture.hpp
  inspector/model.hpp
)

set(EDITOR_SOURCES
  main.cpp
  app.cpp
  utils.cpp
  project.cpp
  scene_view.cpp
  editor.cpp
  inspector.cpp
  inspector/transform.cpp
  inspector/camera.cpp
  inspector/scene.cpp
  inspector/light.cpp
  inspector/render.cpp
  inspector/material.cpp
  inspector/program.cpp
  inspector/shape.cpp
  inspector/mesh.cpp
  inspector/texture.cpp
  inspector/model.cpp
)

set(EDITOR_ASSETS
  noto.ttf
  MaterialIcons-Regular.ttf
  cubemap_0.png
  cubemap_1.png
  cubemap_2.png
  cubemap_3.png
  cubemap_4.png
  cubemap_5.png
)

list(TRANSFORM EDITOR_SOURCES PREPEND "${EDITOR_SRC_DIR}/")
list(TRANSFORM EDITOR_HEADERS PREPEND "${EDITOR_INCLUDE_DIR}/")

set(BGFX_SHADERC_TARGET bgfx::shaderc)
set(EDITOR_ARGS
  --bgfx-shaderc $<TARGET_FILE:${BGFX_SHADERC_TARGET}>
  --bgfx-shader-include ${BGFX_SHADER_INCLUDE_PATH}
)

add_executable(${EDITOR_NAME} WIN32 ${EDITOR_SOURCES})
set_target_properties(${EDITOR_NAME}
  PROPERTIES FOLDER ${TOOLS_FOLDER}
  VS_DEBUGGER_COMMAND_ARGUMENTS "${EDITOR_ARGS}"
)
target_link_libraries(${EDITOR_NAME} PRIVATE darmok)

find_package(imguizmo CONFIG REQUIRED)
target_link_libraries(${EDITOR_NAME} PRIVATE imguizmo::imguizmo)

target_compile_definitions(${EDITOR_NAME} PRIVATE "DARMOK_IMPLEMENT_MAIN=1")

target_sources(${EDITOR_NAME} PUBLIC FILE_SET HEADERS
  BASE_DIRS ${EDITOR_INCLUDE_BASE_DIR} FILES ${EDITOR_HEADERS})
source_group(TREE ${EDITOR_SRC_DIR} PREFIX "Source Files" FILES ${EDITOR_SOURCES})
source_group(TREE ${EDITOR_INCLUDE_BASE_DIR} PREFIX "Header Files" FILES ${EDITOR_HEADERS})

target_include_directories(
  ${EDITOR_NAME}
  PRIVATE
  $<BUILD_INTERFACE:${EDITOR_INCLUDE_BASE_DIR}>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>
)

set(EDITOR_ASSETS_TARGET ${EDITOR_NAME}-assets)
darmok_process_assets(
  TARGET ${EDITOR_ASSETS_TARGET}
  INPUT ${EDITOR_ASSETS_DIR}
  ASSETS ${EDITOR_ASSETS}
)
add_dependencies(${EDITOR_NAME} ${EDITOR_ASSETS_TARGET})
set_target_properties(${EDITOR_ASSETS_TARGET} PROPERTIES FOLDER "darmok/assets")
