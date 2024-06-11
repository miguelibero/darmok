# darmok_copy_dynamic_libs(
#   TARGET
# )
macro(darmok_copy_dynamic_libs TARGET)
  add_custom_command(TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -D BUILD_DIR=${CMAKE_BINARY_DIR} -D TARGET=$<TARGET_FILE:${TARGET}> -P ${CMAKE_SOURCE_DIR}/CMake/copyDynamicLibs.cmake
  )
endmacro()