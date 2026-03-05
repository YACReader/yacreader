# Common libraries for YACReader
# Fine-grained STATIC targets per concern

# --- yr_global (no GUI, used by all 3 apps) ---
add_library(yr_global STATIC
    yacreader_global.h
    yacreader_global.cpp
)
target_include_directories(yr_global PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(yr_global PUBLIC Qt::Core)
if(UNIX AND NOT APPLE)
    target_compile_definitions(yr_global PRIVATE
        "LIBDIR=\"${CMAKE_INSTALL_FULL_LIBDIR}\"")
endif()

# --- naturalsort ---
add_library(naturalsort STATIC
    qnaturalsorting.h
    qnaturalsorting.cpp
)
target_include_directories(naturalsort PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(naturalsort PUBLIC Qt::Core)

# --- concurrent_queue ---
add_library(concurrent_queue STATIC
    concurrent_queue.h
    concurrent_queue.cpp
)
target_include_directories(concurrent_queue PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(concurrent_queue PUBLIC Qt::Core)

# --- worker (header-only thread helpers) ---
add_library(worker INTERFACE)
target_include_directories(worker INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})
target_sources(worker INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/worker_thread.h
    ${CMAKE_CURRENT_SOURCE_DIR}/release_acquire_atomic.h
)

# --- common_all (shared non-GUI code used by all 3 apps) ---
add_library(common_all STATIC
    comic_db.h
    comic_db.cpp
    folder.h
    folder.cpp
    library_item.h
    library_item.cpp
    bookmarks.h
    bookmarks.cpp
    http_worker.h
    http_worker.cpp
    cover_utils.h
    cover_utils.cpp
    global_info_provider.h
    global_info_provider.cpp
)
target_include_directories(common_all PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(common_all PUBLIC
    Qt::Core
    Qt::Gui
    Qt::Sql
    Qt::Network
    yr_global
    naturalsort
    pdf_backend_iface
)

# --- comic_backend (comic file handling + PDF) ---
add_library(comic_backend STATIC
    comic.h
    comic.cpp
    pdf_comic.h
)

# PDF source depends on backend
if(PDF_BACKEND STREQUAL "pdfkit")
    target_sources(comic_backend PRIVATE pdf_comic.mm)
elseif(NOT PDF_BACKEND STREQUAL "no_pdf")
    target_sources(comic_backend PRIVATE pdf_comic.cpp)
endif()

target_include_directories(comic_backend PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
target_link_libraries(comic_backend PUBLIC
    Qt::Core
    Qt::Gui
    common_all
    pdf_backend_iface
    cbx_backend
    QsLog
)

# GUI-only targets: not needed for server-standalone builds (Qt 6.4+) which
# lack Qt::Widgets, Qt::ShaderTools, Qt::GuiPrivate, etc.
if(NOT BUILD_SERVER_STANDALONE)

# --- common_gui (GUI-only code: widgets, version check, themes infrastructure) ---
# NOTE: theme_manager.h/cpp is NOT included here because it depends on app-specific
# theme.h (YACReader vs YACReaderLibrary have different Theme structs).
# Each app includes theme_manager directly alongside its own theme.h/theme_factory.
add_library(common_gui STATIC
    custom_widgets.h
    custom_widgets.cpp
    check_new_version.h
    check_new_version.cpp
    yacreader_global_gui.h
    yacreader_global_gui.cpp
    exit_check.h
    exit_check.cpp
    scroll_management.h
    scroll_management.cpp
    # themes infrastructure (does NOT depend on app-specific theme.h)
    themes/icon_utils.h
    themes/icon_utils.cpp
    themes/appearance_configuration.h
    themes/appearance_configuration.cpp
    themes/theme_variant.h
    themes/themable.h
    themes/yacreader_icon.h
    themes/shared/help_about_dialog_theme.h
    themes/shared/whats_new_dialog_theme.h
    themes/theme_editor_dialog.h
    themes/theme_editor_dialog.cpp
    themes/theme_meta.h
    themes/theme_repository.h
    themes/theme_repository.cpp
    themes/appearance_tab_widget.h
    themes/appearance_tab_widget.cpp
)
target_include_directories(common_gui PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/themes
    ${CMAKE_CURRENT_SOURCE_DIR}/themes/shared
)

target_link_libraries(common_gui PUBLIC
    Qt::Core
    Qt::Core5Compat
    Qt::Widgets
    Qt::Network
    Qt::Svg
    common_all
    yr_global
    QsLog
)

# --- RHI flow (3D coverflow) ---
# Compiled twice with different defines, like the old GL flow pattern.
# Shaders are compiled from GLSL 450 source at build time via qt_add_shaders().

set(RHI_FLOW_SOURCES
    rhi/flow_types.h
    rhi/flow_types.cpp
    rhi/yacreader_flow_rhi.h
    rhi/yacreader_flow_rhi.cpp
    rhi/yacreader_comic_flow_rhi.h
    rhi/yacreader_comic_flow_rhi.cpp
    rhi/yacreader_page_flow_rhi.h
    rhi/yacreader_page_flow_rhi.cpp
)

add_library(rhi_flow_reader STATIC ${RHI_FLOW_SOURCES})
target_include_directories(rhi_flow_reader PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/rhi
)
target_compile_definitions(rhi_flow_reader PRIVATE YACREADER)
target_link_libraries(rhi_flow_reader PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
)
qt_add_shaders(rhi_flow_reader "flow_shaders_reader"
    BASE rhi/shaders
    PREFIX "/shaders"
    GLSL "100es,120,150"
    HLSL 50
    MSL 12
    FILES
        rhi/shaders/flow.vert
        rhi/shaders/flow.frag
)

add_library(rhi_flow_library STATIC ${RHI_FLOW_SOURCES})
target_include_directories(rhi_flow_library PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/rhi
)
target_compile_definitions(rhi_flow_library PRIVATE YACREADER_LIBRARY)
target_link_libraries(rhi_flow_library PUBLIC
    Qt::Core
    Qt::Gui
    Qt::GuiPrivate
    Qt::Widgets
)
qt_add_shaders(rhi_flow_library "flow_shaders_library"
    BASE rhi/shaders
    PREFIX "/shaders"
    GLSL "100es,120,150"
    HLSL 50
    MSL 12
    FILES
        rhi/shaders/flow.vert
        rhi/shaders/flow.frag
)

endif() # NOT BUILD_SERVER_STANDALONE
