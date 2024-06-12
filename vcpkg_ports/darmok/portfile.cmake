vcpkg_check_linkage(ONLY_STATIC_LIBRARY)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO miguelibero/darmok
    REF "v${VERSION}"
    SHA512 8eb0f5a5b4ed693bcc8ad6d69e21ee430eda1bfd731dd613bc67b351d70e04bd2a136add704c801dff880997de964791925139e48c19874844c5de35a543b732
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