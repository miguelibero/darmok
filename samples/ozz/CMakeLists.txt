if(NOT ${DARMOK_BUILD_OZZ})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-ozz
  ASSETS
    darmok-import.json
    animator.json
    BasicMotions_DummyModel.fbx
)
