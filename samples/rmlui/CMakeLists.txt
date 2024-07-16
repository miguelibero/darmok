if(NOT ${DARMOK_BUILD_RMLUI})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-rmlui
  ASSETS 
    assets/hello_world.rml
    assets/LatoLatin-Regular.ttf
    assets/window.rcss
)