# darmok_process_assets(
#   INPUT files or pattern
# 	OUTPUT_DIR directory
#   SOURCES_VAR variable
#   SOURCE_GROUP_NAME name
# )
function(darmok_process_assets)
  set(ONE_VALUE_ARGS INPUT OUTPUT_DIR SOURCES_VAR SOURCE_GROUP_NAME)
  cmake_parse_arguments(ARGS "" "${ONE_VALUE_ARGS}" "" "${ARGN}")
  
  if(NOT DEFINED ARGS_INPUT)
    set(ARGS_INPUT assets)
  endif()
  set(SOURCES "")
  if(IS_DIRECTORY ${ARGS_INPUT})
    file(GLOB_RECURSE SOURCES ${ARGS_INPUT}/*)
  elseif(EXISTS ${ARGS_INPUT})
    list(APPEND SOURCES ${ARGS_INPUT})
  endif()

  if(ARGS_SOURCES_VAR)
    set(${ARGS_SOURCES_VAR} ${SOURCES} PARENT_SCOPE)
  endif()
endfunction()