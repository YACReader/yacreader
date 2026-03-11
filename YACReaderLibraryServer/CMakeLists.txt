# YACReaderLibraryServer - Headless REST server

qt_add_executable(YACReaderLibraryServer
    main.cpp
    console_ui_library_creator.h
    console_ui_library_creator.cpp
    libraries_updater.h
    libraries_updater.cpp
)

target_include_directories(YACReaderLibraryServer PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/db
)

target_compile_definitions(YACReaderLibraryServer PRIVATE
    SERVER_RELEASE
    YACREADER_LIBRARY
)

target_sources(YACReaderLibraryServer PRIVATE ${yacreaderlibraryserver_images_rcc})

# Translations
qt_add_translations(YACReaderLibraryServer
    TS_FILES
        yacreaderlibraryserver_es.ts
        yacreaderlibraryserver_ru.ts
        yacreaderlibraryserver_pt.ts
        yacreaderlibraryserver_fr.ts
        yacreaderlibraryserver_nl.ts
        yacreaderlibraryserver_tr.ts
        yacreaderlibraryserver_de.ts
        yacreaderlibraryserver_zh_CN.ts
        yacreaderlibraryserver_zh_TW.ts
        yacreaderlibraryserver_zh_HK.ts
        yacreaderlibraryserver_source.ts
)

target_link_libraries(YACReaderLibraryServer PRIVATE
    Qt::Core
    Qt::Network
    Qt::Sql
    Qt::Core5Compat
    library_common
    db_helper
    comic_backend
    common_all
    naturalsort
    cbx_backend
    server
    QsLog
    QrCode
)

# Platform-specific
if(WIN32)
    target_link_libraries(YACReaderLibraryServer PRIVATE oleaut32 ole32 shell32 user32)
endif()

if(APPLE)
    set_target_properties(YACReaderLibraryServer PROPERTIES
        MACOSX_BUNDLE TRUE
        MACOSX_BUNDLE_GUI_IDENTIFIER "com.yacreader.YACReaderLibraryServer"
        MACOSX_BUNDLE_BUNDLE_NAME "YACReaderLibraryServer"
    )
    target_link_libraries(YACReaderLibraryServer PRIVATE
        "-framework Foundation"
        "-framework ApplicationServices"
        "-framework AppKit"
    )
endif()

# Linux install rules
if(UNIX AND NOT APPLE)
    target_compile_definitions(YACReaderLibraryServer PRIVATE
        "LIBDIR=\"${CMAKE_INSTALL_FULL_LIBDIR}\""
        "DATADIR=\"${CMAKE_INSTALL_FULL_DATADIR}\""
        "BINDIR=\"${CMAKE_INSTALL_FULL_BINDIR}\""
    )

    install(TARGETS YACReaderLibraryServer RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

    # Standalone server: install server web files alongside the binary
    if(BUILD_SERVER_STANDALONE)
        install(DIRECTORY ${CMAKE_SOURCE_DIR}/release/server
            DESTINATION ${CMAKE_INSTALL_DATADIR}/yacreader)
    endif()

    # systemd service
    install(FILES yacreaderlibraryserver.service
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/systemd/user)
endif()
