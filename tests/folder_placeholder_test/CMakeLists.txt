qt_add_executable(folder_placeholder_test main.cpp)
yacreader_apply_build_options(folder_placeholder_test)
target_link_libraries(folder_placeholder_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME folder_placeholder_test COMMAND folder_placeholder_test)
