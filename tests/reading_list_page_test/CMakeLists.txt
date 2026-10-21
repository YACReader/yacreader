qt_add_executable(reading_list_page_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/reading_list_page.cpp
)
yacreader_apply_build_options(reading_list_page_test)
target_include_directories(reading_list_page_test PRIVATE ${PROJECT_SOURCE_DIR}/YACReaderLibrary)
target_link_libraries(reading_list_page_test PRIVATE Qt6::Core Qt6::Gui Qt6::Test)
add_test(NAME reading_list_page_test COMMAND reading_list_page_test)
set_tests_properties(reading_list_page_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
