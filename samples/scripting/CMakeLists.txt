if(NOT ${DARMOK_BUILD_SCRIPTING_EXE})
  return()
endif()

set(SAMPLE_NAME darmok-samples-scripting)

darmok_process_assets()

add_custom_target(${SAMPLE_NAME}
  COMMAND $<TARGET_FILE:darmok-scripting>
  DEPENDS darmok::scripting
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  SOURCES ${ASSETS}
  COMMENT "running darmok scripting executable"
)