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
set(FBX_MSVC_RT_DLL ON CACHE BOOL "")

FetchContent_MakeAvailable(ozz-animation)

set(OZZ_LIB_TARGETS
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
  list(APPEND OZZ_LIB_TARGETS ozz_animation_fbx)
  list(APPEND OZZ_EXE_TARGETS fbx2ozz)
endif()

set(OZZ_TARGETS ${OZZ_LIB_TARGETS})
list(APPEND OZZ_TARGETS ${OZZ_EXE_TARGETS})

set_target_properties(${OZZ_TARGETS}
  PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

include(GNUInstallDirs)

set(EXPORT_BASE_NAME "ozzAnimation")
set(CONFIG_INSTALL_DIR "${CMAKE_INSTALL_DATADIR}/darmok/cmake")

set(TARGETS_EXPORT "${EXPORT_BASE_NAME}Targets")
install(TARGETS ${OZZ_TARGETS}
    EXPORT ${TARGETS_EXPORT}
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
install(EXPORT ${TARGETS_EXPORT}
    NAMESPACE ozz::
    DESTINATION ${CONFIG_INSTALL_DIR}
)
