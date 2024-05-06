# darmok_process_vertex_layout(
#   HEADER
# 	LAYOUTS filenames
# 	OUTPUT_DIR directory
#   ARGS_OUT_VAR variable
# )
#
function(darmok_process_vertex_layout)
    set(OPTIONS HEADER)
    set(ONE_VALUE_ARGS OUTPUT_DIR ARGS_OUT_VAR)
    set(MULTI_VALUE_ARGS LAYOUTS)
    cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

    if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
        set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
    endif()

    set(OUTPUTS "")

    foreach(LAYOUT_PATH ${ARGS_LAYOUTS})
        get_filename_component(LAYOUT_PATH_NAME_WE ${LAYOUT_PATH} NAME_WLE)

        if(NOT IS_ABSOLUTE ${LAYOUT_PATH})
            set(LAYOUT_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${LAYOUT_PATH})
        endif()

        if(${ARGS_HEADER})
            set(OUTPUT ${ARGS_OUTPUT_DIR}/${LAYOUT_PATH_NAME_WE}.h)
            string(REPLACE "." "_" LAYOUT_ARRAY_NAME "${LAYOUT_PATH_NAME_WE}")
            bgfx_compile_binary_to_header(
                INPUT_FILE ${LAYOUT_PATH}
                OUTPUT_FILE ${OUTPUT}
                ARRAY_NAME ${LAYOUT_ARRAY_NAME}
            )
        else()
            set(OUTPUT ${ARGS_OUTPUT_DIR}/${LAYOUT_PATH_NAME_WE}.json)
            add_custom_command(
                OUTPUT ${OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy ${LAYOUT_PATH} ${OUTPUT}
                MAIN_DEPENDENCY ${LAYOUT_PATH}
            )
        endif()
        list(APPEND OUTPUTS ${OUTPUT})
    endforeach()
    if(NOT ${ARGS_OUT_VAR} STREQUAL "")
        set(${ARGS_OUT_VAR} ${OUTPUTS} PARENT_SCOPE)
    endif()
endfunction()