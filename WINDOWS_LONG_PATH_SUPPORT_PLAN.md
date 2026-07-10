# Windows Long Path Support Plan

## Goal

Support Windows paths longer than the traditional `MAX_PATH` limit for the main YACReader workflows:

- Opening comics and folders in `YACReader`.
- Creating, updating, and browsing libraries in `YACReaderLibrary`.
- Running `YACReaderLibraryServer` against libraries stored in long paths.
- Reading archives, PDFs, covers, and SQLite library databases under long paths.
- Copying, moving, deleting, importing, exporting, and opening containing folders from the GUI.

This should not leak Windows extended-length path prefixes into user-visible UI, saved library paths, recent files, database rows, HTTP routes, or cross-platform code unless a specific low-level boundary requires it.

## Current Assessment

This is not a single CMake or manifest fix. The codebase mostly uses Qt path APIs, which is good, but several edges leave Qt and can still hit `MAX_PATH` or encoding problems.

The safest approach is:

1. Make the Windows executables long-path aware.
2. Keep normal `QString` paths internally.
3. Add narrowly-scoped Windows path helpers for native/third-party boundaries.
4. Fix or verify every boundary where paths are passed to Win32, C libraries, SQLite, external tools, or shell/file URL APIs.
5. Add Windows regression coverage around real paths longer than 260 characters.

## Known System Requirements

Windows long path support generally requires both:

- An application manifest with `longPathAware` enabled.
- The machine policy/registry setting that enables Win32 long paths (`HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled = 1`, available since Windows 10 1607).

Even with both enabled, only the Unicode (`*W`) variants of most Win32 file APIs honor the long-path opt-in. Notable APIs that still effectively cap at `MAX_PATH` regardless of manifest+policy:

- `ShellExecute`/`ShellExecuteEx` and most of `shell32` for shell verbs.
- Explorer `/select,` invocations (the shell process must itself decide to handle the path).
- Older `*A` ANSI APIs.
- Third-party executables linked without the long-path manifest (including the bundled 7z executable).

Implication: a manifest fixes most of the in-process code paths automatically, but anything that hands a path to the shell or to an external process still requires either a `\\?\` prefix at the boundary, a fallback path, or graceful degradation with a clear error. The app should document the OS setting in release notes or troubleshooting docs and handle failure gracefully when the OS setting is not enabled.

## Affected Areas Found

### Windows Executable Manifests

The Windows targets currently add icon resource files, but no app manifest was found. Current state:

- [YACReader/icon.rc](YACReader/icon.rc) is one line: `IDI_ICON1 ICON DISCARDABLE "icon.ico"`.
- [YACReaderLibrary/icon.rc](YACReaderLibrary/icon.rc) is the same shape.
- [YACReaderLibraryServer/CMakeLists.txt](YACReaderLibraryServer/CMakeLists.txt) does not reference any `.rc` file at all.

Affected targets:

- `YACReader` in [YACReader/CMakeLists.txt](YACReader/CMakeLists.txt).
- `YACReaderLibrary` in [YACReaderLibrary/CMakeLists.txt](YACReaderLibrary/CMakeLists.txt).
- `YACReaderLibraryServer` in [YACReaderLibraryServer/CMakeLists.txt](YACReaderLibraryServer/CMakeLists.txt).

Plan:

- Add a single shared `yacreader.manifest` next to the existing icon resources (e.g., `cmake/windows/yacreader.manifest`) containing:

```xml
<?xml version="1.0" encoding="UTF-8" standalone="yes"?>
<assembly xmlns="urn:schemas-microsoft-com:asm.v1" manifestVersion="1.0">
  <application xmlns="urn:schemas-microsoft-com:asm.v3">
    <windowsSettings>
      <longPathAware xmlns="http://schemas.microsoft.com/SMI/2016/WindowsSettings">true</longPathAware>
    </windowsSettings>
  </application>
