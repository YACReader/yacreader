# Compiler policy for YACReader-owned targets.
# Keep this target internal so third-party code does not inherit our rules.
add_library(yacreader_build_options INTERFACE)

target_compile_definitions(yacreader_build_options INTERFACE
    QT_DISABLE_DEPRECATED_UP_TO=0x060400
)

if(MSVC)
    target_compile_definitions(yacreader_build_options INTERFACE
        # Prevent windows.h from defining min/max macros that conflict with
        # std::min, std::max, std::numeric_limits<T>::max(), etc.
        NOMINMAX
    )

    target_compile_options(yacreader_build_options INTERFACE
        # /Zc:__cplusplus: report correct __cplusplus value
        # /permissive-: strict standard conformance
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/Zc:__cplusplus>
        $<$<COMPILE_LANG_AND_ID:CXX,MSVC>:/permissive->
    )
endif()

function(yacreader_apply_build_options)
    foreach(target_name IN LISTS ARGN)
        if(NOT TARGET "${target_name}")
            message(FATAL_ERROR "yacreader_apply_build_options(): unknown target '${target_name}'")
        endif()
        target_link_libraries("${target_name}" PRIVATE yacreader_build_options)
    endforeach()
endfunction()
