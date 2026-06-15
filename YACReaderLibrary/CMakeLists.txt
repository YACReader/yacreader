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
yacreader_apply_build_options(library_common)
target_link_libraries(library_common PUBLIC
    Qt6::Core
    Qt6::Core5Compat
    Qt6::Network
    Qt6::Sql
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
yacreader_apply_build_options(db_helper)
target_link_libraries(db_helper PUBLIC
    Qt6::Core
    Qt6::Sql
    common_all
    QsLog
)

# When BUILD_SERVER_STANDALONE=ON, only library_common and db_helper are built.
# The full YACReaderLibrary GUI app requires Qt6::Widgets and other full-build components.
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
    organize_files_dialog.h
    organize_files_dialog.cpp
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
    ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.h
    ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.cpp
)
yacreader_apply_build_options(YACReaderLibrary)

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
set(yacreaderlibrary_image_files
    ${PROJECT_SOURCE_DIR}/images/shortcuts/accept_shortcut.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/clear_shortcut.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/downArrow.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/nextPage.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/previousPage.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/radioChecked.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/radioUnchecked.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/rowDown.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/rowUp.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/upArrow.svg
    ${PROJECT_SOURCE_DIR}/images/comic_vine/checkBoxTick.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/asignNumber.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/big_size_grid_zoom.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/editComic.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/getInfo.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/hideComicFlow.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/openInYACReader.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/selectAll.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/setReadButton.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/setUnread.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/showMarks.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/small_size_grid_zoom.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/trash.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/show_comic_info.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/setManga.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/setNormal.svg
    ${PROJECT_SOURCE_DIR}/images/comics_view_toolbar/showRecentIndicator.svg
    ${PROJECT_SOURCE_DIR}/images/defaultCover.png
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/edit.svg
    ${PROJECT_SOURCE_DIR}/images/empty_container/empty_current_readings.svg
    ${PROJECT_SOURCE_DIR}/images/empty_container/empty_favorites.svg
    ${PROJECT_SOURCE_DIR}/images/empty_container/empty_folder.svg
    ${PROJECT_SOURCE_DIR}/images/empty_container/empty_label.svg
    ${PROJECT_SOURCE_DIR}/images/empty_container/empty_reading_list.svg
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/exportComicsInfo.svg
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/exportLibrary.svg
    ${PROJECT_SOURCE_DIR}/images/find_folder.svg
    ${PROJECT_SOURCE_DIR}/images/flow1.png
    ${PROJECT_SOURCE_DIR}/images/flow2.png
    ${PROJECT_SOURCE_DIR}/images/flow3.png
    ${PROJECT_SOURCE_DIR}/images/flow4.png
    ${PROJECT_SOURCE_DIR}/images/flow5.png
    ${PROJECT_SOURCE_DIR}/images/glowLine.svg
    ${PROJECT_SOURCE_DIR}/images/metadata_dialog/loadCustomCover.svg
    ${PROJECT_SOURCE_DIR}/images/import/coversToggle.svg
    ${PROJECT_SOURCE_DIR}/images/icon-new.svg
    ${PROJECT_SOURCE_DIR}/images/iconLibrary.png
    ${PROJECT_SOURCE_DIR}/images/import/importBottomCoversDecoration.svg
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/importComicsInfo.svg
    ${PROJECT_SOURCE_DIR}/images/import/importingIcon.svg
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/importLibrary.svg
    ${PROJECT_SOURCE_DIR}/images/import/importTopCoversDecoration.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/back.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/forward.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/settings.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/server.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/help.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/fullscreen.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/flow.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/grid.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/info.svg
    ${PROJECT_SOURCE_DIR}/images/main_toolbar/divider.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/editIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/exportComicsInfoIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/exportLibraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/importComicsInfoIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/importLibraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/removeLibraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/updateLibraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/open_containing_folder.svg
    ${PROJECT_SOURCE_DIR}/images/menus_icons/update_current_folder.svg
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/new.svg
    ${PROJECT_SOURCE_DIR}/images/metadata_dialog/nextCoverPage.svg
    ${PROJECT_SOURCE_DIR}/images/noLibrariesIcon.svg
    ${PROJECT_SOURCE_DIR}/images/noLibrariesLine.svg
    ${PROJECT_SOURCE_DIR}/images/notCover.png
    ${PROJECT_SOURCE_DIR}/images/library_dialogs/openLibrary.svg
    ${PROJECT_SOURCE_DIR}/images/metadata_dialog/previousCoverPage.svg
    ${PROJECT_SOURCE_DIR}/images/readingRibbon.svg
    ${PROJECT_SOURCE_DIR}/images/readRibbon.svg
    ${PROJECT_SOURCE_DIR}/images/metadata_dialog/resetCover.svg
    ${PROJECT_SOURCE_DIR}/images/search_result.svg
    ${PROJECT_SOURCE_DIR}/images/serverConfigBackground.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_comics.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_folders.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_general.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_libraries.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_mglass.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_page.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_reading.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_visualization.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/libraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/setRoot.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/expand.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/colapse.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/newLibraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/openLibraryIcon.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/addNew_sidebar.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/delete_sidebar.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/addLabelIcon.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/renameListIcon.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/branch-closed.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/branch-open.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/folder.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/folder_finished.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/folder_read_overlay.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/libraryIconSelected.svg
    ${PROJECT_SOURCE_DIR}/images/sidebar/libraryOptions.svg
    ${PROJECT_SOURCE_DIR}/images/import/updatingIcon.svg
    ${PROJECT_SOURCE_DIR}/images/custom_dialog/custom_close_button.svg
    ${PROJECT_SOURCE_DIR}/images/whats_new/whatsnew_header.svg
    ${PROJECT_SOURCE_DIR}/images/lists/default_0.svg
    ${PROJECT_SOURCE_DIR}/images/lists/default_1.svg
    ${PROJECT_SOURCE_DIR}/images/lists/default_2.svg
    ${PROJECT_SOURCE_DIR}/images/lists/label_template.svg
    ${PROJECT_SOURCE_DIR}/images/lists/list.svg
    ${PROJECT_SOURCE_DIR}/images/clearSearchNew.svg
    ${PROJECT_SOURCE_DIR}/images/iconSearchNew.svg
)
set(yacreaderlibrary_file_files
    ${PROJECT_SOURCE_DIR}/files/about.html
    ${PROJECT_SOURCE_DIR}/files/helpYACReaderLibrary.html
)
set(yacreaderlibrary_file_files_es
    ${PROJECT_SOURCE_DIR}/files/about_es_ES.html
    ${PROJECT_SOURCE_DIR}/files/helpYACReaderLibrary_es_ES.html
)
set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/files/about_es_ES.html
    PROPERTIES QT_RESOURCE_ALIAS "about.html"
)
set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/files/helpYACReaderLibrary_es_ES.html
    PROPERTIES QT_RESOURCE_ALIAS "helpYACReaderLibrary.html"
)
set(yacreaderlibrary_qml_files
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/GridComicsView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/FolderContentView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/FlowView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoTick.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoFavorites.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoRating.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoComicsView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/tick.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/reading.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/star.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/page.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-indicator.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-shadow.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-top-shadow.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/ComicInfoView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-favorites.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-rating.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-tag.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/info-tick.svg
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/prerendered_cover_shadow.png
)
set(yacreaderlibrary_qml_translation_files
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/GridComicsView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/FolderContentView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/FlowView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoTick.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoFavorites.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoRating.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/InfoComicsView.qml
    ${CMAKE_CURRENT_SOURCE_DIR}/qml/ComicInfoView.qml
)
set(yacreaderlibrary_theme_files
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_classic.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_light.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_light1.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_light2.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_light3.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_light4.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_light5.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_dark.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_dark1.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_dark2.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_dark3.json
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/builtin_dark4.json
)
set(yacreaderlibrary_common_theme_image_files
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-system.svg
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-light.svg
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-dark.svg
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-custom.svg
)

