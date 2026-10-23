qt_add_executable(delayed_progress_dialog_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/delayed_progress_dialog.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/delayed_progress_dialog.h
)
yacreader_apply_build_options(delayed_progress_dialog_test)
target_include_directories(delayed_progress_dialog_test PRIVATE ${PROJECT_SOURCE_DIR}/YACReaderLibrary)
target_link_libraries(delayed_progress_dialog_test PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test)
add_test(NAME delayed_progress_dialog_test COMMAND delayed_progress_dialog_test)
set_tests_properties(delayed_progress_dialog_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
