include(darmokUtils)

# darmok_process_skeleton(
#   FILES asset files
#   CONFIG ozz json config file
# 	OUTPUT_DIR directory
# )
function(darmok_process_skeleton)
  set(OPTIONS "") 
  set(ONE_VALUE_ARGS CONFIG OUTPUT_DIR)
  set(MULTI_VALUE_ARGS FILES)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")
  
  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
    set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
  endif()

  foreach(FILE_PATH ${ARGS_FILES})
    if(NOT IS_ABSOLUTE ${FILE_PATH})
      set(FILE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${FILE_PATH})
    endif()
    get_filename_component(FILE_EXT ${FILE_PATH} EXT)
    if(FILE_EXT STREQUAL ".fbx")
      set(OZZ_TARGET "fbx2ozz")
    else()
      set(OZZ_TARGET "gltf2ozz")
    endif()
    set(CMD $<TARGET_FILE:${OZZ_TARGET}> --file="${FILE_PATH}")
    if(ARGS_CONFIG)
      list(APPEND CMD --config_file="${ARGS_CONFIG}")
    endif()

    # use symbolic output since the output files depend on the config
    get_filename_component(FILE_NAME_WE ${FILE_PATH} NAME_WE)
    set(SYMBOLIC_OUTPUT "${FILE_NAME_WE}.ozz")
    set_property(SOURCE ${SYMBOLIC_OUTPUT} PROPERTY SYMBOLIC)

    add_custom_command(
      OUTPUT ${SYMBOLIC_OUTPUT}
      COMMAND ${CMD}
      WORKING_DIRECTORY ${ARGS_OUTPUT_DIR}
      MAIN_DEPENDENCY ${FILE_PATH}
      DEPENDS ${OZZ_TARGET} ${ARGS_CONFIG}
    )
  endforeach()
endfunction()