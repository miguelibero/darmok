include(darmokUtils)

# darmok_process_model(
#   FILES model files
#   CONFIG ozz json config file
# 	OUTPUT_DIR directory
# )
function(darmok_process_model)
  set(OPTIONS "") 
  set(ONE_VALUE_ARGS CONFIG OUTPUT_DIR)
  set(MULTI_VALUE_ARGS FILES)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
    set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
  endif()

  set(MODELC_TARGET darmok-modelc)

  foreach(FILE_PATH ${ARGS_FILES})
    if(NOT IS_ABSOLUTE ${FILE_PATH})
      set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    endif()

    get_filename_component(FILE_NAME_WE ${FILE_PATH} NAME_WE)
    set(OUTPUT_FILE ${FILE_NAME_WE}.dml)
    set(OUTPUT ${ARGS_OUTPUT_DIR}/${OUTPUT_FILE})

    set(CMD $<TARGET_FILE:${MODELC_TARGET}> -i "${FILE_PATH}" -o "${OUTPUT}")
    if(ARGS_CONFIG)
      list(APPEND CMD -c "${ARGS_CONFIG}")
    endif()
  
    add_custom_command(
      OUTPUT ${OUTPUT}
      COMMAND ${CMD}
      WORKING_DIRECTORY ${ARGS_OUTPUT_DIR}
      MAIN_DEPENDENCY ${ASSET_PATH}
      DEPENDS ${MODELC_TARGET} ${ARGS_CONFIG}
    )
  endforeach()
endfunction()