qt_add_executable(comic_files_manager_test
    main.cpp
)
yacreader_apply_build_options(comic_files_manager_test)
target_link_libraries(comic_files_manager_test PRIVATE
    Qt6::Core
    Qt6::Test
    library_common
)

add_test(NAME comic_files_manager_test COMMAND comic_files_manager_test)
