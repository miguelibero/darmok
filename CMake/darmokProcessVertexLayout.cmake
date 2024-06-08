# darmok_process_vertex_layout(
#   HEADER
# 	FILES filenames
# 	OUTPUT_DIR directory
#   HEADER_VAR_PREFIX variable name
# )
function(darmok_process_vertex_layout)
  set(OPTIONS HEADER) 
  set(ONE_VALUE_ARGS OUTPUT_DIR HEADER_VAR_PREFIX)
  set(MULTI_VALUE_ARGS FILES)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
    set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
  endif()

  if(ARGS_HEADER_INCLUDE_DIR OR ARGS_HEADER_VAR_PREFIX)
    set(ARGS_HEADER ON)
  endif()

  if(ARGS_HEADER)
    set(OUTPUT_SUFFIX ".vlayout.h")
  else()
    set(OUTPUT_SUFFIX ".dvl")
  endif()

  set(VLAYOUTC_EXE "darmok::vlayoutc")

  foreach(FILE_PATH ${ARGS_FILES})
    if(NOT IS_ABSOLUTE ${FILE_PATH})
      set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    endif()
    get_filename_component(FILE_NAME_WE ${FILE_PATH} NAME_WE)
    set(OUTPUT_FILE ${FILE_NAME_WE}${OUTPUT_SUFFIX})
    set(OUTPUT ${ARGS_OUTPUT_DIR}/${OUTPUT_FILE})
    set(CMD $<TARGET_FILE:${VLAYOUTC_EXE}> -i ${FILE_PATH} -o ${OUTPUT})
    if(ARGS_HEADER)
      set(HEADER_VAR "${FILE_NAME_WE}_vlayout")
      if(ARGS_HEADER_VAR_PREFIX)
        set(HEADER_VAR "${ARGS_HEADER_VAR_PREFIX}${HEADER_VAR}")
      endif()
      list(APPEND CMD --bin2c ${HEADER_VAR})
    endif()
    add_custom_command(
      OUTPUT ${OUTPUT}
      COMMAND ${CMD}
      WORKING_DIRECTORY ${ARGS_OUTPUT_DIR}
      MAIN_DEPENDENCY ${FILE_PATH}
      DEPENDS ${VLAYOUT_EXE}
    )
  endforeach()
endfunction()