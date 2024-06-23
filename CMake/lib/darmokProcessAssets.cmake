# darmok_process_assets(
#   [CORE]
#   TARGET name of the custom target that will process the assets
#   INPUT file or directory
# 	OUTPUT file or directory
#   DEPENDS list of files
# )
function(darmok_process_assets)
  set(OPTION_ARGS CORE)
  set(ONE_VALUE_ARGS TARGET INPUT OUTPUT)
  set(MULTI_VALUE_ARGS DEPENDS)
  cmake_parse_arguments(ARGS "${OPTION_ARGS}" "${ONE_VALUE_ARGS}" "${MULTI_VALUE_ARGS}" "${ARGN}")

  if(NOT DEFINED ARGS_INPUT)
    set(ARGS_INPUT assets)
  endif()
  if(NOT DEFINED ARGS_OUTPUT)
    set(ARGS_OUTPUT assets)
  endif()
  if(NOT DEFINED ARGS_TARGET)
    set(ARGS_TARGET assets)
  endif()
  if(NOT IS_ABSOLUTE ${ARGS_INPUT})
    set(ARGS_INPUT ${CMAKE_CURRENT_SOURCE_DIR}/${ARGS_INPUT})
  endif()
  if(NOT IS_ABSOLUTE ${ARGS_OUTPUT})
    set(ARGS_OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/${ARGS_OUTPUT})
  endif()

  if(ARGS_CORE)
    set(EXE_TARGET darmok-coreassetc)
  else()
    set(EXE_TARGET darmok-assetc)
  endif()
  set(CMD $<TARGET_FILE:${EXE_TARGET}>
    --input ${ARGS_INPUT}
    --output ${ARGS_OUTPUT}
    --bgfx-shaderc $<TARGET_FILE:bgfx::shaderc>
    --bgfx-shader-include ${BGFX_SHADER_INCLUDE_PATH}
  )
  add_custom_target(${ARGS_TARGET} ${CMD}
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
    DEPENDS ${ARGS_DEPENDS}
    COMMENT "processing darmok assets in ${ARGS_INPUT}..."
  )
  add_dependencies(${ARGS_TARGET} ${EXE_TARGET})
endfunction()