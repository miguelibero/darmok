
vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miguelibero/darmok
    REF "v${VERSION}"
    SHA512 9e9ba7a4e964e8b6f2405c833210d0c02376abe5ad530665891e60cf12e2fae75108127485f5d792f0ab88343c0a834375c2778cb690292b02128c304dec8021
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