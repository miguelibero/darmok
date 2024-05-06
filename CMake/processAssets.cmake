set(CMAKE_MOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(${CMAKE_MOD_DIR}/processShader.cmake)
include(${CMAKE_MOD_DIR}/processVertexLayout.cmake)
include(${CMAKE_MOD_DIR}/processSkeletalAnimation.cmake)

# darmok_process_assets(
#   ASSETS files or pattern
# 	OUTPUT_DIR directory
# 	HEADER_SHADER_OUTPUT_DIR directory
# 	HEADER_SHADER_INCLUDE_DIR directory
#   SOURCES_VAR variable
# )
function(darmok_process_assets)
    set(OPTIONS "")
    set(ONE_VALUE_ARGS OUTPUT_DIR HEADER_SHADER_OUTPUT_DIR HEADER_SHADER_INCLUDE_DIR SOURCES_VAR)
    set(MULTI_VALUE_ARGS ASSETS)
    cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

    if(NOT DEFINED ARGS_OUTPUT_DIR)
        set(ARGS_OUTPUT_DIR assets)
    endif()
    if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
        set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
    endif()
    if(NOT DEFINED ARGS_ASSETS)
        set(ARGS_ASSETS assets)
    endif()
    if(NOT IS_ABSOLUTE ${ARGS_ASSETS})
        set(ARGS_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_ASSETS})
    endif()
    if(NOT DEFINED ARGS_SOURCES_VAR)
        set(ARGS_SOURCES_VAR ASSETS)
    endif()
    set(SHADER_HEADER "")
    set(SHADER_OUTPUT_DIR ${ARGS_OUTPUT_DIR})
    if(DEFINED ARGS_HEADER_SHADER_OUTPUT_DIR)
        set(SHADER_HEADER "HEADER")
        set(SHADER_OUTPUT_DIR ${ARGS_HEADER_SHADER_OUTPUT_DIR})
    endif()

    set(SOURCES "")

    macro(_darmok_replace_ext VAR INPUT_PATH EXT)
        get_filename_component(NAME_WE ${INPUT_PATH} NAME_WE)
        get_filename_component(INPUT_DIR ${INPUT_PATH} DIRECTORY)
        set(${VAR} ${INPUT_DIR}/${NAME_WE}${EXT})
    endmacro()

    macro(_darmok_process_shader_asset TYPE SHADER_PATH)
        _darmok_replace_ext(VARYINGDEF_PATH ${SHADER_PATH} ".varyingdef")
        darmok_process_shader(
            TYPE ${TYPE}
            ${SHADER_HEADER}
            SHADERS ${SHADER_PATH}
            VARYING_DEF ${VARYINGDEF_PATH}
            HEADER_INCLUDE_DIR ${ARGS_HEADER_SHADER_INCLUDE_DIR}
            OUTPUT_DIR ${SHADER_OUTPUT_DIR}
        )
        list(APPEND ASSET_SOURCES
            ${SHADER_PATH}
            ${VARYINGDEF_PATH})
    endmacro()

    foreach(ASSET_ELM ${ARGS_ASSETS})
        set(ASSET_SOURCES "")
        # check if the param is a directory, a pattern or a file
        if(IS_DIRECTORY ${ASSET_ELM})
            set(ASSET_BASE_PATH ${ASSET_ELM})
            file(GLOB_RECURSE ASSET_PATHS ${ASSET_ELM}/*)
        else()
            string(FIND ${ASSET_ELM} "*" POS)
            if(${POS} GREATER 0)
                string(SUBSTRING ${ASSET_ELM} 0 ${POS} ASSET_BASE_PATH)
                get_filename_component(ASSET_BASE_PATH ${ASSET_BASE_PATH} DIRECTORY)
                file(GLOB_RECURSE ASSET_PATHS ${ASSET_ELM})
            endif()
        endif()
        if(NOT DEFINED ASSET_BASE_PATH)
            get_filename_component(ASSET_BASE_PATH ${ASSET_ELM} DIRECTORY)
            set(ASSET_PATHS ${ASSET_ELM})
        endif()

        # look for assets based on the file suffix
        set(FRAGMENT_SHADERS "")
        set(VERTEX_SHADERS "")
        set(VARYING_DEFS "")
        set(SHADER_INCLUDES "")
        set(VERTEX_LAYOUTS "")
        set(LUA_SCRIPTS "")
        set(COPY_ASSETS "")
        set(SKELETAL_ANIMATIONS "")
        foreach(ASSET_PATH_ABS ${ASSET_PATHS})
            file(RELATIVE_PATH ASSET_PATH ${ASSET_BASE_PATH} ${ASSET_PATH_ABS})
            get_filename_component(ASSET_EXT ${ASSET_PATH_ABS} EXT)
            _darmok_replace_ext(OZZ_CONFIG_PATH ${ASSET_PATH_ABS} ".ozz.json")
            if(${ASSET_EXT} STREQUAL ".fragment.sc")
                list(APPEND FRAGMENT_SHADERS ${ASSET_PATH_ABS})
            elseif(${ASSET_EXT} STREQUAL ".vertex.sc")
                list(APPEND VERTEX_SHADERS ${ASSET_PATH_ABS})
            elseif(${ASSET_EXT} STREQUAL ".include.sc")
                list(APPEND SHADER_INCLUDES ${ASSET_PATH_ABS})                  
            elseif(${ASSET_EXT} STREQUAL ".varyingdef")
                list(APPEND VARYING_DEFS ${ASSET_PATH_ABS})  
            elseif(${ASSET_EXT} STREQUAL ".layout.json")
                list(APPEND VERTEX_LAYOUTS ${ASSET_PATH_ABS})
            elseif(EXISTS ${OZZ_CONFIG_PATH})
                list(APPEND SKELETAL_ANIMATIONS ${ASSET_PATH_ABS})                
            else()
                list(APPEND COPY_ASSETS ${ASSET_PATH})
            endif()
        endforeach()

        foreach(ASSET_PATH ${FRAGMENT_SHADERS})
            _darmok_process_shader_asset(FRAGMENT ${ASSET_PATH})
        endforeach()
        foreach(ASSET_PATH ${VERTEX_SHADERS})
            _darmok_process_shader_asset(VERTEX ${ASSET_PATH})
        endforeach()

        if(NOT "${VERTEX_LAYOUTS}" STREQUAL "")
            darmok_process_vertex_layout(
                LAYOUTS ${VERTEX_LAYOUTS}
                ${SHADER_HEADER}
                OUTPUT_DIR ${SHADER_OUTPUT_DIR}
            )
            list(APPEND ASSET_SOURCES ${VERTEX_LAYOUTS})
        endif()

        foreach(ASSET_PATH ${COPY_ASSETS})
            set(SOURCE ${ASSET_BASE_PATH}/${ASSET_PATH})
            set(OUTPUT ${ARGS_OUTPUT_DIR}/${ASSET_PATH})
            add_custom_command(
                OUTPUT ${OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy ${SOURCE} ${OUTPUT}
                MAIN_DEPENDENCY ${SOURCE}
            )
            list(APPEND ASSET_SOURCES ${SOURCE})
        endforeach()

        source_group(TREE ${ASSET_BASE_PATH} PREFIX "Asset Files" FILES ${ASSET_SOURCES})
        list(APPEND SOURCES ${ASSET_SOURCES})
    endforeach()
    if(NOT ${ARGS_SOURCES_VAR} STREQUAL "")
        set(${ARGS_SOURCES_VAR} ${SOURCES} PARENT_SCOPE)
    endif()

endfunction()