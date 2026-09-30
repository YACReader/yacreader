qt_add_executable(library_item_actions_test
    main.cpp
)
yacreader_apply_build_options(library_item_actions_test)
target_link_libraries(library_item_actions_test PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Test
    db_helper
    library_common
)

add_test(NAME library_item_actions_test COMMAND library_item_actions_test)
