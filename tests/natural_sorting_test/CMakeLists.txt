# Natural sorting test

qt_add_executable(natural_sorting_test
    main.cpp
)
yacreader_apply_build_options(natural_sorting_test)
target_link_libraries(natural_sorting_test PRIVATE
    naturalsort
    Qt6::Core
    Qt6::Test
)
add_test(NAME natural_sorting_test COMMAND natural_sorting_test)
