include_guard(GLOBAL)

function(yacreader_get_windows_arch_subdir out_var)
    if(CMAKE_SYSTEM_PROCESSOR STREQUAL "ARM64" OR CMAKE_CXX_COMPILER_ARCHITECTURE_ID STREQUAL "ARM64")
        set(_arch "arm64")
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 4)
        set(_arch "x86")
    else()
        set(_arch "x64")
    endif()

    set(${out_var} "${_arch}" PARENT_SCOPE)
endfunction()

function(yacreader_add_imported_library target_name)
    set(options)
    set(oneValueArgs TYPE LOCATION IMPORTED_IMPLIB INCLUDE_DIR)
    set(multiValueArgs LINK_LIBRARIES)
    cmake_parse_arguments(YR "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

    if(NOT YR_TYPE)
        set(YR_TYPE UNKNOWN)
    endif()

    if(NOT TARGET "${target_name}")
        add_library("${target_name}" ${YR_TYPE} IMPORTED GLOBAL)
    endif()

    if(YR_LOCATION)
        set_property(TARGET "${target_name}" PROPERTY IMPORTED_LOCATION "${YR_LOCATION}")
    endif()

    if(YR_IMPORTED_IMPLIB)
        set_property(TARGET "${target_name}" PROPERTY IMPORTED_IMPLIB "${YR_IMPORTED_IMPLIB}")
    endif()

    if(YR_INCLUDE_DIR)
        set_property(TARGET "${target_name}" PROPERTY INTERFACE_INCLUDE_DIRECTORIES "${YR_INCLUDE_DIR}")
    endif()

    if(YR_LINK_LIBRARIES)
        set_property(TARGET "${target_name}" PROPERTY INTERFACE_LINK_LIBRARIES "${YR_LINK_LIBRARIES}")
    endif()
endfunction()
