cmake_minimum_required(VERSION 3.15)

project(
  darmok
  VERSION 0.1.0
  LANGUAGES CXX
  DESCRIPTION "hobby game engine"
)

set(sources
  src/window.cpp
  src/platform.cpp
  src/app.cpp
  src/glfw.cpp
  src/input.cpp
  src/imgui.cpp
  src/bgfx.cpp
)

set(headers
  include/darmok/input.hpp
  include/darmok/window.hpp
  include/darmok/app.hpp
  include/darmok/utils.hpp
)

set(LIB_NAME darmok)

add_library(
  ${LIB_NAME}
  ${headers}
  ${sources}
)

set_target_properties(
  ${LIB_NAME}
  PROPERTIES
  ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib/${CMAKE_BUILD_TYPE}
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin/${CMAKE_BUILD_TYPE}
)
set_target_properties(${LIB_NAME} PROPERTIES VERSION ${PROJECT_VERSION})

set(LIBS_BASE ${CMAKE_CURRENT_LIST_DIR}/lib)
set(BGFX_BASE ${LIBS_BASE}/bgfx.cmake/bgfx)

target_include_directories(
    ${PROJECT_NAME}
    PUBLIC
    $<INSTALL_INTERFACE:include>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
    ${BGFX_BASE}/3rdparty
    PRIVATE
    ${BGFX_BASE}/examples/common
    ${CMAKE_CURRENT_SOURCE_DIR}/src
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
add_subdirectory(${LIBS_BASE}/glfw)
target_link_libraries(${LIB_NAME} glfw)
add_compile_definitions(DARMOK_CONFIG_USE_GLFW)

# bgfx
add_subdirectory(${LIBS_BASE}/bgfx.cmake)
target_link_libraries(${LIB_NAME} bgfx bimg bx)

# imgui
include(${CMAKE_CURRENT_LIST_DIR}/CMake/imgui.cmake)
target_link_libraries(${LIB_NAME} imgui)