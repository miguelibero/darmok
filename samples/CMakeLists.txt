function(darmok_sample_setup)
  cmake_parse_arguments(ARGS "" "TARGET;INPUT;OUTPUT" "SOURCES;ASSETS" "${ARGN}")
  if(NOT ARGS_SOURCES)
    set(ARGS_SOURCES src/main.cpp)
  endif()
  add_executable(
    ${ARGS_TARGET}
    ${ARGS_SOURCES}
    ${ARGS_ASSETS}
  )
  darmok_process_assets(
    TARGET ${ARGS_TARGET}-assets
    INPUT ${ARGS_INPUT}
    OUTPUT ${ARGS_OUTPUT}
    DEPENDS ${ASSETS}
  )
  source_group("Asset Files" FILES ${ASSETS})
  target_compile_definitions(${ARGS_TARGET} PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1")
  target_link_libraries(${ARGS_TARGET} PUBLIC darmok)
  darmok_copy_dynamic_libs(${ARGS_TARGET})
  set_target_properties(${ARGS_TARGET} PROPERTIES FOLDER "darmok/samples")
  add_dependencies(${ARGS_TARGET} ${ARGS_TARGET}-assets)
  endfunction()

function(darmok_lua_sample_setup)
  cmake_parse_arguments(ARGS "" "TARGET" "ASSETS" "${ARGN}")
  add_custom_target(${ARGS_TARGET}
    COMMAND $<TARGET_FILE:darmok-lua>
    DEPENDS darmok-lua
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    SOURCES ${ARGS_ASSETS}
    COMMENT "running darmok lua executable"
  )
  set_target_properties(${TARGET} PROPERTIES FOLDER "darmok/samples")
endfunction()

add_subdirectory(imgui)
add_subdirectory(rmlui)
add_subdirectory(scene)
add_subdirectory(assimp)
add_subdirectory(pbr)
add_subdirectory(lua)
add_subdirectory(ozz)
add_subdirectory(jolt)