</assembly>
```

- Embed via the existing `.rc` files using `CREATEPROCESS_MANIFEST_RESOURCE_ID` so we do not depend on CMake's auto-manifest behavior:

```rc
#include <winuser.h>
IDI_ICON1                          ICON  DISCARDABLE "icon.ico"
CREATEPROCESS_MANIFEST_RESOURCE_ID MANIFEST          "yacreader.manifest"
```

- Add a new `icon.rc` (or `app.rc`) for `YACReaderLibraryServer` and wire it into its `target_sources` for `WIN32`. The server can reuse the same shared `.manifest` and the same `icon.ico` if desired, or ship without an icon.
- Keep icon resources intact for the two GUI apps.
- Do not pass `MANIFESTUAC`/`requireAdministrator`-style settings; we only want the long-path opt-in (and optionally `dpiAware`, `activeCodePage=UTF-8`, but those are out of scope for this plan).

### Archive Opening

This is the highest-risk area because comics are normally archives.

#### 7zip backend

[compressed_archive/compressed_archive.cpp](compressed_archive/compressed_archive.cpp) opens archives through 7zip `CInFileStream`.

The key boundary at [compressed_archive.cpp:135-139](compressed_archive/compressed_archive.cpp#L135-L139) is:

```cpp
#ifdef USE_UNICODE_FSTRING
        if (!fileSpec->Open((LPCTSTR)filePath.toStdWString().c_str()))
#else
        if (!fileSpec->Open((LPCTSTR)filePath.toStdString().c_str()))
#endif
```

Verified state:

- `USE_UNICODE_FSTRING` is referenced **only** in this one file. It is never `#define`d in any header or set by CMake. The narrow branch is what compiles today on Windows.
- The bundled lib7zip source already wraps every `CreateFileW` / `FindFirstFileW` call in [CPP/Windows/FileIO.cpp](compressed_archive/lib7zip/CPP/Windows/FileIO.cpp), [CPP/Windows/FileDir.cpp](compressed_archive/lib7zip/CPP/Windows/FileDir.cpp), and [CPP/Windows/FileFind.cpp](compressed_archive/lib7zip/CPP/Windows/FileFind.cpp) with `#ifdef Z7_LONG_PATH` retry-with-superpath blocks (e.g. `if (GetSuperPath(path, superPath, USE_MAIN_PATH)) _handle = ::CreateFileW(superPath, ...)`). `Z7_LONG_PATH` is **not** currently defined for the build either.

Plan:

