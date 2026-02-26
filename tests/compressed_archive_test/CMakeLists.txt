# Compressed archive test

qt_add_executable(compressed_archive_test
    main.cpp
)
target_link_libraries(compressed_archive_test PRIVATE
    Qt::Core
    cbx_backend
)
if(WIN32)
    target_link_libraries(compressed_archive_test PRIVATE oleaut32 ole32)
endif()
add_test(NAME compressed_archive_test COMMAND compressed_archive_test)
