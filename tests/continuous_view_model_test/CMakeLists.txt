# Continuous view model test

qt_add_executable(continuous_view_model_test
    main.cpp
    ${PROJECT_SOURCE_DIR}/YACReader/continuous_view_model.cpp
    ${PROJECT_SOURCE_DIR}/YACReader/continuous_view_model.h
)
yacreader_apply_build_options(continuous_view_model_test)
target_include_directories(continuous_view_model_test PRIVATE
    ${PROJECT_SOURCE_DIR}/YACReader
)
target_link_libraries(continuous_view_model_test PRIVATE
    Qt6::Core
    Qt6::Test
)
add_test(NAME continuous_view_model_test COMMAND continuous_view_model_test)