qt_add_resources(YACReaderLibrary "yacreaderlibrary_images"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}"
    FILES
        ${yacreaderlibrary_image_files}
)
qt_add_resources(YACReaderLibrary "yacreaderlibrary_files"
    PREFIX "/files"
    BASE "${PROJECT_SOURCE_DIR}/files"
    FILES
        ${yacreaderlibrary_file_files}
)
qt_add_resources(YACReaderLibrary "yacreaderlibrary_files_es"
    PREFIX "/files"
    LANG "es_ES"
    BASE "${PROJECT_SOURCE_DIR}/files"
    FILES
        ${yacreaderlibrary_file_files_es}
)
qt_add_resources(YACReaderLibrary "yacreaderlibrary_changelog"
    PREFIX "/files"
    BASE "${PROJECT_SOURCE_DIR}"
    FILES
        ${PROJECT_SOURCE_DIR}/CHANGELOG.md
)
qt_add_resources(YACReaderLibrary "yacreaderlibrary_qml"
    PREFIX "/"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}"
    FILES
        ${yacreaderlibrary_qml_files}
)
qt_add_resources(YACReaderLibrary "yacreaderlibrary_themes"
    PREFIX "/themes"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/themes"
    FILES
        ${yacreaderlibrary_theme_files}
)
qt_add_resources(YACReaderLibrary "yacreaderlibrary_common_theme_images"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}"
    FILES
        ${yacreaderlibrary_common_theme_image_files}
)
if(WIN32 OR (UNIX AND NOT APPLE))
    set(yacreaderlibrary_windows_resource_files
        ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/close.svg
        ${CMAKE_CURRENT_SOURCE_DIR}/icon.ico
    )
    set_source_files_properties(
        ${CMAKE_CURRENT_SOURCE_DIR}/icon.ico
        PROPERTIES QT_RESOURCE_ALIAS "icon.ico"
    )
    qt_add_resources(YACReaderLibrary "yacreaderlibrary_windows_resources"
        PREFIX "/"
        BASE "${PROJECT_SOURCE_DIR}"
        FILES
            ${yacreaderlibrary_windows_resource_files}
    )
