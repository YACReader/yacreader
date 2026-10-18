qt_add_executable(server_v2_lists_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server/yacreader_server_data_helper.cpp
)
yacreader_apply_build_options(server_v2_lists_test)
target_include_directories(server_v2_lists_test PRIVATE ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server)
target_link_libraries(server_v2_lists_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME server_v2_lists_test COMMAND server_v2_lists_test)
