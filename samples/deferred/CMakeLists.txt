if(NOT ${DARMOK_BUILD_ASSIMP})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-deferred
  ASSETS
    darmok-import.json
    Sponza.gltf
)