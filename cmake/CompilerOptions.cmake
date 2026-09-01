# Compiler policy for YACReader-owned targets.
# Keep this target internal so third-party code does not inherit our rules.
add_library(yacreader_build_options INTERFACE)

# Link-time optimization for Release builds of YACReader-owned targets.
# Applied per-target in yacreader_apply_build_options() rather than globally,
# so third_party code keeps building with its own settings.
include(CheckIPOSupported)
check_ipo_supported(RESULT YACREADER_IPO_SUPPORTED OUTPUT YACREADER_IPO_ERROR)
if(NOT YACREADER_IPO_SUPPORTED)
    message(STATUS "LTO not available, building without it: ${YACREADER_IPO_ERROR}")
endif()

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
        if(YACREADER_IPO_SUPPORTED)
            set_property(TARGET "${target_name}"
                PROPERTY INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE)
        endif()
    endforeach()
endfunction()
