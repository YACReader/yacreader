qt_add_executable(yacreader_libraries_test
    main.cpp
)
yacreader_apply_build_options(yacreader_libraries_test)
target_link_libraries(yacreader_libraries_test PRIVATE
    Qt6::Core
    Qt6::Test
    library_common
)

add_test(NAME yacreader_libraries_test COMMAND yacreader_libraries_test)
