add_library(shortcuts_common)
target_sources(
  shortcuts_common
  PRIVATE actions_groups_model.h
          actions_shortcuts_model.h
          edit_shortcut_item_delegate.h
          edit_shortcuts_dialog.h
          edit_shortcuts_dialog.cpp
          actions_groups_model.cpp
          actions_shortcuts_model.cpp
          edit_shortcut_item_delegate.cpp)
target_include_directories(shortcuts_common PUBLIC .)
target_link_libraries(shortcuts_common Qt::Widgets yr_global QsLog)

add_library(shortcuts_reader shortcuts_manager.h shortcuts_manager.cpp)
target_compile_definitions(shortcuts_reader PRIVATE YACREADER)
target_include_directories(shortcuts_reader PUBLIC .)
target_link_libraries(shortcuts_reader shortcuts_common Qt::Widgets yr_global)

add_library(shortcuts_library shortcuts_manager.h shortcuts_manager.cpp)
target_compile_definitions(shortcuts_library PRIVATE YACREADER_LIBRARY)
target_include_directories(shortcuts_library PUBLIC .)
target_link_libraries(shortcuts_library shortcuts_common Qt::Widgets yr_global)
