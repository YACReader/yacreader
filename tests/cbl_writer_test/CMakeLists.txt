qt_add_executable(cbl_writer_test main.cpp)
yacreader_apply_build_options(cbl_writer_test)
target_link_libraries(cbl_writer_test PRIVATE Qt6::Core Qt6::Test cbl)
add_test(NAME cbl_writer_test COMMAND cbl_writer_test)
