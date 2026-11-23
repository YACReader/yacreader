# IP config helper test

qt_add_executable(ip_config_helper_test main.cpp)
yacreader_apply_build_options(ip_config_helper_test)
target_link_libraries(ip_config_helper_test PRIVATE Qt6::Test library_common)
add_test(NAME ip_config_helper_test COMMAND ip_config_helper_test)
