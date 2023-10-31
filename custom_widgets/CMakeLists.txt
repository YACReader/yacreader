add_library(
  widgets_common
  help_about_dialog.cpp
  help_about_dialog.h
  rounded_corners_dialog.cpp
  rounded_corners_dialog.h
  whats_new_controller.cpp
  whats_new_controller.h
  whats_new_dialog.cpp
  whats_new_dialog.h
  yacreader_busy_widget.cpp
  yacreader_busy_widget.h
  yacreader_field_edit.cpp
  yacreader_field_edit.h
  yacreader_field_plain_text_edit.cpp
  yacreader_field_plain_text_edit.h
  yacreader_spin_slider_widget.cpp
  yacreader_spin_slider_widget.h
  yacreader_tool_bar_stretch.cpp
  yacreader_tool_bar_stretch.h)
target_link_libraries(widgets_common Qt::Widgets yr_global)

add_library(custom_widgets_library)
target_sources(
  custom_widgets_library
  PRIVATE yacreader_deleting_progress.cpp
          yacreader_deleting_progress.h
          yacreader_flow_config_widget.cpp
          yacreader_flow_config_widget.h
          yacreader_flow.cpp
          yacreader_flow.h
          yacreader_gl_flow_config_widget.cpp
          yacreader_gl_flow_config_widget.h
          yacreader_library_item_widget.cpp
          yacreader_library_item_widget.h
          yacreader_library_list_widget.cpp
          yacreader_library_list_widget.h
          yacreader_options_dialog.cpp
          yacreader_options_dialog.h
          yacreader_search_line_edit.cpp
          yacreader_search_line_edit.h
          yacreader_sidebar.cpp
          yacreader_sidebar.h
          yacreader_table_view.cpp
          yacreader_table_view.h
          yacreader_titled_toolbar.cpp
          yacreader_titled_toolbar.h
          yacreader_tool_bar_stretch.cpp
          yacreader_tool_bar_stretch.h
          yacreader_treeview.cpp
          yacreader_treeview.h)
target_link_libraries(
  custom_widgets_library
  db_helper
  gl_flow_library
  QsLog
  Qt::Widgets
  shortcuts_library
  widgets_common
  widgets_common
  yr_global)
target_include_directories(custom_widgets_library PUBLIC .)

add_library(custom_widgets_reader)
target_sources(
  custom_widgets_reader
  PRIVATE yacreader_flow_config_widget.cpp
          yacreader_flow_config_widget.h
          yacreader_flow.cpp
          yacreader_flow.h
          yacreader_gl_flow_config_widget.cpp
          yacreader_gl_flow_config_widget.h
          yacreader_options_dialog.cpp
          yacreader_options_dialog.h)
target_link_libraries(custom_widgets_reader Qt::Widgets yr_global
                      gl_flow_reader widgets_common shortcuts_reader)
target_include_directories(custom_widgets_reader PUBLIC .)