endif()
if(APPLE)
    qt_add_resources(YACReaderLibrary "yacreaderlibrary_macos_resources"
        PREFIX "/"
        BASE "${CMAKE_CURRENT_SOURCE_DIR}"
        FILES
            ${CMAKE_CURRENT_SOURCE_DIR}/macostrayicon.svg
    )
endif()

# Translations
qt_add_translations(YACReaderLibrary
    SOURCE_TARGETS
        YACReaderLibrary
        db_helper
        comic_backend
        common_gui
        custom_widgets_library
        shortcuts_library
        comic_vine
    # Keep extraction scoped to targets used by this app and add the QML files
    # directly so qsTr() strings in QML are collected too.
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
        yacreaderlibrary_ko.ts
        yacreaderlibrary_source.ts
        yacreaderlibrary_en.ts
    SOURCES
        ${yacreaderlibrary_qml_translation_files}
)

target_link_libraries(YACReaderLibrary PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Network
    Qt6::Widgets
    Qt6::Sql
    Qt6::Svg
    Qt6::Quick
    Qt6::Qml
    Qt6::QuickWidgets
    Qt6::QuickControls2
    Qt6::Core5Compat
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

# Copy server web files to build output for development (Linux uses DATADIR at runtime)
if(WIN32 OR APPLE)
    add_custom_command(TARGET YACReaderLibrary POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${PROJECT_SOURCE_DIR}/release/server/docroot/images/webui"
            "$<TARGET_FILE_DIR:YACReaderLibrary>/server/docroot/images/webui"
        COMMENT "Copying webui images to build output"
    )
endif()

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
    install(FILES
        "${PROJECT_SOURCE_DIR}/YACReaderLibrary.1"
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    install(FILES
        "${PROJECT_SOURCE_DIR}/YACReaderLibrary.desktop"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES
        "${PROJECT_SOURCE_DIR}/YACReaderLibrary.svg"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)

    # Server web files (for bundled server mode)
    if(NOT BUILD_SERVER_STANDALONE)
        install(DIRECTORY "${PROJECT_SOURCE_DIR}/release/server"
            DESTINATION ${CMAKE_INSTALL_DATADIR}/yacreader)
    endif()
endif()

endif() # NOT BUILD_SERVER_STANDALONE (YACReaderLibrary executable)
