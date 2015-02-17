INCLUDEPATH += $$PWD
DEPENDPATH += $$PWD

HEADERS += $$PWD/help_about_dialog.h \
						$$PWD/yacreader_field_edit.h \
                        $$PWD/yacreader_field_plain_text_edit.h \
                        $$PWD/yacreader_flow.h \
                        $$PWD/yacreader_flow_config_widget.h \
                        #$$PWD/yacreader_gl_flow_config_widget.h \
                        $$PWD/yacreader_options_dialog.h \
                        $$PWD/yacreader_spin_slider_widget.h \
                        $$PWD/yacreader_tool_bar_stretch.h \
                                                $$PWD/yacreader_busy_widget.h

macx{
HEADERS += $$PWD/yacreader_macosx_toolbar.h
}

						
						 
SOURCES += $$PWD/help_about_dialog.cpp \
						 $$PWD/yacreader_field_edit.cpp \
                         $$PWD/yacreader_field_plain_text_edit.cpp \
                         $$PWD/yacreader_flow.cpp \
                         $$PWD/yacreader_flow_config_widget.cpp \
                         #$$PWD/yacreader_gl_flow_config_widget.cpp \
                         $$PWD/yacreader_options_dialog.cpp \
                         $$PWD/yacreader_spin_slider_widget.cpp \
                         $$PWD/yacreader_tool_bar_stretch.cpp \
						$$PWD/yacreader_busy_widget.cpp
macx{
OBJECTIVE_SOURCES += $$PWD/yacreader_macosx_toolbar.mm
}

