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

FetchContent_MakeAvailable(ozz-animation)