qt_add_executable(comic_vine_parser_test
    main.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_vine/comic_vine_json_parser.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_vine/comic_vine_client.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_vine/comic_vine_all_volume_comics_retriever.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_vine/model/response_parser.cpp
)
yacreader_apply_build_options(comic_vine_parser_test)
target_include_directories(comic_vine_parser_test PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_vine
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_vine/model
)
target_link_libraries(comic_vine_parser_test PRIVATE Qt6::Core Qt6::Network Qt6::Widgets Qt6::Test common_all common_gui QsLog)
add_test(NAME comic_vine_parser_test COMMAND comic_vine_parser_test)
