set(SAMPLE_NAME darmok-samples-pbr)

add_executable(
  ${SAMPLE_NAME}
  src/main.cpp
)

target_compile_definitions(
  ${SAMPLE_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)

target_link_libraries(${SAMPLE_NAME} PUBLIC darmok::darmok)
darmok_copy_linked_dlls(${SAMPLE_NAME})
set_target_properties(darmok-samples-pbr PROPERTIES FOLDER "darmok/samples")
