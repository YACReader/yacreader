qt_add_executable(comic_model_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/comic_model.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/yacreader_comics_selection_helper.cpp
)
yacreader_apply_build_options(comic_model_test)
target_include_directories(comic_model_test PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db
)
target_link_libraries(comic_model_test PRIVATE Qt6::Core Qt6::Gui Qt6::Sql Qt6::Test db_helper library_common common_gui)
add_test(NAME comic_model_test COMMAND comic_model_test)
set_tests_properties(comic_model_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
