add_library(cbx_backend)

if(DECOMPRESSION_BACKEND STREQUAL "unarr")

  target_sources(
    cbx_backend PRIVATE unarr/compressed_archive.cpp unarr/compressed_archive.h
                        unarr/extract_delegate.h)
  target_include_directories(cbx_backend PUBLIC unarr)
  target_compile_definitions(cbx_backend PUBLIC use_unarr)
  if(TARGET unarr:unarr)
    target_link_libraries(cbx_backend unarr::unarr Qt::Core)
  else()
    target_link_libraries(cbx_backend PkgConfig::unarr Qt::Core)
  endif()

elseif(DECOMPRESSION_BACKEND STREQUAL "libarchive")

  target_sources(
    cbx_backend
    PRIVATE libarchive/compressed_archive.cpp libarchive/compressed_archive.h
            libarchive/extract_delegate.h)
  target_include_directories(cbx_backend PUBLIC libarchive)
  target_compile_definitions(cbx_backend PUBLIC use_libarchive)
  target_link_libraries(cbx_backend LibArchive::LibArchive Qt::Core)

elseif(DECOMPRESSION_BACKEND STREQUAL "7zip")
  # TODO
  target_sources(cbx_backend PRIVATE compressed_archive.cpp
                                     compressed_archive.h extract_delegate.h)
  target_include_directories(cbx_backend PUBLIC .)
  target_link_libraries(cbx_backend 7zip Qt::Core)

endif()
