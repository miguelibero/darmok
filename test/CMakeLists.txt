cmake_minimum_required(VERSION 3.15)

project(
  darmok-tests
  LANGUAGES CXX
)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(tests src/test.cpp)
target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)

# These tests need their own main
#add_executable(custom-main-tests test.cpp test-main.cpp)
#target_link_libraries(custom-main-tests PRIVATE Catch2::Catch2)