# YACReader - Comic Viewer

qt_add_executable(YACReader WIN32
    configuration.h
    configuration.cpp
    goto_dialog.h
    goto_dialog.cpp
    magnifying_glass.h
    magnifying_glass.cpp
    main_window_viewer.h
    main_window_viewer.cpp
    continuous_page_widget.h
    continuous_page_widget.cpp
    continuous_view_model.h
    continuous_view_model.cpp
    mouse_handler.h
    mouse_handler.cpp
    viewer.h
    viewer.cpp
    options_dialog.h
    options_dialog.cpp
    bookmarks_dialog.h
    bookmarks_dialog.cpp
    render.h
    render.cpp
    translator.h
    translator.cpp
    goto_flow_widget.h
    goto_flow_widget.cpp
    page_label_widget.h
    page_label_widget.cpp
    goto_flow_toolbar.h
    goto_flow_toolbar.cpp
    width_slider.h
    width_slider.cpp
    notifications_label_widget.h
    notifications_label_widget.cpp
    yacreader_local_client.h
    yacreader_local_client.cpp
    main.cpp
    # App-specific themes + shared theme_manager (depends on app theme.h)
    themes/theme.h
    themes/theme_factory.h
    themes/theme_factory.cpp
    ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.h
    ${PROJECT_SOURCE_DIR}/common/themes/theme_manager.cpp
)

target_include_directories(YACReader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/themes
)

target_compile_definitions(YACReader PRIVATE YACREADER)

# Resources
set(yacreader_image_files
    ${PROJECT_SOURCE_DIR}/images/icon-new.svg
    ${PROJECT_SOURCE_DIR}/images/goto.svg
    ${PROJECT_SOURCE_DIR}/images/find_folder.svg
    ${PROJECT_SOURCE_DIR}/images/flow1.png
    ${PROJECT_SOURCE_DIR}/images/flow2.png
    ${PROJECT_SOURCE_DIR}/images/flow3.png
    ${PROJECT_SOURCE_DIR}/images/flow4.png
    ${PROJECT_SOURCE_DIR}/images/flow5.png
    ${PROJECT_SOURCE_DIR}/images/notCover.png
    ${PROJECT_SOURCE_DIR}/images/close.svg
    ${PROJECT_SOURCE_DIR}/images/centerFlow.svg
    ${PROJECT_SOURCE_DIR}/images/gotoFlow.svg
    ${PROJECT_SOURCE_DIR}/images/defaultCover.png
    ${PROJECT_SOURCE_DIR}/images/translator/close.svg
    ${PROJECT_SOURCE_DIR}/images/translator/dropDownArrow.svg
    ${PROJECT_SOURCE_DIR}/images/translator/fromTo.svg
    ${PROJECT_SOURCE_DIR}/images/translator/speaker.svg
    ${PROJECT_SOURCE_DIR}/images/translator/translatorSearch.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/clear_shortcut.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/accept_shortcut.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_comics.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_folders.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_general.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_libraries.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_mglass.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_page.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_reading.svg
    ${PROJECT_SOURCE_DIR}/images/shortcuts/shortcuts_group_visualization.svg
    ${PROJECT_SOURCE_DIR}/images/custom_dialog/custom_close_button.svg
    ${PROJECT_SOURCE_DIR}/images/whats_new/whatsnew_header.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/bookmark.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/close.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/doublePage.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/doubleMangaPage.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/fitToPage.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/flow.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/full.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/goto.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/help.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/info.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/magnifyingGlass.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/next.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/open.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/openFolder.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/openNext.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/openPrevious.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/options.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/previous.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/rotateL.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/rotateR.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/save.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/shortcuts.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/showBookmarks.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/toHeight.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/toContinuousScroll.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/toWidth.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/translator.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/zoom.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/bookmark_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/close_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/doublePage_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/doubleMangaPage_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/fitToPage_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/flow_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/full_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/goto_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/help_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/info_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/magnifyingGlass_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/next_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/open_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/openFolder_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/openNext_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/openPrevious_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/options_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/previous_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/rotateL_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/rotateR_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/save_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/shortcuts_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/showBookmarks_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/toHeight_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/toContinuousScroll_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/toWidth_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/translator_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/zoom_18x18.svg
    ${PROJECT_SOURCE_DIR}/images/viewer_toolbar/menuArrow.svg
)
set(yacreader_file_files
    ${PROJECT_SOURCE_DIR}/files/about.html
    ${PROJECT_SOURCE_DIR}/files/helpYACReader.html
    ${PROJECT_SOURCE_DIR}/files/shortcuts.html
)
set(yacreader_file_files_es
    ${PROJECT_SOURCE_DIR}/files/about_es_ES.html
    ${PROJECT_SOURCE_DIR}/files/helpYACReader_es_ES.html
)
set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/files/about_es_ES.html
    PROPERTIES QT_RESOURCE_ALIAS "about.html"
)
set_source_files_properties(
    ${PROJECT_SOURCE_DIR}/files/helpYACReader_es_ES.html
    PROPERTIES QT_RESOURCE_ALIAS "helpYACReader.html"
)
set(yacreader_theme_files
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
set(yacreader_common_theme_image_files
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-system.svg
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-light.svg
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-dark.svg
    ${PROJECT_SOURCE_DIR}/images/appearance_config/theme-mode-custom.svg
)

qt_add_resources(YACReader "yacreader_images"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}"
    FILES
        ${yacreader_image_files}
)
qt_add_resources(YACReader "yacreader_files"
    PREFIX "/files"
    BASE "${PROJECT_SOURCE_DIR}/files"
    FILES
        ${yacreader_file_files}
)
qt_add_resources(YACReader "yacreader_files_es"
    PREFIX "/files"
    LANG "es_ES"
    BASE "${PROJECT_SOURCE_DIR}/files"
    FILES
        ${yacreader_file_files_es}
)
qt_add_resources(YACReader "yacreader_themes"
    PREFIX "/themes"
    BASE "${CMAKE_CURRENT_SOURCE_DIR}/themes"
    FILES
        ${yacreader_theme_files}
)
qt_add_resources(YACReader "yacreader_common_theme_images"
    PREFIX "/"
    BASE "${PROJECT_SOURCE_DIR}"
    FILES
        ${yacreader_common_theme_image_files}
)

