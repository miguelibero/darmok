# darmok_copy_dynamic_libs(
#   target name
# )
function(darmok_copy_dynamic_libs TARGET)
  # TODO: fix this so that the dependency is copied when it is recompiled
  # rn this only happens if the target is recompiled
  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -D BUILD_DIR=${CMAKE_BINARY_DIR} -D TARGET=$<TARGET_FILE:${TARGET}> -P ${CMAKE_CURRENT_FUNCTION_LIST_DIR}/copyDynamicLibs.cmake
  )
endfunction()