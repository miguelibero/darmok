project(
  darmok::samples::pbr
  LANGUAGES CXX
  DESCRIPTION "physically based rendering example darmok project"
)

add_executable(
  ${PROJECT_NAME}
  src/main.cpp
)

target_compile_definitions(
  ${PROJECT_NAME}
  PRIVATE "-DDARMOK_IMPLEMENT_MAIN=1"
)

target_link_libraries(${PROJECT_NAME} LINK_PUBLIC darmok::darmok)


