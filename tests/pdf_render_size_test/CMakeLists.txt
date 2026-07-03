# PDF render size test

qt_add_executable(pdf_render_size_test
    main.cpp
)
yacreader_apply_build_options(pdf_render_size_test)
target_include_directories(pdf_render_size_test PRIVATE
    ${PROJECT_SOURCE_DIR}/common
)
target_link_libraries(pdf_render_size_test PRIVATE
    Qt6::Core
    Qt6::Test
)
add_test(NAME pdf_render_size_test COMMAND pdf_render_size_test)
