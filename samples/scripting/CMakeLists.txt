if(NOT ${DARMOK_BUILD_SCRIPTING_EXE})
  return()
endif()

set(SAMPLE_NAME darmok-samples-scripting)

darmok_process_assets()

darmok_scripting_sample_setup(${SAMPLE_NAME} ${ASSETS})