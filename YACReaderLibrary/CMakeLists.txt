# YACReaderLibrary - Comic Library Manager

# --- library_common (shared with YACReaderLibraryServer) ---
add_library(library_common STATIC
    library_creator.h
    library_creator.cpp
    package_manager.h
    package_manager.cpp
    bundle_creator.h
    bundle_creator.cpp
    initial_comic_info_extractor.h
    initial_comic_info_extractor.cpp
    xml_info_parser.h
    xml_info_parser.cpp
    xml_info_library_scanner.h
    xml_info_library_scanner.cpp
    yacreader_local_server.h
    yacreader_local_server.cpp
    comics_remover.h
    comics_remover.cpp
    yacreader_libraries.h
    yacreader_libraries.cpp
    comic_files_manager.h
    comic_files_manager.cpp
    ip_config_helper.h
    ip_config_helper.cpp
    libraries_update_coordinator.h
    libraries_update_coordinator.cpp
)
target_include_directories(library_common PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/db
)
target_link_libraries(library_common PUBLIC
    Qt::Core
    Qt::Core5Compat
    Qt::Network
    Qt::Sql
    common_all
    comic_backend
    cbx_backend
    db_helper
    QsLog
)

# --- db_helper (database layer, shared with YACReaderLibraryServer) ---
add_library(db_helper STATIC
    db_helper.h
    db_helper.cpp
    db/data_base_management.h
    db/data_base_management.cpp
    db/reading_list.h
    db/reading_list.cpp
    db/query_lexer.h
    db/query_lexer.cpp
    db/query_parser.h
    db/query_parser.cpp
    db/search_query.h
    db/search_query.cpp
)
target_include_directories(db_helper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/db
)
target_link_libraries(db_helper PUBLIC
    Qt::Core
    Qt::Sql
    common_all
    QsLog
)

# When BUILD_SERVER_STANDALONE=ON, only library_common and db_helper are built.
# The full YACReaderLibrary GUI app requires Qt::Widgets and other full-build components.
if(NOT BUILD_SERVER_STANDALONE)

# --- YACReaderLibrary executable ---
qt_add_executable(YACReaderLibrary WIN32
    main.cpp
    library_window.h
    library_window.cpp
    library_window_actions.h
    library_window_actions.cpp
    create_library_dialog.h
    create_library_dialog.cpp
    add_library_dialog.h
    add_library_dialog.cpp
    rename_library_dialog.h
    rename_library_dialog.cpp
    properties_dialog.h
    properties_dialog.cpp
    options_dialog.h
    options_dialog.cpp
    export_library_dialog.h
    export_library_dialog.cpp
    import_library_dialog.h
    import_library_dialog.cpp
    export_comics_info_dialog.h
    export_comics_info_dialog.cpp
    import_comics_info_dialog.h
    import_comics_info_dialog.cpp
    server_config_dialog.h
    server_config_dialog.cpp
    comic_flow_widget.h
    comic_flow_widget.cpp
    no_libraries_widget.h
    no_libraries_widget.cpp
    import_widget.h
    import_widget.cpp
    trayicon_controller.h
    trayicon_controller.cpp
    yacreader_content_views_manager.h
    yacreader_content_views_manager.cpp
    yacreader_main_toolbar.h
    yacreader_main_toolbar.cpp
    comics_view.h
    comics_view.cpp
    comics_view_transition.h
    comics_view_transition.cpp
    classic_comics_view.h
    classic_comics_view.cpp
    grid_comics_view.h
    grid_comics_view.cpp
    no_search_results_widget.h
    no_search_results_widget.cpp
    folder_content_view.h
    folder_content_view.cpp
    recent_visibility_coordinator.h
    recent_visibility_coordinator.cpp
    library_comic_opener.h
    library_comic_opener.cpp
    db/comic_query_result_processor.h
    db/comic_query_result_processor.cpp
    db/folder_query_result_processor.h
    db/folder_query_result_processor.cpp
    db/folder_item.h
    db/folder_item.cpp
    db/folder_model.h
    db/folder_model.cpp
    db/comic_model.h
    db/comic_model.cpp
    db/comic_item.h
    db/comic_item.cpp
    db/reading_list_model.h
    db/reading_list_model.cpp
    db/reading_list_item.h
    db/reading_list_item.cpp
    yacreader_folders_view.h
    yacreader_folders_view.cpp
    yacreader_reading_lists_view.h
    yacreader_reading_lists_view.cpp
    add_label_dialog.h
    add_label_dialog.cpp
    yacreader_history_controller.h
    yacreader_history_controller.cpp
    yacreader_navigation_controller.h
    yacreader_navigation_controller.cpp
    empty_label_widget.h
    empty_label_widget.cpp
    empty_folder_widget.h
    empty_folder_widget.cpp
    empty_container_info.h
    empty_container_info.cpp
    empty_special_list.h
    empty_special_list.cpp
    empty_reading_list_widget.h
    empty_reading_list_widget.cpp
    info_comics_view.h
    info_comics_view.cpp
    yacreader_comics_selection_helper.h
    yacreader_comics_selection_helper.cpp
    yacreader_comic_info_helper.h
    yacreader_comic_info_helper.cpp
    current_comic_view_helper.h
    current_comic_view_helper.cpp
    # App-specific themes + shared theme_manager (depends on app theme.h)
    themes/theme.h
    themes/theme_factory.h
    themes/theme_factory.cpp
    ${CMAKE_SOURCE_DIR}/common/themes/theme_manager.h
    ${CMAKE_SOURCE_DIR}/common/themes/theme_manager.cpp
)

