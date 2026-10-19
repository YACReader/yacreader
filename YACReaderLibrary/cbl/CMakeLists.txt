# CBL reading lists: reading, matching, and importing; no GUI, so the server can use it

add_library(cbl STATIC
    cbl_reader.h
    cbl_reader.cpp
    cbl_matcher.h
    cbl_matcher.cpp
    cbl_importer.h
    cbl_importer.cpp
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
