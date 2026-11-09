qt_add_executable(library_maintenance_test main.cpp)
target_sources(library_maintenance_test PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReaderLibraryServer/console_ui_library_creator.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibraryServer/console_ui_library_creator.h
)
target_include_directories(library_maintenance_test PRIVATE ${PROJECT_SOURCE_DIR}/YACReaderLibraryServer)
yacreader_apply_build_options(library_maintenance_test)
target_link_libraries(library_maintenance_test PRIVATE Qt6::Gui Qt6::Sql Qt6::Test db_helper library_common)
if(WIN32)
    target_link_libraries(library_maintenance_test PRIVATE advapi32)
endif()
add_test(NAME library_maintenance_test COMMAND library_maintenance_test)

qt_add_executable(library_maintenance_http_test http_test.cpp)
yacreader_apply_build_options(library_maintenance_http_test)
target_link_libraries(library_maintenance_http_test PRIVATE Qt6::Test server_test_support)
add_test(NAME library_maintenance_http_test COMMAND library_maintenance_http_test)
set_tests_properties(library_maintenance_http_test PROPERTIES TIMEOUT 45)

if(NOT BUILD_SERVER_STANDALONE)
    qt_add_executable(library_maintenance_dialog_test dialog_test.cpp)
    target_sources(library_maintenance_dialog_test PRIVATE
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/library_management_coordinator.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/create_library_dialog.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/add_library_dialog.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/export_library_dialog.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/import_library_dialog.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/folder_model.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/folder_item.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/folder_query_result_processor.cpp
        ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes/theme_factory.cpp
        ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.cpp
    )
    yacreader_apply_build_options(library_maintenance_dialog_test)
    target_include_directories(library_maintenance_dialog_test PRIVATE ${PROJECT_SOURCE_DIR}/YACReaderLibrary ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes)
    target_compile_definitions(library_maintenance_dialog_test PRIVATE YACREADER_LIBRARY)
    target_link_libraries(library_maintenance_dialog_test PRIVATE Qt6::Widgets Qt6::Test db_helper library_common common_gui concurrent_queue)
    add_test(NAME library_maintenance_dialog_test COMMAND library_maintenance_dialog_test)
    set_tests_properties(library_maintenance_dialog_test PROPERTIES ENVIRONMENT "QT_QPA_PLATFORM=offscreen" TIMEOUT 30)
    if(TARGET Qt6::QOffscreenIntegrationPlugin)
        set_property(TEST library_maintenance_dialog_test APPEND PROPERTY ENVIRONMENT
            "QT_QPA_PLATFORM_PLUGIN_PATH=$<TARGET_FILE_DIR:Qt6::QOffscreenIntegrationPlugin>")
    endif()
endif()
