include(FetchContent)
FetchContent_Declare(ozz-animation
    GIT_REPOSITORY https://github.com/guillaumeblanc/ozz-animation.git
    GIT_TAG master
)

# FBX SDK can be downloaded from https://aps.autodesk.com/developer/overview/fbx-sdk
set(ozz_build_tools ON CACHE BOOL "")
set(ozz_build_samples OFF CACHE BOOL "")
set(ozz_build_howtos OFF CACHE BOOL "")
set(ozz_build_tests OFF CACHE BOOL "")
set(ozz_build_msvc_rt_dll ON CACHE BOOL "")

set(FBX_MSVC_RT_DLL ON)

FetchContent_MakeAvailable(ozz-animation)

set(OZZ_TARGETS
  ozz_base
  ozz_animation
  ozz_animation_offline
  ozz_geometry
  ozz_options
  ozz_animation_fbx
  ozz_animation_tools
  json
)

set(OZZ_EXE_TARGETS
  dump2ozz
  fbx2ozz
  gltf2ozz
)
set_target_properties(${OZZ_TARGETS} ${OZZ_EXE_TARGETS}
  PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

set(OZZ_TARGETS_EXPORT_NAME "ozzTargets")
set(OZZ_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/ozz")
install(TARGETS ${OZZ_TARGETS}
   EXPORT ${OZZ_TARGETS_EXPORT_NAME}
   LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
   ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
   RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
   PUBLIC_HEADER DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}"
)
install(EXPORT ${OZZ_TARGETS_EXPORT_NAME}
  NAMESPACE ozz
  DESTINATION ${OZZ_INSTALL_DIR}
)