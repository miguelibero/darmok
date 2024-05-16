
if(NOT ${DARMOK_BUILD_ASSIMP})
  return()
endif()

set(SAMPLE_NAME darmok-samples-assimp)

darmok_process_assets(
  SOURCE_GROUP_NAME "Sample Asset Files"
)

add_executable(
  ${SAMPLE_NAME}
  src/main.cpp
  ${ASSETS}
)

darmok_sample_setup(${SAMPLE_NAME})