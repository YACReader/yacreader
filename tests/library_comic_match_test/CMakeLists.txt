qt_add_executable(library_comic_match_test main.cpp)
yacreader_apply_build_options(library_comic_match_test)
target_link_libraries(library_comic_match_test PRIVATE Qt6::Core Qt6::Gui Qt6::Sql Qt6::Test library_common)
add_test(NAME library_comic_match_test COMMAND library_comic_match_test)
