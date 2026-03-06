# AGENTS GUIDANCE

This file provides guidance to AI agents when working with code in this repository.

## Build

Out-of-source builds are required. In-source builds will be rejected by CMake.

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

Build options:
- `DECOMPRESSION_BACKEND`: `unarr` | `7zip` | `libarchive` (default: 7zip on Windows/macOS, unarr on Linux)
- `PDF_BACKEND`: `pdfium` | `poppler` | `pdfkit` | `no_pdf` (default: pdfium on Windows, pdfkit on macOS, poppler on Linux)
- `BUILD_SERVER_STANDALONE=ON`: builds only `YACReaderLibraryServer` (headless), requires only Qt 6.4+
- `BUILD_TESTS=ON` (default): enables the test suite

## Tests

```bash
ctest --test-dir build --output-on-failure
```

Tests live in `tests/` and are built as Qt Test executables (`compressed_archive_test`, `concurrent_queue_test`).

## Code Formatting

CI enforces `clang-format`. Run it before committing. There are multiple `.clang-format` files — subdirectories for third-party code have their own to opt out of reformatting. Always run recursively from the repo root via the provided scripts:

- Linux: `scripts/clang-format-linux.sh`
- macOS: `scripts/clang-format-macos.sh`
- Windows: `scripts\clang-format-windows.cmd` (or `.ps1`)

Style is WebKit-based with custom brace wrapping (braces on same line for control flow, new line after functions/classes), no column limit, and `SortIncludes: false`.

## Architecture

The repo builds three applications that share a common set of static libraries:

| App | Description |
|-----|-------------|
| `YACReader` | Comic viewer |
| `YACReaderLibrary` | Comic library manager (GUI) |
| `YACReaderLibraryServer` | Headless HTTP server |

### Static library dependency layers (bottom to top)

1. **`yr_global`** — version/global constants, no GUI, used by everything
2. **`naturalsort`, `concurrent_queue`, `worker`** — utilities
3. **`common_all`** — shared non-GUI: `ComicDB`, `Folder`, `Bookmarks`, HTTP helpers, cover utils
4. **`comic_backend`** — comic file abstraction + PDF backend (source varies by `PDF_BACKEND`)
5. **`cbx_backend`** — compressed archive abstraction (in `compressed_archive/`)
6. **`db_helper`** — SQLite database layer: schema management, reading lists, query parser
7. **`library_common`** — library scanning, bundle creation, XML metadata parsing; shared between `YACReaderLibrary` and `YACReaderLibraryServer`
8. **`common_gui`** — GUI widgets, themes infrastructure, version check (not built in `BUILD_SERVER_STANDALONE`)
9. **`rhi_flow_reader` / `rhi_flow_library`** — RHI-based 3D coverflow widget, compiled twice with different defines (`YACREADER` vs `YACREADER_LIBRARY`); shaders compiled via `qt_add_shaders()`

### Key design notes

- **Theme system**: `theme_manager.h/cpp` is NOT part of `common_gui` because it depends on app-specific `Theme` structs. Each app (`YACReader/themes/`, `YACReaderLibrary/themes/`) defines its own `theme.h` and includes `theme_manager` directly.
- **Compile-time app identity**: `YACREADER` or `YACREADER_LIBRARY` defines distinguish shared source compiled into different apps.
- **PDF backend**: resolved at configure time into an `INTERFACE` target `pdf_backend_iface` (see `cmake/PdfBackend.cmake`); `comic_backend` and `common_all` link against this interface.
- **Third-party code**: `third_party/` contains QsLog, KDToolBox, QtWebApp, QrCode — each has its own `.clang-format` to prevent reformatting.
- **Runtime dependencies**: Qt binaries must be in `PATH`; third-party DLLs/dylibs must be next to the executable. Check an existing YACReader installation for the required files.

### PRs

Target branch is always `develop`.
