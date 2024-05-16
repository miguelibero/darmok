set(SAMPLE_NAME darmok-samples-pbr)

add_executable(
  ${SAMPLE_NAME}
  src/main.cpp
)

target_compile_definitions(
  ${SAMPLE_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)

darmok_sample_setup(${SAMPLE_NAME})