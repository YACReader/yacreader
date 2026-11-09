add_library(server_test_support STATIC
    test_http_server.h
    test_http_server.cpp
)
yacreader_apply_build_options(server_test_support)
target_include_directories(server_test_support PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(server_test_support PUBLIC Qt6::Core Qt6::Network Qt6::Sql server library_common)
