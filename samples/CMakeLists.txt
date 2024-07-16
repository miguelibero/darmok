set(SHARED_ASSETS_TARGET darmok-samples-assets)
darmok_process_assets(
  TARGET ${SHARED_ASSETS_TARGET}
  ASSETS
    assets/darmok-import.json
    assets/noto.ttf
)
set_target_properties(${SHARED_ASSETS_TARGET} PROPERTIES FOLDER "darmok/samples")

function(darmok_sample_setup)
  cmake_parse_arguments(ARGS "" "TARGET;ASSETS_INPUT;ASSETS_OUTPUT" "SOURCES;ASSETS" "${ARGN}")
  if(NOT ARGS_SOURCES)
    set(ARGS_SOURCES src/main.cpp)
  endif()
  add_executable(
    ${ARGS_TARGET}
    ${ARGS_SOURCES}
    ${ARGS_ASSETS}
  )
  target_compile_definitions(${ARGS_TARGET} PRIVATE "DARMOK_IMPLEMENT_MAIN=1")
  target_link_libraries(${ARGS_TARGET} PUBLIC darmok)
  darmok_copy_dynamic_libs(${ARGS_TARGET})
  set_target_properties(${ARGS_TARGET} PROPERTIES FOLDER "darmok/samples")
  add_dependencies(${ARGS_TARGET} ${SHARED_ASSETS_TARGET})

  if(ARGS_ASSETS)
    set(ASSETS_TARGET ${ARGS_TARGET}-assets)
    darmok_process_assets(
      TARGET ${ASSETS_TARGET}
      INPUT ${ARGS_ASSETS_INPUT}
      OUTPUT ${ARGS_ASSETS_OUTPUT}
      ASSETS ${ARGS_ASSETS}
    )
    add_dependencies(${ARGS_TARGET} ${ASSETS_TARGET})
    set_target_properties(${ASSETS_TARGET} PROPERTIES FOLDER "darmok/samples")
  endif()

endfunction()

function(darmok_lua_sample_setup)
  cmake_parse_arguments(ARGS "" "TARGET" "ASSETS" "${ARGN}")
  add_custom_target(${ARGS_TARGET}
    COMMAND $<TARGET_FILE:darmok-lua>
    DEPENDS darmok-lua
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    COMMENT "running darmok lua executable"
  )
  set_target_properties(${ARGS_TARGET} PROPERTIES FOLDER "darmok/samples")
  add_dependencies(${ARGS_TARGET} ${SHARED_ASSETS_TARGET})
  
  if(ARGS_ASSETS)
    set(ASSETS_TARGET ${ARGS_TARGET}-assets)
    darmok_process_assets(
      TARGET ${ASSETS_TARGET}
      INPUT ${ARGS_ASSETS_INPUT}
      OUTPUT ${ARGS_ASSETS_OUTPUT}
      ASSETS ${ARGS_ASSETS}
    )
    add_dependencies(${ARGS_TARGET} ${ASSETS_TARGET})
    set_target_properties(${ASSETS_TARGET} PROPERTIES FOLDER "darmok/samples")
  endif()
endfunction()

add_subdirectory(imgui)
add_subdirectory(rmlui)
add_subdirectory(scene)
add_subdirectory(text)
add_subdirectory(assimp)
add_subdirectory(pbr)
add_subdirectory(lua)
add_subdirectory(ozz)
add_subdirectory(jolt)