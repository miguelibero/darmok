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
set(FBX_MSVC_RT_DLL TRUE)

set(BUILD_SHARED_LIBS_OLD ${BUILD_SHARED_LIBS})
# always use static linking for ozz
SET(BUILD_SHARED_LIBS OFF CACHE BOOL "")
SET(FBX_SHARED OFF CACHE BOOL "")
FetchContent_MakeAvailable(ozz-animation)
# recover old value
set(BUILD_SHARED_LIBS ${BUILD_SHARED_LIBS_OLD})

set(OZZ_TARGETS
  ozz_base
  ozz_animation
  ozz_animation_offline
  ozz_geometry
  ozz_options
  ozz_animation_tools
  json
)
set(OZZ_EXE_TARGETS
  dump2ozz
  gltf2ozz
)

if(TARGET ozz_animation_fbx)
  list(APPEND OZZ_TARGETS ozz_animation_fbx)
  list(APPEND OZZ_EXE_TARGETS fbx2ozz)
endif()

set_target_properties(${OZZ_TARGETS} ${OZZ_EXE_TARGETS}
  PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

set(OZZ_TARGETS_EXPORT_NAME "ozzTargets")
set(OZZ_INSTALL_DIR "${CMAKE_INSTALL_LIBDIR}/cmake/ozz")
install(TARGETS ${OZZ_TARGETS}
   EXPORT ${OZZ_TARGETS_EXPORT_NAME}
)
install(EXPORT ${OZZ_TARGETS_EXPORT_NAME}
  NAMESPACE ozz
  DESTINATION ${OZZ_INSTALL_DIR}
)