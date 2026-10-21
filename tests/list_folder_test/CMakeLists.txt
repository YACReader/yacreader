qt_add_executable(list_folder_test main.cpp)
yacreader_apply_build_options(list_folder_test)
target_link_libraries(list_folder_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME list_folder_test COMMAND list_folder_test)
