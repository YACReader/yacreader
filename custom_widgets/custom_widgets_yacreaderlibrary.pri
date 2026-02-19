INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += \
        $$PWD/help_about_dialog.h \
        $$PWD/rounded_corners_dialog.h \
        $$PWD/whats_new_dialog.h \
        $$PWD/whats_new_controller.h \
        $$PWD/yacreader_field_edit.h \
        $$PWD/yacreader_field_plain_text_edit.h \
        $$PWD/yacreader_options_dialog.h \
        $$PWD/yacreader_search_line_edit.h \
        $$PWD/yacreader_spin_slider_widget.h \
        $$PWD/yacreader_tool_bar_stretch.h \
        $$PWD/yacreader_titled_toolbar.h \
        $$PWD/yacreader_table_view.h \
        $$PWD/yacreader_sidebar.h \
        $$PWD/yacreader_library_list_widget.h \
        $$PWD/yacreader_library_item_widget.h \
        $$PWD/yacreader_treeview.h \
        $$PWD/yacreader_busy_widget.h \
        $$PWD/yacreader_cover_label.h \
        $$PWD/yacreader_3d_flow_config_widget.h
macx{
HEADERS += $$PWD/yacreader_macosx_toolbar.h
}
						 
SOURCES += \
        $$PWD/help_about_dialog.cpp \
        $$PWD/rounded_corners_dialog.cpp \
        $$PWD/whats_new_dialog.cpp \
        $$PWD/whats_new_controller.cpp \
        $$PWD/yacreader_field_edit.cpp \
        $$PWD/yacreader_field_plain_text_edit.cpp \
        $$PWD/yacreader_options_dialog.cpp \
        $$PWD/yacreader_search_line_edit.cpp \
        $$PWD/yacreader_spin_slider_widget.cpp \
        $$PWD/yacreader_tool_bar_stretch.cpp \
        $$PWD/yacreader_titled_toolbar.cpp \
        $$PWD/yacreader_table_view.cpp \
        $$PWD/yacreader_sidebar.cpp \
        $$PWD/yacreader_library_list_widget.cpp \
        $$PWD/yacreader_library_item_widget.cpp \
        $$PWD/yacreader_treeview.cpp \
        $$PWD/yacreader_busy_widget.cpp \
        $$PWD/yacreader_cover_label.cpp \
        $$PWD/yacreader_3d_flow_config_widget.cpp
macx{
OBJECTIVE_SOURCES += $$PWD/yacreader_macosx_toolbar.mm
}