- Define `USE_UNICODE_FSTRING` for the YACReader / YACReaderLibrary targets on Windows so the wide-string branch compiles. Add it as a target-scoped compile definition; do not change cross-platform behavior.
- Define `Z7_LONG_PATH` for the lib7zip compile on Windows so the upstream long-path retry path is enabled. This is the correct long-path mechanism for this backend; do **not** add a parallel YACReader-side `\\?\` prefix helper for the 7zip boundary, which would duplicate work that lib7zip already does correctly (and locally — at the failing `CreateFileW` call, with proper UNC handling).
- After both are enabled, run smoke tests with `.cbz`, `.cbr`, `.7z`, and `.tar` archives at absolute paths > 260 characters and at > 32 760 characters (to confirm the Win32 cap is the only remaining limit).
- Only if the upstream retry still fails for a specific call site should we add a YACReader-side prefix; in practice this is unlikely.

Out of scope: 7zip handles archive **member names** internally. We do not need to think about long entry names (they are not Win32 paths until extracted, and we extract to memory).

#### unarr backend

[compressed_archive/unarr/compressed_archive.cpp:14-18](compressed_archive/unarr/compressed_archive.cpp#L14-L18) already uses `ar_open_file_w((wchar_t *)filePath.utf16())` on Windows.

Verified state:

- unarr is shipped on Windows as a **prebuilt DLL plus header** under [dependencies/unarr/win/](dependencies/unarr/win/). There are no `.c` sources for unarr in this tree, so we cannot audit or recompile its internal `CreateFileW`/`_wfopen` usage without rebuilding the DLL from upstream.
- This means the only practical levers we have are:
  1. Trust that unarr's wide opener honors the manifest+policy (likely, since it uses `*W` APIs).
  2. Convert the path to `\\?\` form at the `ar_open_file_w` boundary if (1) fails.

Plan:

- Test unarr with paths over 260 characters once the manifest is in place.
- If it fails, apply the boundary helper (see Phase 1) only to the argument passed to `ar_open_file_w`. The result of the helper must not leak back into `filePath` or any caller-visible state.
- Do not block this work on rebuilding the bundled DLL. Document the bundled unarr version in release notes so future upstream bumps are tracked.

#### libarchive backend

`compressed_archive/libarchive/compressed_archive.cpp` uses:

```cpp
archive_read_open_filename(a, filename.toStdString().c_str(), 10240)
```

Risk:

- Not currently supported on Windows, but this would not be safe if Windows support is added.

Plan:

- If libarchive becomes supported on Windows, use a wide Windows opener if available or provide custom callbacks backed by `QFile`.

### PDF Opening

PDFium currently opens PDFs through `QFile` and `FPDF_LoadCustomDocument`, so the file path stays in Qt:

- `common/pdf_comic.cpp`

Risk:

- This is probably fine once the executable is long-path aware.
- Poppler/PDFKit paths should still be smoke-tested by platform.

Plan:

- Include long-path PDF files in manual or automated Windows smoke tests.

### Library Scanning and Folder Comics

Recursive library and folder-comic scanning mostly use `QDir`, `QFileInfo`, and `QFile`.

Relevant paths:

- `common/comic.cpp`: recursive discovery in `Comic::findValidComicFilesInFolder`.
- `common/comic.cpp`: folder-comic image loading through `QFile`.
- `YACReaderLibrary/library_creator.cpp`: library create/update traversal, hashing, cover extraction.

Risk:

- These are likely OK with a manifest and Qt support.
- Recursion uses normal paths, so do not introduce `\\?\` prefixes into stored library paths.
- `QFileInfo::canonicalPath()` and similar calls can behave differently when symlinks or inaccessible long paths are involved, so the update scanner needs test coverage.

Plan:

- Add tests or smoke scripts for:
  - Creating a new library under a long root.
  - Updating an existing library.
  - Opening a folder comic under a long root.
  - Computing pseudo-hashes from long archive paths.

### SQLite Library Databases

SQLite database paths are set through Qt SQL in [YACReaderLibrary/db/data_base_management.cpp](YACReaderLibrary/db/data_base_management.cpp):

- `createDatabase`
- `loadDatabase`
- `loadDatabaseFromFile`

Risk:

- `QSQLITE` should normally handle a `QString` database name, but it should be verified with `library.ydb` stored below a long path. Internally Qt's SQLite driver opens the file with `sqlite3_open_v2`, which on Windows uses `CreateFileW` — so a manifest-aware process should already work.
- `exportComicsInfo` at [data_base_management.cpp:506-510](YACReaderLibrary/db/data_base_management.cpp#L506-L510) builds raw `ATTACH DATABASE` SQL by string-concatenating the path into the SQL text:

  ```cpp
  attach.prepare("ATTACH DATABASE '" + QDir().toNativeSeparators(dest) + "' AS dest;");
  ```

  This is broken for any path containing a single quote, regardless of length. It is a standalone correctness/injection issue — long paths are not the trigger, they just make it more likely a user hits a path the user constructed via the GUI. Treat this as a separate fix that this work happens to touch.

Plan:

- Smoke-test database create/load/update with long library data paths.
- Fix the `ATTACH DATABASE` site as a discrete change. Options, in order of preference:
  1. Use `QSqlQuery::addBindValue` if Qt's SQLite driver supports binding the database filename in `ATTACH DATABASE` (it does in modern SQLite via parameter substitution: `ATTACH DATABASE ? AS dest`).
  2. If binding is rejected by the driver, escape single quotes by doubling them (`path.replace('\'', "''")`) before concatenation, and add a regression test with a path containing `'`.
