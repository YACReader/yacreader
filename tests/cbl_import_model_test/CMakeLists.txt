qt_add_executable(cbl_import_model_test main.cpp)
yacreader_apply_build_options(cbl_import_model_test)
target_link_libraries(cbl_import_model_test PRIVATE Qt6::Core Qt6::Gui Qt6::Test cbl_gui)
add_test(NAME cbl_import_model_test COMMAND cbl_import_model_test)
