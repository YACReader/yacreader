# File organization (rename files / organize into folders) for YACReaderLibrary

add_library(organize_files STATIC
    organize_files_plan.h
    organize_files_plan.cpp
    organize_files_journal.h
    organize_files_journal.cpp
    organize_files_worker.h
    organize_files_worker.cpp
    organize_files_dialog.h
    organize_files_dialog.cpp
    organize_files_coordinator.h
    organize_files_coordinator.cpp
)
target_include_directories(organize_files PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
yacreader_apply_build_options(organize_files)
# App-specific theme.h needed for themable.h → theme_manager.h → theme.h chain
target_include_directories(organize_files PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/themes
)
# ComicModel and FolderModel live in the app target; their headers come from the
# db_helper include dirs, their symbols from the final app link.
target_link_libraries(organize_files PUBLIC
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
    common_all
    common_gui
    custom_widgets_library
    db_helper
    QsLog
)
