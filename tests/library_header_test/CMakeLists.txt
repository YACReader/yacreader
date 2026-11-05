qt_add_executable(library_header_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/header_model.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/library_header.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/view_selector.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/window_frame.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes/theme_factory.cpp
    ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.cpp
    ${PROJECT_SOURCE_DIR}/custom_widgets/yacreader_search_line_edit.cpp
)
if(WIN32)
    target_sources(library_header_test PRIVATE
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/caption_buttons.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/window_frame_windows.cpp
    )
    target_link_libraries(library_header_test PRIVATE dwmapi)
endif()
if(APPLE)
    set(library_header_test_macos_frame ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header/window_frame_macos.mm)
    target_sources(library_header_test PRIVATE ${library_header_test_macos_frame})
    set_source_files_properties(${library_header_test_macos_frame} TARGET_DIRECTORY library_header_test PROPERTIES COMPILE_OPTIONS "-fobjc-arc")
    target_link_libraries(library_header_test PRIVATE "-framework AppKit")
endif()
yacreader_apply_build_options(library_header_test)
target_include_directories(library_header_test PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_header
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes
    ${PROJECT_SOURCE_DIR}/custom_widgets
)
target_compile_definitions(library_header_test PRIVATE YACREADER_LIBRARY)
target_link_libraries(library_header_test PRIVATE Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Test common_gui)
add_test(NAME library_header_test COMMAND library_header_test)
set_tests_properties(library_header_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
