# Qt5 Drop - Cleanup Checklist

Inventory of all Qt5 compatibility code and version branching across the project.
Goal: support only Qt6, remove all Qt5 conditionals and dead branches.

---

## 1. Build System (.pro / .pri files)

### core5compat module additions
These add `QT += core5compat` for Qt6 builds. Needed by third-party QtWebApp (QTextCodec, QRegExp).

| File | Line | Code |
|------|------|------|
| `YACReaderLibrary/YACReaderLibrary.pro` | 51 | `greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat` |
| `YACReader/YACReader.pro` | 53 | `greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat` |
| `YACReaderLibraryServer/YACReaderLibraryServer.pro` | 20 | `greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat` |
| `YACReaderLibraryServer/YACReaderLibraryServer.pro` | 40 | `greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat` (duplicate) |

**Action**: Replace conditionals with unconditional `QT += core5compat` (still needed for QtWebApp). Remove the duplicate in YACReaderLibraryServer.

### macextras module (Qt5-only, removed in Qt6)

| File | Line | Code |
|------|------|------|
| `YACReaderLibrary/YACReaderLibrary.pro` | 44 | `lessThan(QT_MAJOR_VERSION, 6): QT += macextras` |
| `YACReader/YACReader.pro` | 48 | `lessThan(QT_MAJOR_VERSION, 6): QT += macextras` |

**Action**: Remove these lines entirely.

### gui-private module (Qt 6.7+)

| File | Line | Code |
|------|------|------|
| `YACReaderLibrary/YACReaderLibrary.pro` | 53-55 | `greaterThan(QT_MAJOR_VERSION, 5):greaterThan(QT_MINOR_VERSION, 6) { QT += gui-private }` |
| `YACReader/YACReader.pro` | 55-57 | Same |

**Action**: Simplify to `greaterThan(QT_MINOR_VERSION, 6): QT += gui-private` (drop the Qt5 guard).

### Poppler backend version branching

| File | Lines | What |
|------|-------|------|
| `dependencies/pdf_backend.pri` | 60-84 | Qt5 uses poppler-qt5, Qt6 uses poppler-qt6 (pkg-config, include paths, link flags) |

**Action**: Keep only the Qt6 (poppler-qt6) branch, remove poppler-qt5 paths.

### Minimum version check

| File | Lines | What |
|------|-------|------|
| `config.pri` | 18-41 | `minQtVersion()` function, enforces minimum Qt 5.15.0 |

**Action**: Update minimum to Qt 6.x.

### QML resource branching (DONE)

| File | Lines | What |
|------|-------|------|
| `YACReaderLibrary/YACReaderLibrary.pro` | 317-321 | Was `greaterThan(QT_MAJOR_VERSION, 5) { qml6.qrc } else { qml.qrc }` |

**Action**: Already cleaned up - now unconditional `RESOURCES += qml.qrc`.

### Qt4 compatibility in third-party

| File | Line | Code |
|------|------|------|
| `third_party/QsLog/QsLog.pri` | 11 | `greaterThan(QT_MAJOR_VERSION, 4): QT += widgets` |

**Action**: Replace with unconditional `QT += widgets`.

---

## 2. C++ Source Code (#if QT_VERSION checks)

### Qt5 vs Qt6 API changes (own code)

| File | Line(s) | Qt5 branch | Qt6 branch | API change |
|------|---------|------------|------------|------------|
| `common/pdf_comic.h` | 49 | `#include "poppler-qt5.h"` | `#include <poppler-qt6.h>` | Poppler header |
| `common/pdf_comic.mm` | 3 | (nothing) | `#undef __OBJC_BOOL_IS_BOOL` | macOS ObjC compat |
| `common/comic.cpp` | 812 | `auto _pdfComic = Poppler::Document::load()` | `pdfComic = Poppler::Document::load()` | Poppler return type |
| `YACReaderLibrary/initial_comic_info_extractor.cpp` | 44 | Same Poppler pattern | Same | Poppler return type |
| `YACReader/mouse_handler.cpp` | 22, 56, 107 | `QPointF(event->x(), event->y())` | `event->position()` | QMouseEvent API |
| `YACReader/viewer.cpp` | 762 | `pixmap(Qt::ReturnByValue)` | `pixmap()` | QPixmap return semantics |
| `custom_widgets/help_about_dialog.cpp` | 14, 88 | `QTextCodec` / `setCodec()` | `QStringConverter` / `setEncoding()` | Text encoding API |
| `YACReader/translator.cpp` | 291 | `player->setMedia()` | `player->setSource()` | QMediaPlayer API |
| `YACReaderLibrary/comic_vine/comic_vine_dialog.cpp` | 149, 160 | `QtConcurrent::run(this, &fn, args)` | `QtConcurrent::run(&fn, this, args)` | Argument order |
| `YACReaderLibrary/library_window.cpp` | 173 | Different key handling | `keySequence[0]` (QKeyCombination) | Key event API |
| `YACReaderLibrary/library_window.cpp` | 790-793 | Qt5 toolbar connections | Qt6 toolbar connections | macOS toolbar |
| `YACReader/main_window_viewer.cpp` | 483, 1357 | Qt5 toolbar/slider | Qt6 toolbar/slider | macOS toolbar |
| `YACReaderLibrary/trayhandler.mm` | 3 | (nothing) | `#undef __OBJC_BOOL_IS_BOOL` | macOS ObjC compat |
| `YACReader/main.cpp` | 102 | (nothing) | `QImageReader::setAllocationLimit(0)` | Image reader limit |
| `YACReaderLibraryServer/main.cpp` | 53 | (nothing) | `QImageReader::setAllocationLimit(0)` | Image reader limit |

