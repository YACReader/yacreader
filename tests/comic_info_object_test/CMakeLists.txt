qt_add_executable(comic_info_object_test
    main.cpp
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_info_object.h
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/comic_info_object.cpp
)
yacreader_apply_build_options(comic_info_object_test)
target_include_directories(comic_info_object_test PRIVATE ${CMAKE_SOURCE_DIR}/YACReaderLibrary)
target_link_libraries(comic_info_object_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    db_helper
    library_common
)

add_test(NAME comic_info_object_test COMMAND comic_info_object_test)
