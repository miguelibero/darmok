cmake_minimum_required(VERSION 3.15)

set(TESTS_NAME darmok-tests)

project(
  ${TESTS_NAME}
  LANGUAGES CXX
)

find_package(Catch2 3 REQUIRED)
# These tests can use the Catch2-provided main
add_executable(${TESTS_NAME}
  src/data_test.cpp
  src/optional_ref_test.cpp
  src/entity_filter_test.cpp
  src/shape_test.cpp
  src/scene_serialize_test.cpp
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


find_package(Protobuf CONFIG REQUIRED)
set(TESTS_PROTO_FILES
  scene_serialize_test.proto
)

set(TESTS_PROTO_DIR ${PROJECT_SOURCE_DIR}/assets/protobuf)
set(TESTS_PROTO_BINARY_DIR "${CMAKE_CURRENT_BINARY_DIR}/include/private/protobuf")
set(TESTS_PROTO_LIB_NAME ${TESTS_NAME}-proto-objects)

list(TRANSFORM TESTS_PROTO_FILES PREPEND "${TESTS_PROTO_DIR}/")
add_library(${TESTS_PROTO_LIB_NAME} OBJECT ${TESTS_PROTO_FILES})
target_link_libraries(${TESTS_PROTO_LIB_NAME} PRIVATE protobuf::libprotobuf)
target_include_directories(${TESTS_PROTO_LIB_NAME} PRIVATE "$<BUILD_INTERFACE:${PROTO_BINARY_DIR}>")
protobuf_generate(
    TARGET ${TESTS_PROTO_LIB_NAME}
    IMPORT_DIRS "${TESTS_PROTO_DIR}"
    PROTOC_OUT_DIR "${TESTS_PROTO_BINARY_DIR}")
target_link_libraries(${TESTS_NAME} PRIVATE ${TESTS_PROTO_LIB_NAME})
target_include_directories(
  ${TESTS_NAME}
  PRIVATE
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include/private>
)