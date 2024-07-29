if(NOT ${DARMOK_BUILD_OZZ})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-ozz
  ASSETS
    darmok-import.json
    animator.json
    BasicMotions@Idle01.fbx
    BasicMotions@Run01.fbx
    BasicMotions@Talk01.fbx
    BasicMotionsDummyModel.fbx
    BasicMotionsTexture.png
)
