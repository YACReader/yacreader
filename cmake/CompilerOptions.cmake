# Compiler options for YACReader

if(MSVC)
    # Prevent windows.h from defining min/max macros that conflict with
    # std::min, std::max, std::numeric_limits<T>::max(), etc.
    add_compile_definitions(NOMINMAX)

    # /Zc:__cplusplus: report correct __cplusplus value
    # /permissive-: strict standard conformance
    add_compile_options(/Zc:__cplusplus /permissive-)
endif()
