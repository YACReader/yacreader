qt_add_executable(folder_model_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/folder_model.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/folder_item.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/folder_query_result_processor.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes/theme_factory.cpp
    ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.cpp
)
yacreader_apply_build_options(folder_model_test)
target_include_directories(folder_model_test PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes
)
target_compile_definitions(folder_model_test PRIVATE YACREADER_LIBRARY)
target_link_libraries(folder_model_test PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Test db_helper library_common common_gui concurrent_queue)
add_test(NAME folder_model_test COMMAND folder_model_test)
set_tests_properties(folder_model_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