**Action**: For each, keep only the Qt6 branch, remove the `#if`/`#else`/`#endif` and the Qt5 code.

### Qt4 vs Qt5 leftovers (very old)

| File | Line(s) | What |
|------|---------|------|
| `custom_widgets/yacreader_table_view.cpp` | 31, 39, 46 | `#if QT_VERSION >= 0x050000` — `setResizeMode` vs `setSectionResizeMode` |
| `YACReaderLibrary/comic_vine/scraper_tableview.cpp` | 9, 16, 23 | Same Qt4 vs Qt5 header API |
| `YACReader/translator.cpp` | 3, 164 | `#if QT_VERSION >= 0x050000` — Phonon vs QMediaPlayer |
| `YACReader/main.cpp` | 135 | `#if QT_VERSION >= 0x050800` — QCommandLineOption::HiddenFromHelp |

**Action**: Remove the `#if` guards entirely, keep only the Qt5+ code (which is also valid Qt6).

### Qt 6.7+ specific (RHI widget)

| File | Lines | What |
|------|-------|------|
| `common/rhi/yacreader_flow_rhi.h` | 8-302 | `#if QT_VERSION >= QT_VERSION_CHECK(6, 7, 0)` — entire QRhiWidget implementation |

**Action**: Keep this conditional (gates on Qt 6.7 minor version, not Qt5 vs Qt6).

### Qt 6.9+ specific (test code)

| File | Line | What |
|------|------|------|
| `tests/concurrent_queue_test/concurrent_queue_test.cpp` | 212 | `#if QT_VERSION < QT_VERSION_CHECK(6, 9, 0)` — QDebug for std::array |

**Action**: Keep this conditional (Qt6 minor version gate).

---

## 3. Third-Party Libraries

### QtWebApp (`third_party/QtWebApp/`)

| File | Lines | What |
|------|-------|------|
| `templateengine/templateengine.pri` | 5 | `QT += core5compat` |
| `templateengine/templateloader.cpp` | 13, 77, 86, 107 | QRegExp→QRegularExpression, QString::split flag |
| `httpserver/httpconnectionhandler.h` | 23 | `tSocketDescriptor` typedef (Qt4 vs Qt5) |
| `httpserver/httpconnectionhandlerpool.cpp` | 152 | SSL cert handling (Qt <5.15 vs 5.15+) |

**Action**: Decide whether to update QtWebApp or replace it. If updating: remove Qt4/Qt5 branches, keep Qt6 code, potentially drop core5compat dependency by migrating QTextCodec→QStringConverter and QRegExp→QRegularExpression.

### QsLog (`third_party/QsLog/`)

| File | Lines | What |
|------|-------|------|
| `QsLog.pri` | 11 | Qt4 widgets guard |
| `QsLogDestFile.cpp` | 32, 157, 179 | QTextCodec vs QStringConverter |
| `QsLogWindow.cpp` | 129 | Qt4 header resize API |

**Action**: Same approach — remove Qt4/Qt5 branches, keep Qt6 code.

---

## 4. CI/CD and Build Scripts

### GitHub Actions (`.github/workflows/build.yml`)

**Qt5 build targets to remove:**
- Ubuntu Linux Qt5 build (~line 63)
- Windows x64 Qt5 build (~line 346)
- Windows x86 Qt5 build (~line 798)
- macOS Qt5 build (~line 201)

**Qt6 build targets to keep (and simplify):**
- Linux Qt6 builds (~lines 90, 116)
- macOS Qt6 Universal (~line 145) — still installs `qt5compat` module (needed for QtWebApp)
- Windows x64 Qt6 (~line 408) — same
- Windows ARM64 Qt6 (~line 574) — same

### Build scripts

| File | What |
|------|------|
| `build_scripts/ubuntu_24.04/build.sh` | Qt6 — installs `libqt6core5compat6-dev`, `qml6-module-qt5compat-graphicaleffects` |
| `build_scripts/ubuntu_22.04/build.sh` | Qt5 — entire script is Qt5-only |

**Action**: Remove `ubuntu_22.04` build script. In `ubuntu_24.04`, remove `qml6-module-qt5compat-graphicaleffects` (no longer used in QML). Keep `libqt6core5compat6-dev` (needed for QtWebApp).

### Docker

| File | What |
|------|------|
| `docker/Dockerfile` | Ubuntu x64 Qt6 — installs `libqt6core5compat6-dev` (build) and `libqt6core5compat6` (runtime) |
| `docker/Dockerfile.aarch64` | Ubuntu ARM64 Qt5-only build |

**Action**: Convert `Dockerfile.aarch64` to Qt6. Keep core5compat packages in main Dockerfile (QtWebApp). Make sure the conversion is good, there must be other images online that use the arm+qt6 combo, we can search online.

---

## 5. Summary

### Safe to remove now (no dependencies)
- All `lessThan(QT_MAJOR_VERSION, 6)` lines (macextras)
- All `#if QT_VERSION >= 0x050000` / `0x050800` guards (Qt4 leftovers)
- All Qt5 CI build targets
- `build_scripts/ubuntu_22.04/` (Qt5-only)
- `qml6-module-qt5compat-graphicaleffects` from build scripts (QML migration done)

### Requires code changes (keep Qt6 branch only)
- ~15 `#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)` blocks in own code
- Poppler backend selection in `pdf_backend.pri`
- `config.pri` minimum version bump

### Keep as-is (Qt6 minor version gates)
- `QT_VERSION_CHECK(6, 7, 0)` — RHI widget
- `QT_VERSION_CHECK(6, 9, 0)` — test code
- `gui-private` for Qt 6.7+ (simplify guard only)

### Depends on third-party decisions
- `core5compat` — still needed unless QtWebApp and QsLog are updated to drop QTextCodec/QRegExp
