# Concurrent queue test

qt_add_executable(concurrent_queue_test
    concurrent_queue_test.cpp
)
target_link_libraries(concurrent_queue_test PRIVATE
    Qt::Core
    Qt::Test
    concurrent_queue
)
add_test(NAME concurrent_queue_test
    COMMAND concurrent_queue_test -maxwarnings 100000)
