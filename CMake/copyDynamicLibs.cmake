if(NOT BUILD_DIR)
  return()
endif()
file(GLOB_RECURSE PROJ_DEPS ${BUILD_DIR}/**/*.dll)
set(BUILD_DEP_DIRS ${BUILD_DIR})
set(VCPKG_DIR_SLASH ${BUILD_DIR}/vcpkg_installed/)
foreach(PROJ_DEP ${PROJ_DEPS})
  string(FIND "${PROJ_DEP}" "${VCPKG_DIR_SLASH}" POS)
  if (POS EQUAL 0)
    continue() # vcpkg manages dynamic libs by themselves
  endif()
  get_filename_component(PROJ_DEP_DIR ${PROJ_DEP} DIRECTORY)
  list(FIND BUILD_DEP_DIRS ${PROJ_DEP_DIR} INDEX)
  if (${INDEX} LESS 0)
    list(APPEND BUILD_DEP_DIRS ${PROJ_DEP_DIR})
  endif()
endforeach()

file(GET_RUNTIME_DEPENDENCIES
  RESOLVED_DEPENDENCIES_VAR DEPENDENCIES
  UNRESOLVED_DEPENDENCIES_VAR UNRESOLVED_DEPENDENCIES
  DIRECTORIES ${BUILD_DEP_DIRS}
  LIBRARIES ${TARGET}
  EXECUTABLES ${TARGET}
  MODULES ${TARGET}
)
get_filename_component(TARGET_DIR ${TARGET} DIRECTORY)
set(BUILD_DIR_SLASH "${BUILD_DIR}/")

foreach(DEP_PATH ${DEPENDENCIES})
  string(FIND "${DEP_PATH}" "${BUILD_DIR_SLASH}" POS)
  if (NOT POS EQUAL 0)
    continue()
  endif()
  get_filename_component(DEP_NAME ${DEP_PATH} NAME)
  set(TARGET_PATH "${TARGET_DIR}/${DEP_NAME}")
  file(COPY_FILE ${DEP_PATH} ${TARGET_PATH} ONLY_IF_DIFFERENT)
endforeach()