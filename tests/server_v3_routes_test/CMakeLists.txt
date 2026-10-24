qt_add_executable(server_v3_routes_test
    main.cpp
)
yacreader_apply_build_options(server_v3_routes_test)
target_link_libraries(server_v3_routes_test PRIVATE Qt6::Core Qt6::Test server_test_support)
add_test(NAME server_v3_routes_test COMMAND server_v3_routes_test)
