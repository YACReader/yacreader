# ComicBookDS integration test: opens synthetic .cbds books through the
# configured decompression backend, the reader and the library scanner

qt_add_executable(cbds_comic_test
    main.cpp
)
yacreader_apply_build_options(cbds_comic_test)
target_link_libraries(cbds_comic_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Test
    comic_backend
    library_common
)
if(WIN32)
    target_link_libraries(cbds_comic_test PRIVATE oleaut32 ole32)
endif()

# The 7zip backend loads its library from <executable dir>/utils at runtime
if(WIN32 AND DECOMPRESSION_BACKEND STREQUAL "7zip")
    if(CMAKE_SYSTEM_PROCESSOR MATCHES "ARM64|arm64|aarch64")
        set(_cbds_test_7z_arch arm64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(_cbds_test_7z_arch x64)
    else()
        set(_cbds_test_7z_arch x86)
    endif()
    add_custom_command(TARGET cbds_comic_test POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:cbds_comic_test>/utils"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${PROJECT_SOURCE_DIR}/dependencies/7zip/win/${_cbds_test_7z_arch}/7z.dll"
            "$<TARGET_FILE_DIR:cbds_comic_test>/utils/7z.dll"
    )
endif()

add_test(NAME cbds_comic_test COMMAND cbds_comic_test)
