vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miguelibero/darmok
    REF "v${VERSION}"
    SHA512 388218e1e56bf5823b88d2c07196f61c4e6045bf858ca242f379293b79e10f94c58c4c4e7f8eef377ccd5818a608ca204539bf83ec2b80e50f28c975255c9708
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        DARMOK_BUILD_SAMPLES=OFF
        DARMOK_BUILD_TESTS=OFF
        DARMOK_BUILD_SCRIPTING=ON
        DARMOK_BUILD_SCRIPTING_EXE=ON
        DARMOK_BUILD_IMGUI=ON
        DARMOK_BUILD_RMLUI=ON
        DARMOK_BUILD_ASSIMP=ON
        DARMOK_BUILD_OZZ=ON
        DARMOK_BUILD_JOLT=ON
)

vcpkg_cmake_install()
vcpkg_cmake_config_fixup(PACKAGE_NAME "darmok")

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")
file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/share")

file(INSTALL "${SOURCE_PATH}/LICENSE" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)
configure_file("${CMAKE_CURRENT_LIST_DIR}/usage.txt" "${CURRENT_PACKAGES_DIR}/share/${PORT}/usage" COPYONLY)