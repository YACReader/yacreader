# Shortcuts management for YACReader
# Two targets: reader-specific, library-specific (compiled with different defines)

set(SHORTCUTS_SOURCES
    edit_shortcuts_dialog.h
    edit_shortcuts_dialog.cpp
    actions_groups_model.h
    actions_groups_model.cpp
    actions_shortcuts_model.h
    actions_shortcuts_model.cpp
    edit_shortcut_item_delegate.h
    edit_shortcut_item_delegate.cpp
    shortcuts_manager.h
    shortcuts_manager.cpp
)

# --- shortcuts_reader (YACREADER define) ---
add_library(shortcuts_reader STATIC ${SHORTCUTS_SOURCES})
target_include_directories(shortcuts_reader PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
# App-specific theme.h needed because themable.h → theme_manager.h → theme.h
target_include_directories(shortcuts_reader PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReader/themes
)
target_compile_definitions(shortcuts_reader PRIVATE YACREADER)
target_link_libraries(shortcuts_reader PUBLIC Qt::Core Qt::Widgets yr_global common_gui)

# --- shortcuts_library (YACREADER_LIBRARY define) ---
add_library(shortcuts_library STATIC ${SHORTCUTS_SOURCES})
target_include_directories(shortcuts_library PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_include_directories(shortcuts_library PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/themes
)
target_compile_definitions(shortcuts_library PRIVATE YACREADER_LIBRARY)
target_link_libraries(shortcuts_library PUBLIC Qt::Core Qt::Widgets yr_global common_gui)
