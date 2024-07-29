if(NOT ${DARMOK_BUILD_MINIAUDIO})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-audio
  ASSETS
    darmok-import.json
    sound.wav
)