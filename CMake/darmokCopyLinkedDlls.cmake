macro(darmok_copy_linked_dlls TARGET)
  add_custom_command(
    TARGET ${TARGET} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
            $<TARGET_RUNTIME_DLLS:${TARGET}> $<TARGET_FILE_DIR:${TARGET}>
    COMMAND_EXPAND_LISTS
  )
endmacro()