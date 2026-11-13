# Double page layout test

qt_add_executable(double_page_layout_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReader/double_page_layout.cpp
    ${PROJECT_SOURCE_DIR}/YACReader/double_page_layout.h
)
yacreader_apply_build_options(double_page_layout_test)
target_include_directories(double_page_layout_test PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReader
)
target_link_libraries(double_page_layout_test PRIVATE
    Qt6::Core
    Qt6::Test
)
add_test(NAME double_page_layout_test COMMAND double_page_layout_test)
