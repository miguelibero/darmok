if(NOT ${DARMOK_BUILD_IMGUI})
  return()
endif()

set(SAMPLE_NAME darmok-samples-imgui)

darmok_process_assets(
  SOURCE_GROUP_NAME "Sample Asset Files"
)

add_executable(
  ${SAMPLE_NAME}
  src/main.cpp
  ${ASSETS}
)

target_compile_definitions(
  ${SAMPLE_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)
target_link_libraries(${SAMPLE_NAME} PUBLIC darmok::darmok)
darmok_copy_linked_dlls(${SAMPLE_NAME})
set_target_properties(${SAMPLE_NAME} PROPERTIES FOLDER "darmok/samples")