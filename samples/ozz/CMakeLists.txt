if(NOT ${DARMOK_BUILD_OZZ})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-ozz
  ASSETS
    assets/darmok-import.json
    assets/animator.json
    assets/BasicMotions@Idle01.fbx
    assets/BasicMotions@Run01.fbx
    assets/BasicMotions@Talk01.fbx
    assets/BasicMotionsDummyModel.fbx
    assets/BasicMotionsTexture.png
)