# macOS-specific sources
if(APPLE)
    target_sources(YACReaderLibrary PRIVATE
        trayhandler.h
        trayhandler.mm
    )
endif()

target_include_directories(YACReaderLibrary PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/db
    ${CMAKE_CURRENT_SOURCE_DIR}/themes
)

target_compile_definitions(YACReaderLibrary PRIVATE
    SERVER_RELEASE
    YACREADER_LIBRARY
)

# Resources
qt_add_resources(yacreaderlibrary_images_rcc "${CMAKE_CURRENT_SOURCE_DIR}/images.qrc")
qt_add_resources(yacreaderlibrary_files_rcc "${CMAKE_CURRENT_SOURCE_DIR}/files.qrc")
qt_add_resources(yacreaderlibrary_qml_rcc "${CMAKE_CURRENT_SOURCE_DIR}/qml.qrc")
target_sources(YACReaderLibrary PRIVATE
    ${yacreaderlibrary_images_rcc}
    ${yacreaderlibrary_files_rcc}
    ${yacreaderlibrary_qml_rcc}
)
if(WIN32 OR (UNIX AND NOT APPLE))
    qt_add_resources(yacreaderlibrary_images_win_rcc "${CMAKE_CURRENT_SOURCE_DIR}/images_win.qrc")
    target_sources(YACReaderLibrary PRIVATE ${yacreaderlibrary_images_win_rcc})
endif()
if(APPLE)
    qt_add_resources(yacreaderlibrary_images_osx_rcc "${CMAKE_CURRENT_SOURCE_DIR}/images_osx.qrc")
    target_sources(YACReaderLibrary PRIVATE ${yacreaderlibrary_images_osx_rcc})
endif()

# Translations
qt_add_translations(YACReaderLibrary
    TS_FILES
        yacreaderlibrary_es.ts
        yacreaderlibrary_ru.ts
        yacreaderlibrary_pt.ts
        yacreaderlibrary_fr.ts
        yacreaderlibrary_nl.ts
        yacreaderlibrary_tr.ts
        yacreaderlibrary_de.ts
        yacreaderlibrary_zh_CN.ts
        yacreaderlibrary_zh_TW.ts
        yacreaderlibrary_zh_HK.ts
        yacreaderlibrary_it.ts
        yacreaderlibrary_en.ts
)

target_link_libraries(YACReaderLibrary PRIVATE
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::Network
    Qt::Widgets
    Qt::Sql
    Qt::Svg
    Qt::Quick
    Qt::Qml
    Qt::QuickWidgets
    Qt::QuickControls2
    Qt::Core5Compat
    library_common
    db_helper
    comic_backend
    common_gui
    rhi_flow_library
    custom_widgets_library
    shortcuts_library
    server
    comic_vine
    cbx_backend
    concurrent_queue
    worker
    QsLog
    QrCode
    KDSignalThrottler
)

# Platform-specific
if(WIN32)
    target_sources(YACReaderLibrary PRIVATE icon.rc)
    target_link_libraries(YACReaderLibrary PRIVATE oleaut32 ole32 shell32 user32)
endif()

if(APPLE)
    target_sources(YACReaderLibrary PRIVATE YACReaderLibrary.icns)
    set_source_files_properties(YACReaderLibrary.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(YACReaderLibrary PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yacreader.YACReaderLibrary"
        MACOSX_BUNDLE_BUNDLE_NAME "YACReaderLibrary"
    )
    target_link_libraries(YACReaderLibrary PRIVATE
        "-framework Foundation"
        "-framework ApplicationServices"
        "-framework AppKit"
    )
endif()

# Linux install
if(UNIX AND NOT APPLE)
    target_compile_definitions(YACReaderLibrary PRIVATE
        "LIBDIR=\"${CMAKE_INSTALL_FULL_LIBDIR}\""
        "DATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}\""
        "BINDIR=\"${CMAKE_INSTALL_FULL_BINDIR}\"")
    install(TARGETS YACReaderLibrary RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Server web files (for bundled server mode)
    if(NOT BUILD_SERVER_STANDALONE)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/release/server
            DESTINATION ${CMAKE_INSTALL_DATADIR}/yacreader)
    endif()
endif()

endif() # NOT BUILD_SERVER_STANDALONE (YACReaderLibrary executable)
