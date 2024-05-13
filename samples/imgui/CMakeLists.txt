project(
  darmok::samples::empty
  LANGUAGES CXX
  DESCRIPTION "empty example darmok project"
)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

set(DARMOK_BUILD_IMGUI ON CACHE BOOL "")

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)
target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok::darmok)