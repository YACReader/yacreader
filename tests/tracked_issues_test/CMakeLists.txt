qt_add_executable(tracked_issues_test main.cpp)
yacreader_apply_build_options(tracked_issues_test)
target_link_libraries(tracked_issues_test PRIVATE Qt6::Core Qt6::Gui Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME tracked_issues_test COMMAND tracked_issues_test)
