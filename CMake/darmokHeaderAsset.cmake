# darmok_header_asset(
#   HEADER
# 	FILES filenames
# 	OUTPUT_DIR directory
#   HEADER_VAR_PREFIX prefix
# )
#
function(darmok_header_asset)
  set(OPTIONS HEADER)
  set(ONE_VALUE_ARGS OUTPUT_DIR HEADER_VAR_PREFIX)
  set(MULTI_VALUE_ARGS FILES)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
    set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
  endif()

  foreach(FILE_PATH ${ARGS_FILES})
    get_filename_component(FILE_PATH_NAME_WE ${FILE_PATH} NAME_WLE)

    if(NOT IS_ABSOLUTE ${FILE_PATH})
      set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    endif()

    if(ARGS_HEADER)
      set(OUTPUT ${ARGS_OUTPUT_DIR}/${FILE_PATH_NAME_WE}.h)
      string(REPLACE "." "_" ARRAY_NAME "${FILE_PATH_NAME_WE}")
      set(ARRAY_NAME "${ARGS_HEADER_VAR_PREFIX}${ARRAY_NAME}")
      bgfx_compile_binary_to_header(
        INPUT_FILE ${FILE_PATH}
        OUTPUT_FILE ${OUTPUT}
        ARRAY_NAME ${ARRAY_NAME}
      )
    else()
      set(OUTPUT ${ARGS_OUTPUT_DIR}/${FILE_PATH_NAME_WE}.json)
      add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy ${FILE_PATH} ${OUTPUT}
        MAIN_DEPENDENCY ${FILE_PATH}
      )
    endif()
  endforeach()
endfunction()