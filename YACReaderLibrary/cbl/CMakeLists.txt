# CBL reading lists: reading, matching, and importing; no GUI, so the server can use it

add_library(cbl STATIC
    cbl_reader.h
    cbl_reader.cpp
    cbl_matcher.h
    cbl_matcher.cpp
    cbl_importer.h
    cbl_importer.cpp
    cbl_list_match.h
    cbl_list_match.cpp
    cbl_writer.h
    cbl_writer.cpp
)
target_include_directories(cbl PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)
yacreader_apply_build_options(cbl)
target_link_libraries(cbl PUBLIC
    Qt6::Core
    Qt6::Sql
    db_helper
    library_common
    QsLog
)

# The import dialog of YACReaderLibrary
if(NOT BUILD_SERVER_STANDALONE)
    add_library(cbl_gui STATIC
        cbl_import_model.h
        cbl_import_model.cpp
        cbl_entry_detail.h
        cbl_entry_detail.cpp
        cbl_entries_view.h
        cbl_entries_view.cpp
        cbl_find_comic_page.h
        cbl_find_comic_page.cpp
        cbl_import_dialog.h
        cbl_import_dialog.cpp
        cbl_match_comic_dialog.h
        cbl_match_comic_dialog.cpp
    )
    target_include_directories(cbl_gui PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
    )
    yacreader_apply_build_options(cbl_gui)
    target_link_libraries(cbl_gui PUBLIC
        Qt6::Core
        Qt6::Widgets
        Qt6::Sql
        cbl
        naturalsort
        QsLog
    )
endif()
