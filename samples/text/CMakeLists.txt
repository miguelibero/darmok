if(NOT ${DARMOK_BUILD_FREETYPE})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-text
  ASSETS
    assets/darmok-import.json
    assets/COMIC.ttf
)