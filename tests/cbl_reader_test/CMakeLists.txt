qt_add_executable(cbl_reader_test main.cpp)
yacreader_apply_build_options(cbl_reader_test)
target_link_libraries(cbl_reader_test PRIVATE Qt6::Core Qt6::Test library_common)
add_test(NAME cbl_reader_test COMMAND cbl_reader_test)
