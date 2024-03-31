set(VCPKG_BUILD_TYPE release)

vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO nyyakko/LibError
    REF f485e6374ff26f69a28a56e6f43970ce949fecb3
    HEAD_REF master
    SHA512 3db02e805aa04a497aab309998ede9e9c09e55a5a621e23875384e7bee91330a77ff1f2faa4cbc66cee7ce398e6989196805c9f52f4dcf353ae77040cfb49b9a
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
    OPTIONS
        -DENABLE_TESTING=FALSE
)
vcpkg_cmake_install()
vcpkg_fixup_pkgconfig()

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")