- Include import/export comics info in the test matrix with both a long path and a path containing `'`.

### External 7z Package Import/Export

`YACReaderLibrary/package_manager.cpp` launches an external 7z process for `.clc` packages:

- `createPackage`: passes destination `.clc` path and library path.
- `extractPackage`: passes output directory and package path.

Risk:

- External 7z may or may not support long paths depending on the bundled executable/version and argument format.
- `-o<destDir>` combines the option and path in one argument. That is valid for 7z, but long paths should be tested carefully.
- If the external tool needs extended-length paths, the prefix should be applied only to the arguments passed to 7z, not to stored app paths.

Plan:

- Verify bundled `utils/7zip` on Windows with long package paths, long output paths, and long library roots.
- If it fails, convert affected process arguments at the process boundary.
- Consider replacing this external-process path with the in-process 7zip library in a later cleanup, as the existing TODO suggests.

### Copy, Move, Delete, and Trash

Relevant paths:

- `YACReaderLibrary/comic_files_manager.cpp`
  - Drag/drop path extraction through `QUrl::toLocalFile`.
  - `QDir().mkpath`.
  - `QFile::copy`.
  - `QFile::remove`.
- `YACReaderLibrary/comics_remover.cpp`
  - `QFile::moveToTrash`.
  - `QFile::remove`.
  - `QDir::removeRecursively`.
- `YACReaderLibrary/library_window.cpp`
  - Deleting library data through `QDir::removeRecursively`.
  - Saving selected covers through `QFile::copy`.

Risk:

- Qt filesystem operations should be mostly OK once the process is long-path aware.
- Trash integration can be more OS/shell dependent than direct remove.
- Drag/drop `QUrl` handling needs testing with long local file URLs.

Plan:

- Test copy, move, delete-to-trash, fallback delete, recursive folder delete, and save-covers workflows under long paths.
- If `moveToTrash` fails with long paths, keep fallback direct delete behavior and improve user-facing error reporting.

### Shell and File URL Integration

Relevant paths:

