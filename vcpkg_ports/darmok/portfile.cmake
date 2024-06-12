vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miguelibero/darmok
    REF "v${VERSION}"
    SHA512 e3e78a96db576bf5fc473f130d90a5557b9e04c0adff2d92532e521a4309d0313bf9908351e010346d6f4079f334a2acf0dc28851f87c0d6caf9004dc7f98727
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        DARMOK_BUILD_SAMPLES=OFF
        DARMOK_BUILD_TESTS=OFF
        DARMOK_BUILD_LUA=ON
        DARMOK_BUILD_LUA_EXE=ON
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