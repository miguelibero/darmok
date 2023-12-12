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
  src/asset.cpp
  src/scene.cpp
  src/utils.cpp
  src/sprite.cpp
)

set(headers
  include/darmok/app.hpp
  include/darmok/input.hpp
  include/darmok/window.hpp
  include/darmok/utils.hpp
  include/darmok/asset.hpp
  include/darmok/imgui.hpp
  include/darmok/scene.hpp
  include/darmok/sprite.hpp
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
target_link_libraries(${LIB_NAME} PRIVATE glfw)
add_compile_definitions(DARMOK_CONFIG_USE_GLFW)

# glm
add_subdirectory(${LIBS_BASE}/glm)
target_link_libraries(${LIB_NAME} PUBLIC glm)
install(
  TARGETS
  glm
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
add_subdirectory(${LIBS_BASE}/bgfx.cmake)
target_link_libraries(${LIB_NAME} PUBLIC bgfx bimg bimg_decode bx)
set_property(TARGET ${LIB_NAME} PROPERTY BGFX_SHADER_INCLUDE_PATH  ${LIBS_BASE}/bgfx.cmake/bgfx/src)

# imgui
include(${CMAKE_CURRENT_LIST_DIR}/CMake/imgui.cmake)
target_link_libraries(${LIB_NAME} PUBLIC imgui)

# entt
add_subdirectory(${LIBS_BASE}/entt)
target_link_libraries(${LIB_NAME} PUBLIC EnTT)



# darmok_compile_shader(
	# 	TYPE VERTEX|FRAGMENT|COMPUTE
	# 	SHADERS filenames
	# 	VARYING_DEF filename
	# 	OUTPUT_DIR directory
	# 	INCLUDE_DIRS directories
	# )
	#
	function(darmok_compile_shader)
		set(options "")
		set(oneValueArgs TYPE VARYING_DEF OUTPUT_DIR OUT_FILES_VAR)
		set(multiValueArgs SHADERS INCLUDE_DIRS)
		cmake_parse_arguments(ARGS "${options}" "${oneValueArgs}" "${multiValueArgs}" "${ARGN}")

		set(PROFILES 120 300_es spirv) # pssl
		if(UNIX AND NOT APPLE)
			set(PLATFORM LINUX)
		elseif(EMSCRIPTEN)
			set(PLATFORM ASM_JS)
		elseif(APPLE)
			set(PLATFORM OSX)
			list(APPEND PROFILES metal)
		elseif(
			WIN32
			OR MINGW
			OR MSYS
			OR CYGWIN
		)
			set(PLATFORM WINDOWS)
			list(APPEND PROFILES s_4_0)
			list(APPEND PROFILES s_5_0)
		else()
			message(error "shaderc: Unsupported platform")
		endif()

    get_target_property(BGFX_SHADER_INCLUDE_PATH darmok BGFX_SHADER_INCLUDE_PATH)

		set(ALL_OUTPUTS "")
		foreach(SHADER_FILE ${ARGS_SHADERS})
			source_group("Shaders" FILES "${SHADER}")
			get_filename_component(SHADER_FILE_BASENAME ${SHADER_FILE} NAME)
			get_filename_component(SHADER_FILE_ABSOLUTE ${SHADER_FILE} ABSOLUTE)

			# Build output targets and their commands
			set(OUTPUTS "")
			set(COMMANDS "")
			foreach(PROFILE ${PROFILES})
				_bgfx_get_profile_ext(${PROFILE} PROFILE_EXT)
				set(OUTPUT ${ARGS_OUTPUT_DIR}/${SHADER_FILE_BASENAME}.${PROFILE_EXT}.bin)
				set(PLATFORM_I ${PLATFORM})
				if(PROFILE STREQUAL "spirv")
					set(PLATFORM_I LINUX)
				endif()
				_bgfx_shaderc_parse(
					CLI #
					${ARGS_TYPE} ${PLATFORM_I} WERROR "$<$<CONFIG:debug>:DEBUG>$<$<CONFIG:relwithdebinfo>:DEBUG>"
					FILE ${SHADER_FILE_ABSOLUTE}
					OUTPUT ${OUTPUT}
					PROFILE ${PROFILE}
					O "$<$<CONFIG:debug>:0>$<$<CONFIG:release>:3>$<$<CONFIG:relwithdebinfo>:3>$<$<CONFIG:minsizerel>:3>"
					VARYINGDEF ${CMAKE_SOURCE_DIR}/${ARGS_VARYING_DEF}
					INCLUDES ${BGFX_SHADER_INCLUDE_PATH} ${ARGS_INCLUDE_DIRS}
				)
				list(APPEND OUTPUTS ${OUTPUT})
				list(APPEND ALL_OUTPUTS ${OUTPUT})
				list(APPEND COMMANDS COMMAND bgfx::shaderc ${CLI})
			endforeach()

			add_custom_command(
				OUTPUT ${OUTPUTS}
				COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGS_OUTPUT_DIR} ${COMMANDS}
				MAIN_DEPENDENCY ${SHADER_FILE_ABSOLUTE}
				DEPENDS ${ARGS_VARYING_DEF}
			)
		endforeach()

		if(DEFINED ARGS_OUT_FILES_VAR)
			set(${ARGS_OUT_FILES_VAR} ${ALL_OUTPUTS} PARENT_SCOPE)
		endif()
	endfunction()