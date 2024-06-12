
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miguelibero/darmok
    REF "v${VERSION}"
    SHA512 6100670b7fc7fda1e61d51155668a1256267ef36ea73cb5849a1a137e81e3c9a951ab8dbe2fd547af2736c467627f3aedbd30d7e11f4e57992f6761caeb3e494
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