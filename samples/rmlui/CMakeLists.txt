if(NOT ${DARMOK_BUILD_RMLUI})
  return()
endif()

darmok_sample_setup(
  TARGET darmok-samples-rmlui
  ASSETS 
    hello_world.rml
    LatoLatin-Regular.ttf
    window.rcss
)