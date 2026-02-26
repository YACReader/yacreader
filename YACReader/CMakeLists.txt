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
    ${CMAKE_SOURCE_DIR}/common/themes/theme_manager.h
    ${CMAKE_SOURCE_DIR}/common/themes/theme_manager.cpp
)

target_include_directories(YACReader PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/themes
)

target_compile_definitions(YACReader PRIVATE YACREADER)

# Resources
qt_add_resources(yacreader_images_rcc "${CMAKE_CURRENT_SOURCE_DIR}/yacreader_images.qrc")
qt_add_resources(yacreader_files_rcc "${CMAKE_CURRENT_SOURCE_DIR}/yacreader_files.qrc")
target_sources(YACReader PRIVATE
    ${yacreader_images_rcc}
    ${yacreader_files_rcc}
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
        yacreader_en.ts
)

target_link_libraries(YACReader PRIVATE
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::Network
    Qt::Widgets
    Qt::Multimedia
    Qt::Svg
    Qt::Core5Compat
    comic_backend
    common_gui
    rhi_flow_reader
    custom_widgets_reader
    shortcuts_reader
    cbx_backend
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
endif()
