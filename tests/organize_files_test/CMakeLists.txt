qt_add_executable(organize_files_test
    main.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/organize_files/organize_files_plan.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/organize_files/organize_files_journal.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/organize_files/organize_files_worker.cpp
)
yacreader_apply_build_options(organize_files_test)
target_include_directories(organize_files_test PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/organize_files
)
target_link_libraries(organize_files_test PRIVATE
    Qt6::Core
    Qt6::Sql
    Qt6::Test
    db_helper
    library_common
)

add_test(NAME organize_files_test COMMAND organize_files_test)
