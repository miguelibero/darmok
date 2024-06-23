if(NOT ${DARMOK_BUILD_LUA_EXE})
  return()
endif()

darmok_lua_sample_setup(
  TARGET darmok-samples-lua
  ASSETS
    assets/darmok-import.json
    assets/main.lua
)