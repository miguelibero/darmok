project(
  darmok::samples::assimp
  LANGUAGES CXX
  DESCRIPTION "assimp example darmok project"
)

set(DARMOK_BUILD_ASSIMP ON CACHE BOOL "")

darmok_process_assets()

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
  ${ASSETS}
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok::darmok)


