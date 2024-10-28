if(NOT ${DARMOK_BUILD_ASSIMP})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-deferred
  ASSETS
    darmok-import.json
)

# sponza not included to reduce repo size
# download sponza gltf from
# https://github.com/KhronosGroup/glTF-Sample-Models/tree/main/2.0/Sponza/glTF
# copy 2.0/Sponza/glTF to samples/deferred/assets/sponza folder