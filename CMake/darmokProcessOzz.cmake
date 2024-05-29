include(darmokUtils)

# darmok_process_ozz(
#   ASSETS fbx or gltf files
#   CONFIG ozz json config file
# 	OUTPUT_DIR directory
#   SOURCES_VAR variable
# )
function(darmok_process_ozz)
    set(OPTIONS "") 
    set(ONE_VALUE_ARGS CONFIG OUTPUT_DIR SOURCES_VAR)
    set(MULTI_VALUE_ARGS ASSETS)
    cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")
    
    if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
        set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
    endif()

    foreach(ASSET_PATH ${ARGS_ASSETS})
        if(NOT IS_ABSOLUTE ${ASSET_PATH})
            set(ASSET_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${ASSET_PATH})
        endif()
        get_filename_component(ASSET_EXT ${ASSET_PATH} EXT)
        if(${ASSET_EXT} STREQUAL ".fbx")
            set(OZZ_EXE "fbx2ozz")
        else()
            set(OZZ_EXE "gltf2ozz")
        endif()
        set(CMD $<TARGET_FILE:${OZZ_EXE}> --file="${ASSET_PATH}")
        if(DEFINED ARGS_CONFIG)
            list(APPEND CMD --config_file="${ARGS_CONFIG}")
        endif()

        # use symbolic output since the output files depend on the config
        get_filename_component(ASSET_NAME_WE ${ASSET_PATH} NAME_WE)
        set(SYMBOLIC_OUTPUT "${ASSET_NAME_WE}.ozz")
        set_property(SOURCE ${SYMBOLIC_OUTPUT} PROPERTY SYMBOLIC)

        add_custom_command(
            OUTPUT ${SYMBOLIC_OUTPUT}
            COMMAND ${CMD}
            WORKING_DIRECTORY ${ARGS_OUTPUT_DIR}
            MAIN_DEPENDENCY ${ASSET_PATH}
            DEPENDS ${OZZ_EXE} ${ARGS_CONFIG}
        )
    endforeach()
endfunction()