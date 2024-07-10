if(NOT ${DARMOK_BUILD_JOLT})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-jolt
  ASSETS
    assets/noto.ttf
)