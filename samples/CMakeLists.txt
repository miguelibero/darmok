add_subdirectory(imgui)
add_subdirectory(scene)
add_subdirectory(assimp)
add_subdirectory(pbr)
add_subdirectory(scripting)
add_subdirectory(ozz)

set_target_properties(
  darmok-samples-assimp
  darmok-samples-imgui
  darmok-samples-ozz
  darmok-samples-pbr
  darmok-samples-scene
  darmok-samples-scripting
  PROPERTIES FOLDER "darmok/samples"
)