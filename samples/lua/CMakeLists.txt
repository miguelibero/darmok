if(NOT ${DARMOK_BUILD_SCRIPTING_EXE})
  return()
endif()

set(SAMPLE_NAME darmok-samples-lua)

darmok_process_assets()

darmok_lua_sample_setup(${SAMPLE_NAME} ${ASSETS})