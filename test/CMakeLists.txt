cmake_minimum_required(VERSION 3.15)

set(TESTS_NAME darmok-tests)

project(
  ${TESTS_NAME}
  LANGUAGES CXX
)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(${TESTS_NAME}
  src/test.cpp
  src/optional_ref_test.cpp
  src/utf8char_test.cpp
  src/render_graph_test.cpp
)
target_link_libraries(${TESTS_NAME}
  PRIVATE Catch2::Catch2WithMain
  PUBLIC darmok
  )
include(CTest)
include(Catch)
catch_discover_tests(${TESTS_NAME})

# These tests need their own main
#add_executable(custom-main-tests test.cpp test-main.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)