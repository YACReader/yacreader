# ComicBookDS page index test

qt_add_executable(cbds_page_index_test
    main.cpp
)
yacreader_apply_build_options(cbds_page_index_test)
target_link_libraries(cbds_page_index_test PRIVATE
    Qt6::Core
    Qt6::Test
    cbds_page_index
)
add_test(NAME cbds_page_index_test COMMAND cbds_page_index_test)
