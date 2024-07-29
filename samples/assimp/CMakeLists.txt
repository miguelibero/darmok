if(NOT ${DARMOK_BUILD_ASSIMP})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-assimp
  ASSETS
    darmok-import.json
    human.fbx
    human.png
)