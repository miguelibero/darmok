macro(darmok_sample_setup TARGET)
    target_compile_definitions(${TARGET} PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1")
    target_link_libraries(${TARGET} PUBLIC darmok)
    darmok_copy_dynamic_libs(${TARGET})
    set_target_properties(${TARGET} PROPERTIES FOLDER "darmok/samples")
endmacro()

macro(darmok_lua_sample_setup TARGET ASSETS)
    add_custom_target(${TARGET}
        COMMAND $<TARGET_FILE:darmok-lua>
        DEPENDS darmok-lua
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        SOURCES ${ASSETS}
        COMMENT "running darmok lua executable"
    )
    set_target_properties(${TARGET} PROPERTIES FOLDER "darmok/samples")
endmacro()

add_subdirectory(imgui)
add_subdirectory(rmlui)
add_subdirectory(scene)
add_subdirectory(assimp)
add_subdirectory(pbr)
add_subdirectory(lua)
add_subdirectory(ozz)