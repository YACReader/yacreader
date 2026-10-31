# Comic first page test

qt_add_executable(comic_first_page_test
    main.cpp
)
yacreader_apply_build_options(comic_first_page_test)
target_link_libraries(comic_first_page_test PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Test
    comic_backend
)
if(WIN32)
    target_link_libraries(comic_first_page_test PRIVATE oleaut32 ole32)
endif()
add_test(NAME comic_first_page_test COMMAND comic_first_page_test)
