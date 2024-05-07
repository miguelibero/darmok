set(CMAKE_MOD_DIR ${CMAKE_CURRENT_SOURCE_DIR}/CMake)
include(${CMAKE_MOD_DIR}/utils.cmake)

# darmok_process_skeletal_animation(
#   SKELETONS fbx or gltf files
#   CONFIG ozz json config file
# 	OUTPUT_DIR directory
#   SOURCES_VAR variable
# )
function(darmok_process_ozz_skeleton)
    set(OPTIONS "") 
    set(ONE_VALUE_ARGS CONFIG OUTPUT_DIR SOURCES_VAR)
    set(MULTI_VALUE_ARGS SKELETONS)
    cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")
    
    if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
        set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
    endif()

    macro(_darmok_get_ozz_config_outputs VAR PATH)
        file(READ ${PATH} DATA)
        string(JSON SKEL_FILENAME ERROR_VARIABLE ERR GET ${DATA} skeleton filename)
        if("${ERR}" STREQUAL "NOTFOUND")
            list(APPEND ${VAR} ${ARGS_OUTPUT_DIR}/${SKEL_FILENAME})
        endif()
        string(JSON ANIM_LEN ERROR_VARIABLE ERR LENGTH ${DATA} animations)
        if("${ERR}" STREQUAL "NOTFOUND")
            foreach(ANIM_I RANGE ${ANIM_LEN})
                string(JSON ANIM_FILENAME ERROR_VARIABLE ERR GET ${DATA} animations ${ANIM_I} filename)
                if("${ERR}" STREQUAL "NOTFOUND")
                    list(APPEND ${VAR} ${ARGS_OUTPUT_DIR}/${ANIM_FILENAME})
                endif()
                string(JSON MOTION_FILENAME ERROR_VARIABLE ERR GET ${DATA} animations ${ANIM_I} tracks motion filename)
                if("${ERR}" STREQUAL "NOTFOUND")
                    list(APPEND ${VAR} ${ARGS_OUTPUT_DIR}/${MOTION_FILENAME})
                endif()
                string(JSON PROP_LEN ERROR_VARIABLE ERR LENGTH ${DATA} animations ${ANIM_I} tracks properties)
                if(NOT "${ERR}" STREQUAL "NOTFOUND")
                    continue()
                endif()
                foreach(PROP_I RANGE ${PROP_LEN})
                    string(JSON PROP_FILENAME ERROR_VARIABLE ERR GET ${DATA} animations ${ANIM_I} tracks properties ${PROP_I} filename)
                    if("${ERR}" STREQUAL "NOTFOUND")
                        list(APPEND ${VAR} ${ARGS_OUTPUT_DIR}/${PROP_FILENAME})
                    endif()
                endforeach()            
            endforeach()
        endif()
    endmacro()

    set(OUTPUTS "")
    if(DEFINED ARGS_CONFIG)
        if(NOT IS_ABSOLUTE ${ARGS_CONFIG})
            set(ARGS_CONFIG ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_CONFIG})
        endif()
        _darmok_get_ozz_config_outputs(OUTPUTS ${ARGS_CONFIG})
    endif()

    foreach(SKELETON_PATH ${ARGS_SKELETONS})
        if(NOT IS_ABSOLUTE ${SKELETON_PATH})
            set(SKELETON_PATH ${CMAKE_CURRENT_SOURCE_DIR}/${SKELETON_PATH})
        endif()
        get_filename_component(SKELETON_EXT ${SKELETON_PATH} EXT)
        if(${SKELETON_EXT} STREQUAL ".fbx")
            set(OZZ_EXE "fbx2ozz")
        else()
            set(OZZ_EXE "gltf2ozz")
        endif()
        set(CMD $<TARGET_FILE:${OZZ_EXE}> --file="${SKELETON_PATH}")
        if(DEFINED ARGS_CONFIG)
            list(APPEND CMD --config_file="${ARGS_CONFIG}")
        endif()

        # use symbolic output if no output found
        if("${OUTPUTS}" STREQUAL "")
            get_filename_component(SKELETON_NAME_WE ${SKELETON_PATH} NAME_WE)
            set(SYMBOLIC_OUTPUT "${SKELETON_NAME_WE}.ozz")
            set_property(SOURCE ${SYMBOLIC_OUTPUT} PROPERTY SYMBOLIC)
            list(APPEND OUTPUTS ${SYMBOLIC_OUTPUT})
        endif()

        add_custom_command(
            OUTPUT ${OUTPUTS}
            COMMAND ${CMD}
            WORKING_DIRECTORY ${ARGS_OUTPUT_DIR}
            MAIN_DEPENDENCY ${SKELETON_PATH}
            DEPENDS ${OZZ_EXE} ${ARGS_CONFIG}
        )
    endforeach()
endfunction()