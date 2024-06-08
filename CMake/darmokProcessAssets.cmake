include(darmokUtils)
include(darmokProcessShader)
include(darmokHeaderAsset)
include(darmokProcessVertexLayout)
include(darmokProcessSkeleton)
include(darmokProcessModel)

# darmok_process_assets(
#   ASSETS files or pattern
#   ASSETS_EXCLUDES files or patterns
#   ASSETS_INCLUDES files or patterns
# 	OUTPUT_DIR directory
#   HEADER_VAR_PREFIX prefix
# 	HEADER_OUTPUT_DIR directory
# 	HEADER_SHADER_INCLUDE_DIR directory
#   SOURCES_VAR variable
#   SOURCE_GROUP_NAME name
# )
function(darmok_process_assets)
  set(OPTIONS "")
  set(ONE_VALUE_ARGS OUTPUT_DIR HEADER_OUTPUT_DIR HEADER_VAR_PREFIX HEADER_SHADER_INCLUDE_DIR SOURCES_VAR SOURCE_GROUP_NAME)
  set(MULTI_VALUE_ARGS ASSETS ASSETS_EXCLUDES ASSETS_INCLUDES)
  cmake_parse_arguments(ARGS "${OPTIONS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

  if(NOT DEFINED ARGS_ASSETS)
    set(ARGS_ASSETS assets)
  endif()
  if(NOT IS_ABSOLUTE ${ARGS_ASSETS})
    set(ARGS_ASSETS ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_ASSETS})
  endif()

  if(NOT DEFINED ARGS_OUTPUT_DIR)
    set(ARGS_OUTPUT_DIR assets)
  endif()
  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT_DIR})
    set(ARGS_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT_DIR})
  endif()
  if(NOT DEFINED ARGS_SOURCES_VAR)
    set(ARGS_SOURCES_VAR ASSETS)
  endif()
  if(NOT DEFINED ARGS_SOURCE_GROUP_NAME)
    set(ARGS_SOURCE_GROUP_NAME "Asset Files")
  endif()
  set(SHADER_HEADER "")
  set(POSSIBLE_HEADER_OUTPUT_DIR ${ARGS_OUTPUT_DIR})
  if(DEFINED ARGS_HEADER_OUTPUT_DIR)
    set(HEADER_OPTION "HEADER")
    set(POSSIBLE_HEADER_OUTPUT_DIR ${ARGS_HEADER_OUTPUT_DIR})
  endif()

  set(SOURCES "")

  macro(_darmok_process_shader_asset TYPE SHADER_PATH)
    _darmok_replace_ext(VARYING_DEF_PATH ${SHADER_PATH} ".varyingdef")
    darmok_process_shader(
      TYPE ${TYPE}
      ${HEADER_OPTION}
      SHADERS ${SHADER_PATH}
      VARYING_DEF ${VARYING_DEF_PATH}
      HEADER_VAR_PREFIX ${ARGS_HEADER_VAR_PREFIX}
      HEADER_INCLUDE_DIR ${ARGS_HEADER_SHADER_INCLUDE_DIR}
      OUTPUT_DIR ${POSSIBLE_HEADER_OUTPUT_DIR}
    )
    list(APPEND ASSET_SOURCES
      ${SHADER_PATH}
      ${VARYING_DEF_PATH})
  endmacro()

  foreach(ASSET_ELM ${ARGS_ASSETS})
    set(ASSET_SOURCES "")
    # check if the param is a directory, a pattern or a file
    if(IS_DIRECTORY ${ASSET_ELM})
      set(ASSET_BASE_PATH ${ASSET_ELM})
      file(GLOB_RECURSE ASSET_PATHS ${ASSET_ELM}/*)
    else()
      string(FIND ${ASSET_ELM} "*" POS)
      if(POS GREATER 0)
        string(SUBSTRING ${ASSET_ELM} 0 ${POS} ASSET_BASE_PATH)
        get_filename_component(ASSET_BASE_PATH ${ASSET_BASE_PATH} DIRECTORY)
        file(GLOB_RECURSE ASSET_PATHS ${ASSET_ELM})
      endif()
    endif()
    if(NOT DEFINED ASSET_BASE_PATH)
      get_filename_component(ASSET_BASE_PATH ${ASSET_ELM} DIRECTORY)
      set(ASSET_PATHS ${ASSET_ELM})
    endif()

    # look for assets based on the filename
    set(FRAGMENT_SHADERS "")
    set(VERTEX_SHADERS "")
    set(SHADER_INCLUDES "")
    set(VERTEX_LAYOUTS "")
    set(COPY_ASSETS "")
    set(SKELETON_ASSETS "")
    set(MODEL_ASSETS "")
    foreach(ASSET_PATH_ABS ${ASSET_PATHS})
      file(RELATIVE_PATH ASSET_PATH ${ASSET_BASE_PATH} ${ASSET_PATH_ABS})
      set(EXCLUDED OFF)
      if(ARGS_ASSETS_INCLUDES)
        set(EXCLUDED ON)
        foreach(INCLUDE ${ARGS_ASSETS_INCLUDES})
          if(ASSET_PATH MATCHES INCLUDE)
            set(EXCLUDED OFF)
          endif()
        endforeach()
      endif()
      foreach(EXCLUDE ${ARGS_ASSETS_EXCLUDES})
        if(ASSET_PATH MATCHES EXCLUDE)
          set(EXCLUDED ON)
        endif()
      endforeach()      
      if(EXCLUDED)
        continue()
      endif()
      get_filename_component(ASSET_EXT ${ASSET_PATH_ABS} EXT)
      _darmok_replace_ext(VLAYOUT_PATH ${ASSET_PATH_ABS} ".vlayout.json")
      if(ASSET_EXT STREQUAL ".fragment.sc")
        list(APPEND FRAGMENT_SHADERS ${ASSET_PATH_ABS})
      elseif(ASSET_EXT STREQUAL ".vertex.sc")
        list(APPEND VERTEX_SHADERS ${ASSET_PATH_ABS})
      elseif(ASSET_EXT STREQUAL ".include.sc")
        list(APPEND SHADER_INCLUDES ${ASSET_PATH_ABS})          
      elseif(ASSET_EXT STREQUAL ".varyingdef" AND NOT EXISTS ${VLAYOUT_PATH})
        list(APPEND VERTEX_LAYOUTS ${ASSET_PATH_ABS})  
      elseif(ASSET_EXT STREQUAL ".vlayout.json")
        list(APPEND VERTEX_LAYOUTS ${ASSET_PATH_ABS})
      else()
        _darmok_replace_ext(SKEL_CONFIG_PATH ${ASSET_PATH_ABS} ".skel.json")
        _darmok_replace_ext(MODEL_CONFIG_PATH ${ASSET_PATH_ABS} ".model.json")
        if(ASSET_PATH_ABS STREQUAL ${SKEL_CONFIG_PATH})
          continue()
        elseif(ASSET_PATH_ABS STREQUAL ${MODEL_CONFIG_PATH})
          continue()
        endif()
        if(EXISTS ${SKEL_CONFIG_PATH})
          list(APPEND SKELETON_ASSETS ${ASSET_PATH_ABS})
        elseif(EXISTS ${MODEL_CONFIG_PATH})
          list(APPEND MODEL_ASSETS ${ASSET_PATH_ABS})
        else()
          list(APPEND COPY_ASSETS ${ASSET_PATH})
        endif()
      endif()

    endforeach()

    foreach(ASSET_PATH ${FRAGMENT_SHADERS})
      _darmok_process_shader_asset(FRAGMENT ${ASSET_PATH})
    endforeach()
    foreach(ASSET_PATH ${VERTEX_SHADERS})
      _darmok_process_shader_asset(VERTEX ${ASSET_PATH})
    endforeach()

    if(VERTEX_LAYOUTS)
      darmok_process_vertex_layout(
        FILES ${VERTEX_LAYOUTS}
        ${HEADER_OPTION}
        HEADER_VAR_PREFIX ${ARGS_HEADER_VAR_PREFIX}
        OUTPUT_DIR ${POSSIBLE_HEADER_OUTPUT_DIR}
      )
      list(APPEND ASSET_SOURCES ${VERTEX_LAYOUTS})
    endif()

    foreach(ASSET_PATH ${SKELETON_ASSETS})
      _darmok_replace_ext(CONFIG_PATH ${ASSET_PATH} ".skel.json")
      darmok_process_skeleton(
        FILES ${ASSET_PATH}
        CONFIG ${CONFIG_PATH}
        OUTPUT_DIR ${ARGS_OUTPUT_DIR}
      )
      list(APPEND ASSET_SOURCES ${ASSET_PATH} ${CONFIG_PATH})
    endforeach()

    foreach(ASSET_PATH ${MODEL_ASSETS})
      _darmok_replace_ext(CONFIG_PATH ${ASSET_PATH} ".model.json")
      darmok_process_model(
        FILES ${ASSET_PATH}
        CONFIG ${OZZ_CONFIG_PATH}
        OUTPUT_DIR ${ARGS_OUTPUT_DIR}
      )
      list(APPEND ASSET_SOURCES ${ASSET_PATH} ${CONFIG_PATH})
    endforeach()

    foreach(ASSET_PATH ${COPY_ASSETS})
      set(SOURCE ${ASSET_BASE_PATH}/${ASSET_PATH})
      set(OUTPUT ${ARGS_OUTPUT_DIR}/${ASSET_PATH})
      add_custom_command(
        OUTPUT ${OUTPUT}
        COMMAND ${CMAKE_COMMAND} -E copy_if_different ${SOURCE} ${OUTPUT}
        MAIN_DEPENDENCY ${SOURCE}
      )
      list(APPEND ASSET_SOURCES ${SOURCE})
    endforeach()

    if(DEFINED ARGS_SOURCE_GROUP_NAME)
      source_group(TREE ${ASSET_BASE_PATH} PREFIX ${ARGS_SOURCE_GROUP_NAME} FILES ${ASSET_SOURCES})
    endif()
    list(APPEND SOURCES ${ASSET_SOURCES})
  endforeach()

  if(ARGS_SOURCES_VAR)
    set(${ARGS_SOURCES_VAR} ${SOURCES} PARENT_SCOPE)
  endif()

endfunction()