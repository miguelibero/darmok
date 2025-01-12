include(FetchContent)
FetchContent_Declare(ozz-animation
  GIT_REPOSITORY https://github.com/guillaumeblanc/ozz-animation.git
  GIT_TAG master
)

# FBX SDK can be downloaded from https://aps.autodesk.com/developer/overview/fbx-sdk
set(ozz_build_tools OFF CACHE BOOL "")
set(ozz_build_samples OFF CACHE BOOL "")
set(ozz_build_howtos OFF CACHE BOOL "")
set(ozz_build_tests OFF CACHE BOOL "")
set(ozz_build_msvc_rt_dll ON CACHE BOOL "")
set(FBX_MSVC_RT_DLL ON CACHE BOOL "")

FetchContent_MakeAvailable(ozz-animation)

set(OZZ_TARGETS
  ozz_base
  ozz_animation
  ozz_animation_offline
  ozz_geometry
)

set_target_properties(${OZZ_TARGETS}
  PROPERTIES MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL"
)

# error in include/ozz/animation/runtime/blending_job.h:107
if (CMAKE_CXX_COMPILER_FRONTEND_VARIANT STREQUAL "GNU")
target_compile_options(ozz_animation PUBLIC -Wno-ignored-attributes)
endif()

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
