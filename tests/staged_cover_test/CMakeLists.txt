qt_add_executable(staged_cover_test main.cpp)
yacreader_apply_build_options(staged_cover_test)
target_link_libraries(staged_cover_test PRIVATE Qt6::Gui Qt6::Test library_common)
add_test(NAME staged_cover_test COMMAND staged_cover_test)
