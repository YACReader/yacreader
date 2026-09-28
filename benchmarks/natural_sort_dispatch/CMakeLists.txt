cmake_minimum_required(VERSION 3.16)
project(natural_sort_dispatch LANGUAGES CXX)
find_package(Qt6 REQUIRED COMPONENTS Core)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

foreach(mode IN ITEMS direct indirect boolean)
    add_executable(dispatch_${mode} main.cpp)
    target_compile_definitions(dispatch_${mode} PRIVATE DISPATCH_${mode})
    target_link_libraries(dispatch_${mode} PRIVATE Qt6::Core)
endforeach()
