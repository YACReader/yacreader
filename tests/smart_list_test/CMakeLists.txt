qt_add_executable(smart_list_test main.cpp)
yacreader_apply_build_options(smart_list_test)
target_link_libraries(smart_list_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME smart_list_test COMMAND smart_list_test)
