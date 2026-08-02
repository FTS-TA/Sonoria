vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO FTS-TA/Sonoria
    REF "v${VERSION}"
    SHA512 00000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000
    HEAD_REF master
)

# Dependencies are handled by vcpkg's manifest mode
# The library will be built using CMake

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DBUILD_SHARED_LIBS=ON
        -DBUILD_EXAMPLES=OFF
        -DBUILD_PYTHON=OFF
        -DBUILD_VAMP=OFF
        -DBUILD_TESTS=OFF
    OPTIONS_RELEASE
        -DCMAKE_BUILD_TYPE=Release
    OPTIONS_DEBUG
        -DCMAKE_BUILD_TYPE=Debug
)

vcpkg_cmake_install()

# Install headers
file(INSTALL "${SOURCE_PATH}/src/sonoria/" DESTINATION "${CURRENT_PACKAGES_DIR}/include/sonoria" FILES_MATCHING PATTERN "*.h")
file(INSTALL "${SOURCE_PATH}/src/algorithms/" DESTINATION "${CURRENT_PACKAGES_DIR}/include/sonoria/algorithms" FILES_MATCHING PATTERN "*.h")

# Move license files
file(INSTALL "${SOURCE_PATH}/COPYING.txt" DESTINATION "${CURRENT_PACKAGES_DIR}/share/${PORT}" RENAME copyright)

vcpkg_copy_pdbs()
