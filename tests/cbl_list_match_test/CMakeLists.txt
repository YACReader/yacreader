qt_add_executable(cbl_list_match_test main.cpp)
yacreader_apply_build_options(cbl_list_match_test)
target_link_libraries(cbl_list_match_test PRIVATE Qt6::Core Qt6::Gui Qt6::Sql Qt6::Test cbl)
add_test(NAME cbl_list_match_test COMMAND cbl_list_match_test)
