if(NOT ${DARMOK_BUILD_OZZ})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-ozz
  ASSETS
    assets/animator.json
    assets/BasicMotions@Idle01.fbx
    assets/BasicMotions@Idle01.skel.json
    assets/BasicMotions@Run01.fbx
    assets/BasicMotions@Run01.skel.json
    assets/BasicMotions@Talk01.fbx
    assets/BasicMotions@Talk01.skel.json
    assets/BasicMotionsDummyModel.fbx
    assets/BasicMotionsDummyModel.model.json
    assets/BasicMotionsDummyModel.skel.json
    assets/BasicMotionsTexture.png
)
