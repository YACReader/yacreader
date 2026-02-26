# Custom widgets for YACReader
# Shared widgets + app-specific variants

# Files shared between both YACReader and YACReaderLibrary
set(WIDGETS_COMMON_SOURCES
    help_about_dialog.h
    help_about_dialog.cpp
    yacreader_field_edit.h
    yacreader_field_edit.cpp
    yacreader_field_plain_text_edit.h
    yacreader_field_plain_text_edit.cpp
    yacreader_options_dialog.h
    yacreader_options_dialog.cpp
    yacreader_spin_slider_widget.h
    yacreader_spin_slider_widget.cpp
    yacreader_tool_bar_stretch.h
    yacreader_tool_bar_stretch.cpp
    yacreader_busy_widget.h
    yacreader_busy_widget.cpp
    rounded_corners_dialog.h
    rounded_corners_dialog.cpp
    whats_new_dialog.h
    whats_new_dialog.cpp
    whats_new_controller.h
    whats_new_controller.cpp
    yacreader_3d_flow_config_widget.h
    yacreader_3d_flow_config_widget.cpp
)

# macOS toolbar (shared by both apps)
if(APPLE)
    list(APPEND WIDGETS_COMMON_SOURCES
        yacreader_macosx_toolbar.h
        yacreader_macosx_toolbar.mm
    )
endif()

# --- custom_widgets_reader (YACReader-specific) ---
add_library(custom_widgets_reader STATIC ${WIDGETS_COMMON_SOURCES})
target_include_directories(custom_widgets_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# App-specific theme.h needed because themable.h → theme_manager.h → theme.h
target_include_directories(custom_widgets_reader PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReader/themes
)
target_compile_definitions(custom_widgets_reader PRIVATE YACREADER)
target_link_libraries(custom_widgets_reader PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Network
    common_gui
    rhi_flow_reader
    QsLog
)
if(APPLE)
    target_link_libraries(custom_widgets_reader PRIVATE
        "-framework Foundation"
        "-framework AppKit"
    )
endif()

# --- custom_widgets_library (YACReaderLibrary-specific) ---
set(LIBRARY_EXTRA_WIDGETS
    yacreader_search_line_edit.h
    yacreader_search_line_edit.cpp
    yacreader_titled_toolbar.h
    yacreader_titled_toolbar.cpp
    yacreader_table_view.h
    yacreader_table_view.cpp
    yacreader_sidebar.h
    yacreader_sidebar.cpp
    yacreader_library_list_widget.h
    yacreader_library_list_widget.cpp
    yacreader_library_item_widget.h
    yacreader_library_item_widget.cpp
    yacreader_treeview.h
    yacreader_treeview.cpp
    yacreader_cover_label.h
    yacreader_cover_label.cpp
)

add_library(custom_widgets_library STATIC
    ${WIDGETS_COMMON_SOURCES}
    ${LIBRARY_EXTRA_WIDGETS}
)
target_include_directories(custom_widgets_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# App-specific theme.h + YACReaderLibrary headers needed by macosx_toolbar
target_include_directories(custom_widgets_library PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/themes
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/db
)
target_compile_definitions(custom_widgets_library PRIVATE YACREADER_LIBRARY)
target_link_libraries(custom_widgets_library PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Network
    common_gui
    rhi_flow_library
    QsLog
)
if(APPLE)
    target_link_libraries(custom_widgets_library PRIVATE
        "-framework Foundation"
        "-framework AppKit"
    )
endif()
