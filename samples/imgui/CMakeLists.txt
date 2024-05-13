if(NOT ${DARMOK_BUILD_IMGUI})
  return()
endif()

set(SAMPLE_NAME darmok-samples-imgui)

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