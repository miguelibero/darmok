
# darmok_compile_shader(
# 	TYPE VERTEX|FRAGMENT|COMPUTE
#   HEADER
# 	SHADERS filenames
# 	VARYING_DEF filename
# 	OUTPUT_DIR directory
# 	INCLUDE_DIRS directories
# )
#
function(darmok_compile_shader)
    set(options HEADER)
    set(oneValueArgs TYPE VARYING_DEF BASE_OUTPUT_DIR OUTPUT_DIR OUT_FILES_VAR)
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
    if(ARGS_HEADER)
        set(OUTPUT_EXT "h")
    else()
        set(OUTPUT_EXT "bin")
    endif()
    set(ABS_OUTPUT_DIR ${ARGS_OUTPUT_DIR})
    if(NOT IS_ABSOLUTE ${ABS_OUTPUT_DIR} AND ARGS_BASE_OUTPUT_DIR)
        set(ABS_OUTPUT_DIR "${ARGS_BASE_OUTPUT_DIR}/${ABS_OUTPUT_DIR}")
    endif()
    if(NOT IS_ABSOLUTE ${ABS_OUTPUT_DIR})
        set(ABS_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/${ABS_OUTPUT_DIR}")
    endif()

    set(BGFX_SHADER_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/lib/bgfx.cmake/bgfx/src)

    set(ALL_OUTPUTS "")
    foreach(SHADER_FILE ${ARGS_SHADERS})
        source_group("Shaders" FILES "${SHADER}")
        get_filename_component(SHADER_FILE_BASENAME ${SHADER_FILE} NAME)
        get_filename_component(SHADER_FILE_NAME_WE ${SHADER_FILE} NAME_WE)
        get_filename_component(SHADER_FILE_ABSOLUTE ${SHADER_FILE} ABSOLUTE)

        # Build output targets and their commands
        set(OUTPUTS "")
        set(COMMANDS "")
        set(COMBINED_CONTENT "")
        foreach(PROFILE ${PROFILES})
            _bgfx_get_profile_ext(${PROFILE} PROFILE_EXT)
            set(OUTPUT_FILE ${SHADER_FILE_NAME_WE}.${PROFILE_EXT}.${OUTPUT_EXT})
            set(OUTPUT ${ABS_OUTPUT_DIR}/${OUTPUT_FILE})
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
                VARYINGDEF ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_VARYING_DEF}
                INCLUDES ${BGFX_SHADER_INCLUDE_PATH} ${ARGS_INCLUDE_DIRS}
                BIN2C BIN2C ${SHADER_FILE_NAME_WE}_${PROFILE_EXT}
            )
            list(APPEND OUTPUTS ${OUTPUT})
            list(APPEND ALL_OUTPUTS ${OUTPUT})
            list(APPEND COMMANDS COMMAND bgfx::shaderc ${CLI})
            if(ARGS_HEADER)
                set(COMBINED_CONTENT "${COMBINED_CONTENT}\n#include \"${ARGS_OUTPUT_DIR}/${OUTPUT_FILE}\"")
            endif()
        endforeach()

        if(ARGS_HEADER)
            set(OUTPUT ${ABS_OUTPUT_DIR}/${SHADER_FILE_NAME_WE}.${OUTPUT_EXT})
            file(WRITE ${OUTPUT} ${COMBINED_CONTENT})
            list(APPEND OUTPUTS ${OUTPUT})
            list(APPEND ALL_OUTPUTS ${OUTPUT})
        endif()

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