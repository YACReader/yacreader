qt_add_executable(server_v2_routes_test
    main.cpp
)
yacreader_apply_build_options(server_v2_routes_test)
target_compile_definitions(server_v2_routes_test PRIVATE EXPECTED_RESPONSES_PATH="${CMAKE_CURRENT_SOURCE_DIR}/expected_v2_responses.json")
target_link_libraries(server_v2_routes_test PRIVATE Qt6::Core Qt6::Gui Qt6::Network Qt6::Sql Qt6::Test server library_common)
add_test(NAME server_v2_routes_test COMMAND server_v2_routes_test)
