if(NOT ${DARMOK_BUILD_ASSIMP})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-assimp
  ASSETS
    assets/human.fbx
    assets/human.png
)