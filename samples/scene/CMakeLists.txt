
set(SAMPLE_NAME darmok-samples-scene)

darmok_process_assets(
  SOURCE_GROUP_NAME "Sample Asset Files"
)

add_executable(
  ${SAMPLE_NAME}
  src/main.cpp
  ${ASSETS}
)

darmok_sample_setup(${SAMPLE_NAME})