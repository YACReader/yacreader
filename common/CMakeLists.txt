# Common for all targets
add_library(
  common_all
  bookmarks.cpp
  bookmarks.h
  comic_db.cpp
  comic_db.h
  http_worker.cpp
  http_worker.h
  library_item.cpp
  library_item.h)
target_link_libraries(common_all PUBLIC Qt::Gui Qt::Network)
target_include_directories(common_all PUBLIC .)

# Common for gui targets
add_library(common_gui exit_check.cpp exit_check.h opengl_checker.cpp
                       opengl_checker.h)
target_link_libraries(common_gui Qt::Widgets QsLog)
target_include_directories(common_gui PUBLIC .)

add_library(version_checker check_new_version.cpp check_new_version.h)
target_link_libraries(version_checker Qt::Network Qt::Core5Compat)

add_library(worker release_acquire_atomic.h worker_thread.h)
target_link_libraries(worker Qt::Core)

add_library(comic_backend comic.h comic.cpp folder.h folder.cpp pdf_comic.h
                          pdf_comic.cpp)
target_link_libraries(comic_backend cbx_backend Qt::Gui QsLog)
target_include_directories(comic_backend PUBLIC .)

if(PDF_BACKEND STREQUAL "pdfium")
  target_compile_definitions(comic_backend PUBLIC USE_PDFIUM)
  target_link_libraries(comic_backend PkgConfig::PDFIUM)
elseif(PDF_BACKEND STREQUAL "poppler")
  target_link_libraries(comic_backend PkgConfig::POPPLER)
elseif(APPLE AND (PDF_BACKEND STREQUAL "pdfkit"))
  # TODO
else()
  target_compile_definitions(comic_backend PUBLIC NO_PDF)
endif()

add_library(yr_global yacreader_global.h yacreader_global.cpp
                      yacreader_global_gui.cpp yacreader_global_gui.h)
target_link_libraries(yr_global Qt::Widgets)
target_include_directories(yr_global PUBLIC .)

add_library(concurrent_queue concurrent_queue.h concurrent_queue.cpp)
target_link_libraries(concurrent_queue Qt::Core)
target_include_directories(concurrent_queue PUBLIC .)

add_library(naturalsort qnaturalsorting.h qnaturalsorting.cpp)
target_include_directories(naturalsort PUBLIC .)
target_link_libraries(naturalsort Qt::Core)

# Flow
add_library(pictureflow pictureflow.h pictureflow.cpp scroll_management.cpp
                        scroll_management.h)
target_link_libraries(pictureflow Qt::Widgets)
target_include_directories(pictureflow PUBLIC .)

# GL flow is slightly different for reader and library, so we need to build it
# twice
add_library(gl_flow_reader gl/yacreader_flow_gl.cpp gl/yacreader_flow_gl.h)
target_link_libraries(gl_flow_reader pictureflow Qt::OpenGLWidgets Qt::Gui)
target_include_directories(gl_flow_reader PUBLIC gl)
target_compile_definitions(gl_flow_reader PRIVATE YACREADER)

add_library(gl_flow_library gl/yacreader_flow_gl.cpp gl/yacreader_flow_gl.h)
target_link_libraries(gl_flow_library pictureflow Qt::OpenGLWidgets Qt::Gui)
target_include_directories(gl_flow_library PUBLIC gl)
target_compile_definitions(gl_flow_library PRIVATE YACREADER_LIBRARY)
