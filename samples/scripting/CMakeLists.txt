project(
  darmok::samples::scripting
  LANGUAGES CXX
  DESCRIPTION "scripting example darmok project"
)

set(DARMOK_BUILD_SCRIPTING ON CACHE BOOL "")
set(DARMOK_BUILD_SCRIPTING_EXE ON CACHE BOOL "")

darmok_process_assets()

add_custom_target(${PROJECT_NAME}
  COMMAND $<TARGET_FILE:darmok-scripting>
  DEPENDS darmok::scripting
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
  SOURCES ${ASSETS}
  COMMENT "running darmok scripting executable"
)