qt_add_executable(library_load_benchmark
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/comic_model.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db/comic_query_result_processor.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/current_comic_view_helper.cpp
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server/yacreader_server_data_helper.cpp
)
yacreader_apply_build_options(library_load_benchmark)
target_include_directories(library_load_benchmark PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/db
    ${PROJECT_SOURCE_DIR}/YACReaderLibrary/server
)
target_link_libraries(library_load_benchmark PRIVATE Qt6::Core Qt6::Gui Qt6::Sql Qt6::Test db_helper library_common common_gui concurrent_queue)
add_test(NAME library_load_benchmark COMMAND library_load_benchmark --sizes 10 --runs 1)
