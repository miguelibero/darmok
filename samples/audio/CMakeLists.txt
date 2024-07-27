if(NOT ${DARMOK_BUILD_MINIAUDIO})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-audio
  ASSETS
    assets/darmok-import.json
    assets/sound.wav
)