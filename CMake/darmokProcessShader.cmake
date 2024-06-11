
# darmok_process_shader(
# 	TYPE VERTEX|FRAGMENT|COMPUTE
#   HEADER
# 	SHADERS filenames
# 	VARYING_DEF filename
# 	OUTPUT_DIR directory
# 	HEADER_INCLUDE_DIR directory
# 	INCLUDE_DIRS directories
#   HEADER_VAR_PREFIX variable name
# )
#
function(darmok_process_shader)
  set(OPTIONS HEADER HEADER_VAR_PREFIX_REPLACE)
  set(ONE_VALUE_ARGS TYPE VARYING_DEF OUTPUT_DIR HEADER_INCLUDE_DIR HEADER_VAR_PREFIX)
  set(MULTI_VALUE_ARGS SHADERS INCLUDE_DIRS)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

  set(PROFILES 120 300_es spirv) # pssl
  if(UNIX AND NOT APPLE)
    set(PLATFORM LINUX)
  elseif(EMSCRIPTEN)
    set(PLATFORM ASM_JS)
  elseif(APPLE)
    set(PLATFORM OSX)
    list(APPEND PROFILES metal)
  elseif(WIN32 OR MINGW OR MSYS OR CYGWIN)
    set(PLATFORM WINDOWS)
    list(APPEND PROFILES s_4_0)
    list(APPEND PROFILES s_5_0)
  else()
    message(error "shaderc: Unsupported platform")
  endif()

  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
    set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
  endif()  
  if(ARGS_HEADER_INCLUDE_DIR OR ARGS_HEADER_VAR_PREFIX)
    set(ARGS_HEADER ON)
  endif()
  if(ARGS_HEADER)
    set(OUTPUT_EXT "h")
  else()
    set(OUTPUT_EXT "dsh")
  endif()

  if(NOT IS_ABSOLUTE ${ARGS_VARYING_DEF})
    set(ARGS_VARYING_DEF ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_VARYING_DEF})
  endif()

  set(OUTPUTS "")
  foreach(SHADER_PATH ${ARGS_SHADERS})
    get_filename_component(SHADER_PATH_BASENAME ${SHADER_PATH} NAME)
    get_filename_component(SHADER_NAME_WE ${SHADER_PATH} NAME_WLE)
    get_filename_component(SHADER_PATH_ABSOLUTE ${SHADER_PATH} ABSOLUTE)

    # Build output targets and their commands
    set(SHADER_OUTPUTS "")
    set(SHADER_COMMANDS "")
    set(SHADER_COMBINED_INCLUDES "")
    set(SHADER_HEADER_VAR_PREFIX ${ARGS_HEADER_VAR_PREFIX})
    if(NOT ARGS_HEADER_VAR_PREFIX_REPLACE)
      string(REPLACE "." "_" SHADER_HEADER_PATH_NAME_PREFIX ${SHADER_NAME_WE})
      string(APPEND SHADER_HEADER_VAR_PREFIX ${SHADER_HEADER_PATH_NAME_PREFIX})
    endif()
    foreach(PROFILE ${PROFILES})
      _bgfx_get_profile_ext(${PROFILE} PROFILE_EXT)
      set(OUTPUT_FILE ${SHADER_NAME_WE}.${PROFILE_EXT}.${OUTPUT_EXT})
      set(OUTPUT ${ARGS_OUTPUT_DIR}/${OUTPUT_FILE})
      set(PROFILE_PLATFORM ${PLATFORM})
      if(PROFILE STREQUAL "spirv")
        set(PROFILE_PLATFORM LINUX)
      endif()

      _bgfx_shaderc_parse(
        CLI #
        ${ARGS_TYPE} ${PROFILE_PLATFORM} WERROR "$<$<CONFIG:debug>:DEBUG>$<$<CONFIG:relwithdebinfo>:DEBUG>"
        FILE ${SHADER_PATH_ABSOLUTE}
        OUTPUT ${OUTPUT}
        PROFILE ${PROFILE}
        O "$<$<CONFIG:debug>:0>$<$<CONFIG:release>:3>$<$<CONFIG:relwithdebinfo>:3>$<$<CONFIG:minsizerel>:3>"
        VARYINGDEF ${ARGS_VARYING_DEF}
        INCLUDES ${BGFX_SHADER_INCLUDE_PATH} ${ARGS_INCLUDE_DIRS}
        BIN2C BIN2C ${SHADER_HEADER_VAR_PREFIX}_${PROFILE_EXT}
      )
      list(APPEND SHADER_OUTPUTS ${OUTPUT})
      list(APPEND OUTPUTS ${OUTPUT})
      list(APPEND SHADER_COMMANDS COMMAND bgfx::shaderc ${CLI})
      if(ARGS_HEADER)
        set(PROFILE_INCLUDE_PATH ${OUTPUT_FILE})
        if(DEFINED ARGS_HEADER_INCLUDE_DIR)
          set(PROFILE_INCLUDE_PATH ${ARGS_HEADER_INCLUDE_DIR}/${PROFILE_INCLUDE_PATH})
        endif()
        list(APPEND SHADER_COMBINED_INCLUDES ${PROFILE_INCLUDE_PATH})
      endif()
    endforeach()

    if(ARGS_HEADER)
      set(COMBINED_OUTPUT ${ARGS_OUTPUT_DIR}/${SHADER_NAME_WE}.${OUTPUT_EXT})
      list(APPEND OUTPUTS ${COMBINED_OUTPUT})
      string(REPLACE ";" "$<SEMICOLON>" GENERATOR_COMBINED_INCLUDES "${SHADER_COMBINED_INCLUDES}")
      list(APPEND SHADER_COMMANDS COMMAND ${CMAKE_COMMAND} -D HEADER_INCLUDES="${GENERATOR_COMBINED_INCLUDES}" -D OUTPUT="${COMBINED_OUTPUT}" -P ${CMAKE_SOURCE_DIR}/CMake/generateCombinedHeader.cmake)
    endif()

    add_custom_command(
      OUTPUT ${SHADER_OUTPUTS}
      COMMAND ${CMAKE_COMMAND} -E make_directory ${ARGS_OUTPUT_DIR} ${SHADER_COMMANDS}
      MAIN_DEPENDENCY ${SHADER_PATH_ABSOLUTE}
      DEPENDS ${ARGS_VARYING_DEF}
    )

  endforeach()
endfunction()