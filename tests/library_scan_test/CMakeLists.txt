qt_add_executable(library_scan_test
    main.cpp
)
yacreader_apply_build_options(library_scan_test)
target_include_directories(library_scan_test PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary
)
target_link_libraries(library_scan_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Sql
    Qt6::Test
    db_helper
    library_common
)

add_test(NAME library_scan_test COMMAND library_scan_test)
