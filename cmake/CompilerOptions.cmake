# Compiler options for YACReader
# Mirrors config.pri: C++20, NOMINMAX, MSVC conformance flags

add_compile_definitions(NOMINMAX)

if(MSVC)
    # /Zc:__cplusplus: report correct __cplusplus value
    # /permissive-: strict standard conformance
    add_compile_options(/Zc:__cplusplus /permissive-)

    # Release optimizations (mirrors qmake QMAKE_CXXFLAGS_RELEASE)
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " /DNDEBUG")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    string(APPEND CMAKE_CXX_FLAGS_RELEASE " -DNDEBUG")
endif()

# Qt deprecation warnings
add_compile_definitions(
    QT_DEPRECATED_WARNINGS
)
