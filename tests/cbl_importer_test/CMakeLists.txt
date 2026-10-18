qt_add_executable(cbl_importer_test main.cpp)
yacreader_apply_build_options(cbl_importer_test)
target_link_libraries(cbl_importer_test PRIVATE Qt6::Core Qt6::Sql Qt6::Test db_helper library_common)
add_test(NAME cbl_importer_test COMMAND cbl_importer_test)
