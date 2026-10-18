qt_add_executable(missing_comics_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server/yacreader_server_data_helper.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server/missing_comic_server_helper.cpp
)
qt_add_resources(missing_comics_test "missing_comic_cover"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}"
    FILES "${PROJECT_SOURCE_DIR}/images/defaultCover.png"
)
yacreader_apply_build_options(missing_comics_test)
target_include_directories(missing_comics_test PRIVATE ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server)
target_link_libraries(missing_comics_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME missing_comics_test COMMAND missing_comics_test)
