add_executable(YACReader main.cpp)
target_sources(
  YACReader
  PRIVATE bookmarks_dialog.cpp
          bookmarks_dialog.h
          configuration.cpp
          configuration.h
          goto_dialog.cpp
          goto_dialog.h
          goto_flow_gl.cpp
          goto_flow_toolbar.cpp
          goto_flow_toolbar.h
          goto_flow_widget.cpp
          goto_flow_widget.h
          goto_flow.cpp
          goto_flow.h
          magnifying_glass.cpp
          magnifying_glass.h
          main_window_viewer.cpp
          main_window_viewer.h
          notifications_label_widget.cpp
          notifications_label_widget.h
          options_dialog.cpp
          options_dialog.h
          page_label_widget.cpp
          page_label_widget.h
          render.cpp
          render.h
          translator.cpp
          translator.h
          viewer.cpp
          viewer.h
          width_slider.cpp
          width_slider.h
          yacreader_local_client.cpp
          yacreader_local_client.h)

qt_add_resources(yr_resources yacreader_images.qrc yacreader_files.qrc)
target_sources(YACReader PRIVATE ${yr_resources})

qt_add_translations(
  YACReader
  TS_FILES
  yacreader_de.ts
  yacreader_en.ts
  yacreader_es.ts
  yacreader_fr.ts
  yacreader_it.ts
  yacreader_nl.ts
  yacreader_pt.ts
  yacreader_ru.ts
  yacreader_tr.ts
  yacreader_zh_CN.ts
  yacreader_zh_HK.ts
  yacreader_zh_TW.ts
  QM_FILES_OUTPUT_VARIABLE
  reader_translations)

target_link_libraries(
  YACReader
  PRIVATE comic_backend
          common_all
          common_gui
          custom_widgets_reader
          naturalsort
          QsLog
          Qt::Gui
          Qt::Multimedia
          Qt::Network
          Qt::OpenGLWidgets
          Qt::Svg
          Qt::Widgets
          version_checker
          yr_global
          Qt::Core5Compat)
target_compile_definitions(YACReader
                           PRIVATE DATADIR="${CMAKE_INSTALL_FULL_DATADIR}")
