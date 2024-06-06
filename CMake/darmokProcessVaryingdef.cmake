
# darmok_process_varyingdef(
#   HEADER
# 	VARYING_DEFS filenames
# 	OUTPUT_DIR directory
#   HEADER_VAR variable name
# )
#
function(darmok_process_varyingdef)
    set(OPTIONS HEADER) 
    set(ONE_VALUE_ARGS OUTPUT_DIR HEADER_VAR)
    set(MULTI_VALUE_ARGS VARYING_DEFS)
    cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

    if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
        set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
    endif()

    if(ARGS_HEADER_INCLUDE_DIR OR ARGS_HEADER_VAR_PREFIX)
        set(ARGS_HEADER ON)
    endif()

    if(ARGS_HEADER)
        set(OUTPUT_EXT "h")
    else()
        set(OUTPUT_EXT "dvl")
    endif()

    set(VLAYOUTC_EXE "darmok::vlayoutc")

    foreach(VARYING_DEF_PATH ${ARGS_VARYING_DEFS})
        if(NOT IS_ABSOLUTE ${VARYING_DEF_PATH})
            set(VARYING_DEF_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${VARYING_DEF_PATH})
        endif()
        get_filename_component(VARYING_DEF_NAME_WE ${VARYING_DEF_PATH} NAME_WLE)
        set(OUTPUT_FILE ${VARYING_DEF_NAME_WE}.${OUTPUT_EXT})
        set(OUTPUT ${ARGS_OUTPUT_DIR}/${OUTPUT_FILE})
        set(CMD $<TARGET_FILE:${VLAYOUTC_EXE}> -i ${VARYING_DEF_PATH} -o ${OUTPUT})
        if(ARGS_HEADER)
            if(ARGS_HEADER_VAR)
                set(HEADER_VAR ${ARGS_HEADER_VAR})
            else()
                set(HEADER_VAR ${VARYING_DEF_NAME_WE})
            endif()
            string(APPEND CMD -bin2c ${HEADER_VAR})
        endif()
        add_custom_command(
            OUTPUT ${OUTPUT}
            COMMAND ${CMD}
            WORKING_DIRECTORY ${ARGS_OUTPUT_DIR}
            MAIN_DEPENDENCY ${VARYING_DEF_PATH}
            DEPENDS ${VLAYOUT_EXE}
        )
    endforeach()
endfunction()