# Comic start page test

qt_add_executable(comic_start_page_test
    main.cpp
)
yacreader_apply_build_options(comic_start_page_test)
target_link_libraries(comic_start_page_test PRIVATE
    Qt6::Core
    Qt6::Test
    comic_backend
)
if(WIN32)
    target_link_libraries(comic_start_page_test PRIVATE oleaut32 ole32)
endif()
add_test(NAME comic_start_page_test COMMAND comic_start_page_test)
