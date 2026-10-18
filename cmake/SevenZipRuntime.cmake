# With the 7zip backend, the apps load the 7z library at runtime from utils/ next to the executable
# (on Linux after the installed paths). Copy the bundled library there, so builds run and tests can read archives.
function(yacreader_deploy_7z_runtime target_name)
    if(NOT DECOMPRESSION_BACKEND STREQUAL "7zip")
        return()
    endif()

    set(dependencies "${CMAKE_SOURCE_DIR}/dependencies/7zip")
    if(WIN32)
        if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(ARM64|arm64|aarch64)$")
            set(library "${dependencies}/win/arm64/7z.dll")
        elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
            set(library "${dependencies}/win/x64/7z.dll")
        else()
            set(library "${dependencies}/win/x86/7z.dll")
        endif()
    elseif(APPLE)
        set(library "${dependencies}/macx/universal/7z.so")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(aarch64|arm64)$")
        set(library "${dependencies}/linux/arm64/7z.so")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(x86_64|AMD64|amd64)$")
        set(library "${dependencies}/linux/x86_64/7z.so")
    elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^i.86$")
        set(library "${dependencies}/linux/x86/7z.so")
    endif()

    if(NOT DEFINED library OR NOT EXISTS "${library}")
        message(WARNING "No bundled 7z library for ${CMAKE_SYSTEM_NAME} ${CMAKE_SYSTEM_PROCESSOR}; ${target_name} needs an installed one")
        return()
    endif()

    get_filename_component(library_name "${library}" NAME)
    add_custom_command(TARGET "${target_name}" POST_BUILD
        COMMAND "${CMAKE_COMMAND}" -E make_directory "$<TARGET_FILE_DIR:${target_name}>/utils"
        COMMAND "${CMAKE_COMMAND}" -E copy_if_different "${library}" "$<TARGET_FILE_DIR:${target_name}>/utils/${library_name}"
        VERBATIM)
endfunction()