- [YACReaderLibrary/library_window.cpp:1503](YACReaderLibrary/library_window.cpp#L1503) — `QDesktopServices::openUrl(QUrl("file:///" + ...))` on the parent folder.
- [YACReaderLibrary/library_window.cpp:1639](YACReaderLibrary/library_window.cpp#L1639) — `comic.data(...).toString().remove("file:///").remove("file:")` — string-based URL stripping to recover a local path. Fragile for the same reasons manual URL construction is.
- [YACReaderLibrary/library_window.cpp:2192](YACReaderLibrary/library_window.cpp#L2192) — Linux/X11 "open containing folder" via manual `file:///`.
- [YACReaderLibrary/library_window.cpp:2224](YACReaderLibrary/library_window.cpp#L2224) — open folder via manual `file:///`.
- `library_window.cpp` also calls `ShellExecuteW` with `explorer.exe /select,"path"` for Windows "open containing folder, select file".
- [YACReaderLibrary/library_comic_opener.cpp](YACReaderLibrary/library_comic_opener.cpp) — `QProcess::startDetached` for launching `YACReader.exe`, plus third-party reader command placeholder substitution.

Risk:

- Manually constructing `file:///` URLs is fragile for spaces, non-ASCII, `#`, `%`, UNC paths, and very long paths. Stripping URL prefixes via `.remove(...)` has the inverse problem (it does not percent-decode).
- Explorer `/select,` is known to be sensitive: even with the long-path manifest+policy, the **Explorer process** must itself be long-path aware to honor the selected path. In practice it is not reliably so.
- `ShellExecuteW` is the wide variant but the shell verb dispatch (`open`, `explore`) routes through shell handlers that may not respect the long-path policy.
- Third-party apps may not support long paths; this is not fully controllable by YACReader.

Plan:

- Replace **all** manual `file:///` construction with `QUrl::fromLocalFile(path)`. This is a net code-quality win independent of long paths.
- Replace the [library_window.cpp:1639](YACReaderLibrary/library_window.cpp#L1639) string-strip pattern with `QUrl(roleString).toLocalFile()` (or change the role to expose a plain path in the first place).
- For "open containing folder, select file" on Windows, prefer the COM API `SHOpenFolderAndSelectItems` over `explorer.exe /select,`. It accepts a `PIDL` from `SHParseDisplayName`, which works with long paths when the shell namespace cooperates. If both fail, fall back to opening the parent folder with `QDesktopServices::openUrl(QUrl::fromLocalFile(parentPath))` and surface a non-fatal warning.
- Keep `QProcess::startDetached` arguments as plain `QString`s first; test the Library → Reader handoff with a long-path comic. If `YACReader.exe` cannot open the path, the issue is in `YACReader.exe` itself, not the IPC.
- Document that third-party reader support depends on the external application; do not silently rewrite arguments to `\\?\` form because most third-party readers will reject that prefix.

### Server Path Routing

Relevant paths:

- `YACReaderLibrary/server/requestmapper.cpp`
- `YACReaderLibrary/server/controllers/v2/*`

Many controllers decode a request path using:

```cpp
QString path = QUrl::fromPercentEncoding(request.getPath()).toUtf8();
```

Risk:

- These are relative library paths, not native absolute paths, so `MAX_PATH` is not directly the issue.
- However, long relative paths and non-ASCII names need round-trip testing through HTTP percent encoding, database lookup, and final filesystem access.
- The `.toUtf8()` conversion back into a `QString` deserves a separate cleanup; it is not necessarily a long-path bug, but it is suspicious.

Plan:

- Add server smoke tests for long nested relative paths and non-ASCII filenames.
- Avoid native Windows extended prefixes in HTTP routes.
- Consider cleaning up decode code to keep explicit `QString` handling.

### Logging and Console Output

Several places convert paths for logs or console output:

- `YACReaderLibraryServer/console_ui_library_creator.cpp`
- `YACReader/main.cpp`
- `YACReaderLibrary/main.cpp`
- `YACReaderLibraryServer/main.cpp`

Risk:

- These conversions should not block file access, but they can make debugging long/non-ASCII paths harder.

Plan:

- Leave as low priority unless logs corrupt important diagnostics.
- Prefer Qt logging of `QString` paths where practical.

## Path-Length Asymmetry in Real Libraries

A YACReader library on disk has a strongly asymmetric path-length distribution:

- **Library root** (e.g. `C:\Users\me\Documents\mycomics\`) — almost always short.
- **`.yacreaderlibrary` metadata folder** lives directly under the root, so `library.ydb`, the cover cache, and DB `ATTACH` source/dest paths inherit a short prefix. Their absolute paths almost never exceed `MAX_PATH`.
- **Package destinations and shell "open folder" targets** — usually point at user-chosen locations that are also short (Desktop, Documents, library root).
- **Comic files themselves (the library leaves)** — live at deep nested paths like `C:\Users\me\Documents\mycomics\Marvel\Spider-Man\Volume 1 (1963)\The Amazing Spider-Man v1 #234 - The Final Showdown - Date Foo More Foo Blah.cbz`. **These are the paths that blow past `MAX_PATH` in real-world use.**

Implication: a small subset of the work above — covering only the leaf-file read path — delivers most of the user-visible value. DB attach quoting, shell "select in Explorer", external 7z package import/export, and `.yacreaderlibrary` metadata I/O can stay in the broader plan but are not on the critical path for "users with long-named comics can finally open them."

This reframes the work as two scopes:

- **Minimal scope (Phase 0):** make leaf comic files openable end-to-end, from library scan through reader display.
- **Full scope (Phases 1–5):** harden every other path-handling boundary so corner cases (rename into long path, package import/export at long destinations, shell integration, server routes, etc.) all work too.

The minimal scope is recommended as a separate, ship-first deliverable.

## Proposed Implementation Phases

### Phase 0: Minimal Leaf-File Long-Path Support (recommended ship-first scope)

**Goal:** A library rooted at a short path can contain comic files whose absolute path exceeds 260 characters, and those comics can be scanned, opened, and read. Nothing else has to work yet.

Changes:

1. Embed a `longPathAware` manifest in `YACReader.exe` and `YACReaderLibrary.exe` (as described under "Windows Executable Manifests"). Skip `YACReaderLibraryServer` in this phase — it is not on the read path for the GUI workflow.
2. Define `USE_UNICODE_FSTRING` for the Windows `YACReader` and `YACReaderLibrary` targets so [compressed_archive.cpp:135](compressed_archive/compressed_archive.cpp#L135) takes the wide-string branch.
3. Define `Z7_LONG_PATH` for the lib7zip Windows compile so its existing superpath-retry logic activates.
4. Smoke-test the unarr backend with a long-path archive. If it fails, prefix only the argument passed to `ar_open_file_w` at [unarr/compressed_archive.cpp:15](compressed_archive/unarr/compressed_archive.cpp#L15) using the Phase 1 boundary helper. Do not modify any other unarr call site.
5. Smoke-test PDFium with a long-path PDF. It uses `QFile` already, so the manifest alone is expected to be sufficient; if not, the fix is contained to [common/pdf_comic.cpp](common/pdf_comic.cpp).

What is **explicitly deferred** from Phase 0:

- `ATTACH DATABASE` quoting fix — independent correctness bug, can ship separately at any time.
- All `file:///` URL cleanup and the [library_window.cpp:1639](YACReaderLibrary/library_window.cpp#L1639) string-strip pattern — code quality, no user-visible impact for short library roots.
- "Open containing folder, select file" via `SHOpenFolderAndSelectItems` — works fine today on short library roots; the failure mode (user clicks "open folder" on a deeply-nested comic) is rare and gracefully degrades to "folder didn't open".
- External 7z package import/export — affects `.clc` workflow only, infrequent.
- Server long-path routes — server is not used by the leaf-file GUI workflow.
- Library rename/move flows that could turn a short path into a long one — uncommon.
- Cover save / drag-out flows.
- `YACReaderLibraryServer` manifest and `.rc`.

Phase 0 acceptance:

- A `.cbz`, `.cbr`, and `.7z` file at an absolute path > 260 chars opens in `YACReader` from a fresh Library scan.
- The library scanner enumerates and ingests those files without skipping them.
- Reader → next/prev page works for long-path archives across both 7zip and unarr backends.
- A `.pdf` at a long path opens.
- No `\\?\` prefixes appear in the library DB rows or in any user-visible UI.
- Manifest is verifiable in the linked `.exe` via `mt.exe`.

Estimated change footprint: 1 manifest file, 2 `.rc` edits, 2 CMake `target_compile_definitions` lines, plus the unarr boundary fallback if needed. No source edits beyond conditional compilation.

If Phase 0 ships independently, Phases 1–5 below subsume the Phase 0 changes and add the broader hardening. The Phase 0 manifest mechanics, `USE_UNICODE_FSTRING`, and `Z7_LONG_PATH` definitions carry forward unchanged.

### Phase 1: Manifest and Boundary Helper

If Phase 0 has already shipped, Phase 1 only needs to **extend** the manifest coverage; the helper work is the new piece.

- Add the `longPathAware` manifest (see "Windows Executable Manifests" above for exact CMake/`.rc` mechanics) to the targets not already covered. After Phase 0 this means adding a `.rc` for `YACReaderLibraryServer` and wiring it into its `target_sources` for `WIN32`.
- If Phase 0 was skipped, embed the manifest for all three executables now (`YACReader`, `YACReaderLibrary`, `YACReaderLibraryServer`).
- Verify the manifest is actually present in each linked binary (`mt.exe -inputresource:YACReader.exe;#1 -out:check.manifest`, or open in a resource viewer). CMake's auto-manifest generation can silently override an embedded manifest in some toolchains; the explicit `CREATEPROCESS_MANIFEST_RESOURCE_ID` form avoids that.
- Add a Windows-only helper for native boundary conversion, scoped narrowly:
  - Normal absolute drive path: `C:\...` → `\\?\C:\...`
  - UNC path: `\\server\share\...` → `\\?\UNC\server\share\...`
  - Already-prefixed path: unchanged
  - Relative paths, URLs, Qt resources: rejected with a clear assertion in debug, returned unchanged in release
- Forbid the helper from being used by callers that store, log, or display the result. Document this in the helper's header.
- The helper is a **fallback**, not a default. Most boundaries (Qt file APIs, lib7zip with `Z7_LONG_PATH`, unarr's wide opener) are expected to work without it once the manifest is in place. Use only where verification proves it is needed.

### Phase 2: Archive Backend Fixes

If Phase 0 has shipped, steps 1–4 are already done; this phase reduces to step 5 (libarchive guard) plus deeper test coverage. If Phase 0 was skipped, run the full sequence:

1. Define `USE_UNICODE_FSTRING` for the Windows YACReader and YACReaderLibrary targets (target-scoped compile definition; not a global header `#define`). This is the prerequisite for everything else in this phase.
2. Define `Z7_LONG_PATH` for the lib7zip compile on Windows. This activates the upstream long-path retry logic that already exists in `compressed_archive/lib7zip/CPP/Windows/`.
3. Smoke-test the 7zip backend with long-path archives. Expect this to be sufficient.
4. Smoke-test the unarr backend with long-path archives. If it fails, apply the Phase 1 boundary helper at the `ar_open_file_w` call only.
5. Leave libarchive Windows support explicitly unsupported. Add a CMake-level guard or comment at [compressed_archive/libarchive/compressed_archive.cpp:54](compressed_archive/libarchive/compressed_archive.cpp#L54) noting that the `archive_read_open_filename` call uses narrow strings and would need a wide opener / custom `QFile` callbacks before Windows enablement.

### Phase 3: Shell, Process, and URL Cleanup

The biggest risk in this phase is **external 7z** (see "External 7z Package Import/Export" above) — it is the most likely path-handling site that will fail and need a non-trivial workaround. Address it first so any downstream design choices (e.g., switching to in-process 7zip) can be made early.

- Verify external 7z package import/export with long paths. If it fails, decide between: (a) prefixing only the arguments passed to the external process, (b) creating a junction/symlink to a short path as a temporary working dir, (c) accelerating the existing TODO to switch to in-process 7zip.
- Replace **all** manual local file URL construction in `YACReaderLibrary` with `QUrl::fromLocalFile`.
- Replace the `.remove("file:///").remove("file:")` pattern at [library_window.cpp:1639](YACReaderLibrary/library_window.cpp#L1639) with `QUrl(...).toLocalFile()` — or, better, change the upstream `ComicModel::CoverPathRole` to return a plain path.
- Verify Library-to-Reader launch with long paths.
- Fix Windows "open containing folder, select file" using `SHOpenFolderAndSelectItems` with `SHParseDisplayName`. Add a fallback that opens the parent folder via `QDesktopServices::openUrl(QUrl::fromLocalFile(parentPath))` if selection fails.

### Phase 4: Database and Library Workflows

- Fix `ATTACH DATABASE` quoting in [data_base_management.cpp:506-510](YACReaderLibrary/db/data_base_management.cpp#L506-L510) (this is a standalone correctness fix, not just a long-path fix). Try parameter binding first; fall back to `''` escaping with a regression test.
- Verify SQLite create/load/update/import/export under long paths.
- Test library create/update/delete paths and rename/move flows (including renaming a comic so its DB row's path becomes long).
- Test cover save/copy paths.

### Phase 5: Tests and CI

Add Windows-only tests or smoke scripts that create a temporary directory tree whose absolute path exceeds 260 characters.

Suggested test cases:

- `QFile`, `QDir`, and `QFileInfo` can create, enumerate, and remove long paths.
- `CompressedArchive` opens and extracts a page from a long-path `.cbz`, `.cbr`, `.7z`, and `.tar`.
- `InitialComicInfoExtractor` extracts a cover from a long-path archive.
- `PdfiumComic` opens a long-path PDF.
- `LibraryCreator` creates and updates a library rooted in a long path.
- `DataBaseManagement` creates and loads `library.ydb` in a long path.
- `DataBaseManagement` exports/imports `comics-info` to and from paths containing a single quote (`'`).
- Library-to-reader handoff opens a long-path comic.
- Library rename / move of a comic from a short path into a long path.
- Package export/import works with long source and destination paths.
- Server routes can address a comic with a long nested relative path and with non-ASCII filenames.
- Delete/trash/copy/move workflows behave correctly or report a useful error.
- Manifest verification: a built `YACReader.exe` reports `longPathAware = true` via `mt.exe`.

CI consideration: the GitHub Actions Windows runners default to `LongPathsEnabled = 0`. The test job must either set it (admin registry write) or skip long-path tests with a clear message. Document the choice.

## Acceptance Criteria

- All three Windows executables are manifest long-path aware (verified via `mt.exe` or equivalent on the linked binary).
- A comic archive at an absolute path longer than 260 characters opens in `YACReader`.
- `YACReaderLibrary` can create, update, browse, and open comics from a library rooted at a long path.
- Covers and `library.ydb` are created under long `.yacreaderlibrary` paths.
- The default Windows archive backend (`7zip`) works with long paths.
- The unarr backend works with long paths or has a documented, enforced boundary fix.
- `comics-info` import/export works against paths containing a single quote (`'`) — regardless of length.
- Manual `file:///` URL construction for local paths is removed from touched code; `.remove("file:///")` patterns are removed.
- **No `\\?\` extended-length prefixes appear in stored library paths, recent-files lists, database rows, HTTP routes, settings, logs, or anywhere user-visible.** Prefixes exist only inside narrowly-scoped boundary helpers and are stripped before return.
- Failure modes are clear when Windows policy is not enabled or a third-party app/tool cannot handle long paths.

## Open Questions

- Which bundled 7z executable/version is shipped on Windows under [utils/7zip](utils/7zip), and does it support long path arguments? If it does not, do we accept (a) prefixing args at the boundary, (b) a short-path junction shim, or (c) accelerating the in-process 7zip migration?
- ~~Does the in-tree 7zip `CInFileStream` path type currently compile as Unicode on Windows?~~ **Resolved:** No. `USE_UNICODE_FSTRING` is undefined; the narrow branch compiles. Phase 2 fixes this.
- Should `YACReaderLibraryServer` on Windows be treated as a first-class long-path target, or only verified opportunistically? (Phase 1 includes adding a `.rc` for it, which assumes first-class.)
- Do we want automated Windows CI coverage for long paths (requires admin registry write on the runner), or a documented manual release checklist first?
- The bundled unarr is a prebuilt DLL — do we want to track its upstream version in release notes / `dependencies/unarr/README` so future bumps are auditable?
- Should the `ATTACH DATABASE` escaping fix ship as a separate PR before this work, since it is a standalone correctness bug?
