if(NOT ${DARMOK_BUILD_ASSIMP})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-assimp
  ASSETS
    assets/darmok-import.json
    assets/human.fbx
    assets/human.png
)