# Translations
qt_add_translations(YACReader
    TS_FILES
        yacreader_es.ts
        yacreader_fr.ts
        yacreader_ru.ts
        yacreader_pt.ts
        yacreader_nl.ts
        yacreader_tr.ts
        yacreader_de.ts
        yacreader_zh_CN.ts
        yacreader_zh_TW.ts
        yacreader_zh_HK.ts
        yacreader_it.ts
        yacreader_source.ts
        yacreader_en.ts
)

target_link_libraries(YACReader PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::GuiPrivate
    Qt6::Network
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::TextToSpeech
    Qt6::Svg
    Qt6::Core5Compat
    comic_backend
    common_gui
    rhi_flow_reader
    custom_widgets_reader
    shortcuts_reader
    cbx_backend
    image_processing
    QsLog
)

# Platform-specific
if(WIN32)
    target_sources(YACReader PRIVATE icon.rc)
    target_link_libraries(YACReader PRIVATE oleaut32 ole32 shell32 user32)
endif()

if(APPLE)
    target_sources(YACReader PRIVATE YACReader.icns)
    set_source_files_properties(YACReader.icns PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
    set_target_properties(YACReader PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_INFO_PLIST "${CMAKE_CURRENT_SOURCE_DIR}/Info.plist"
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yacreader.YACReader"
        MACOSX_BUNDLE_BUNDLE_NAME "YACReader"
    )
    target_link_libraries(YACReader PRIVATE
        "-framework Foundation"
        "-framework ApplicationServices"
        "-framework AppKit"
    )
endif()

# Linux install
if(UNIX AND NOT APPLE)
    target_compile_definitions(YACReader PRIVATE
        "DATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}\"")

    install(TARGETS YACReader RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
    install(FILES
        "${PROJECT_SOURCE_DIR}/YACReader.1"
        DESTINATION ${CMAKE_INSTALL_MANDIR}/man1)
    install(FILES
        "${PROJECT_SOURCE_DIR}/YACReader.desktop"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/applications)
    install(FILES
        "${PROJECT_SOURCE_DIR}/YACReader.svg"
        DESTINATION ${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps)
endif()
