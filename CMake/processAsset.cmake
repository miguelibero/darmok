# darmok_process_asset(
#   HEADER
# 	FILES filenames
# 	OUTPUT_DIR directory
#   HEADER_VAR_PREFIX prefix
#   ARGS_OUT_VAR variable
# )
#
function(darmok_process_asset)
    set(OPTIONS HEADER)
    set(ONE_VALUE_ARGS OUTPUT_DIR HEADER_VAR_PREFIX OUT_VAR)
    set(MULTI_VALUE_ARGS FILES)
    cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

    if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
        set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
    endif()

    set(OUTPUTS "")

    foreach(FILE_PATH ${ARGS_FILES})
        get_filename_component(FILE_PATH_NAME_WE ${FILE_PATH} NAME_WLE)

        if(NOT IS_ABSOLUTE ${FILE_PATH})
            set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
        endif()

        if(ARGS_HEADER)
            set(OUTPUT ${ARGS_OUTPUT_DIR}/${FILE_PATH_NAME_WE}.h)
            string(REPLACE "." "_" ARRAY_NAME "${FILE_PATH_NAME_WE}")
            bgfx_compile_binary_to_header(
                INPUT_FILE ${FILE_PATH}
                OUTPUT_FILE ${OUTPUT}
                ARRAY_NAME ${ARRAY_NAME}
                HEADER_VAR_PREFIX ${ARGS_HEADER_VAR_PREFIX}
            )
        else()
            set(OUTPUT ${ARGS_OUTPUT_DIR}/${FILE_PATH_NAME_WE}.json)
            add_custom_command(
                OUTPUT ${OUTPUT}
                COMMAND ${CMAKE_COMMAND} -E copy ${FILE_PATH} ${OUTPUT}
                MAIN_DEPENDENCY ${FILE_PATH}
            )
        endif()
        list(APPEND OUTPUTS ${OUTPUT})
    endforeach()
    if(NOT ${ARGS_OUT_VAR} STREQUAL "")
        set(${ARGS_OUT_VAR} ${OUTPUTS} PARENT_SCOPE)
    endif()
endfunction()