macro(darmok_sample_setup TARGET)
    target_compile_definitions(${TARGET} PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1")
    target_link_libraries(${TARGET} PUBLIC darmok)
    darmok_copy_dynamic_libs(${TARGET})
    set_target_properties(${TARGET} PROPERTIES FOLDER "darmok/samples")
endmacro()

macro(darmok_scripting_sample_setup TARGET ASSETS)
    add_custom_target(${TARGET}
        COMMAND $<TARGET_FILE:darmok-scripting>
        DEPENDS darmok-scripting
        WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
        SOURCES ${ASSETS}
        COMMENT "running darmok scripting executable"
    )
    set_target_properties(${TARGET} PROPERTIES FOLDER "darmok/samples")
endmacro()

add_subdirectory(imgui)
add_subdirectory(rmlui)
add_subdirectory(scene)
add_subdirectory(assimp)
add_subdirectory(pbr)
add_subdirectory(scripting)
add_subdirectory(ozz)