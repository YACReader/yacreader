qt_add_executable(cbl_matcher_test main.cpp)
yacreader_apply_build_options(cbl_matcher_test)
target_link_libraries(cbl_matcher_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper cbl)
add_test(NAME cbl_matcher_test COMMAND cbl_matcher_test)
