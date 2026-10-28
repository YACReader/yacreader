# ComicBookDS (`.cbds`) format

ComicBookDS is a homebrew comic reader for the Nintendo DS. Its books are made on a PC with the
PictoDS converter (Windows) or the open source `cbdsconv` script, and use the `.cbds` extension.
This document describes what YACReader relies on and where that knowledge comes from.

Sources:

- 25 real `.cbds` books made by PictoDS (all `Version = 200`), inspected entry by entry.
  They are not part of the repository.
- The `cbdsconv` converter (<https://github.com/son-link/cbdsconv>, GPL), which writes the format.
- Descriptions of ComicBookDS and PictoDS on gamebrew.org and fileinfo.com.

## Container

A `.cbds` file is a regular ZIP archive. The real books use Deflate with data descriptors
(general purpose flag bit 3), `cbdsconv` uses Python's `ZIP_DEFLATED`. No encryption, no
ZIP64, no comments. Every decompression backend of YACReader opens it by its `PK\x03\x04`
signature, nothing format specific is needed to read the container.

## Layout

```
ComicBookDS_book.ini
IMAGE/1.jpg  ... IMAGE/N.jpg      pages
NAME/1.txt   ... NAME/N.txt       original file name of each page
SMALL_N/1.jpg ... SMALL_N/N.jpg   preview, normal orientation
SMALL_R/1.jpg ... SMALL_R/N.jpg   preview, rotated 90 degrees
THMB_N/1.jpg ... THMB_N/N.jpg     thumbnail, normal orientation
THMB_R/1.jpg ... THMB_R/N.jpg     thumbnail, rotated 90 degrees
```

- Pages are numbered from 1 without leading zeros. Entries are stored in lexicographic order
  (`1, 10, 11, ..., 2, 20, ...`), so the archive order is **not** the reading order; natural
  sorting of the names is.
- Every page exists in five resolutions. In the real books: `IMAGE` 700 px wide (about 900-1030
  px high), `SMALL_N` 140x192, `SMALL_R` 256x187, `THMB_N` 33x46, `THMB_R` 62x45. `cbdsconv`
  targets 700x1054, 127x192, 30x46, 256x169 and 62x40, rotating the image by -90 degrees for
  the `_R` variants (used by the DS when it is held sideways).
- Only `IMAGE/` holds pages to display. All other folders are derived data of the reader.
- `NAME/N.txt` holds the source file name of page N (Latin-1 text, CRLF). It is not needed to
  order the pages.
- All images are JPEG. No tiling or other quality levels were found.

## `ComicBookDS_book.ini`

Latin-1 text with CRLF line endings, `;` comments and `key = value` lines, no sections:

| Key | Meaning |
|---|---|
| `CbCredits1..3` | Credit lines shown by the reader (title, author, provider), often `Unknown` |
| `LeftToRight` | `1` left to right, `0` right to left |
| `NbPages` | Number of pages |
| `Version` | Compatibility version, `200` in all known books |
| `iWidth`, `iHeight`, `iQuality`, `iSize` | Target size and JPEG quality of `IMAGE` |
| `oWidth`, `oHeight`, `oQuality`, `oSize` | Same for the `SMALL_*` previews |
| `thWidth`, `thHeight`, `thQuality`, `thSize` | Same for the `THMB_*` thumbnails |

In the 21 intact PictoDS books `NbPages` matches the number of `IMAGE/` entries exactly. It is
still not something to rely on in general: `cbdsconv` writes the number of pages plus one and
skips the number of any page it fails to convert, so its books can have gaps in their numbering.

## How YACReader reads it

An archive is handled as a ComicBookDS book when its name ends in `.cbds` or when it has a
`ComicBookDS_book.ini` entry at its root. For such a book only the files directly inside the
top level `IMAGE/` folder are page candidates (see `common/cbds_page_index.cpp`). After that it
goes through the same path as any other comic: unsupported files are filtered out by extension
and the cover is the first page. The reader and the library sort the pages the same way, by the
natural (numeric) order of their names written as `IMAGE/<file>`, whatever separator, case or
leading separator the archive uses. The reader's heuristic for spread files such as `1011.jpg`
is not applied, because ComicBookDS page names are plain sequence numbers.

- The ini file is not read. The page list comes from the archive, which is the only reliable
  source, and the optional metadata is not used yet. Broken, huge or missing ini files cannot
  affect the result.
- A book without any `IMAGE/` entry falls back to the regular behaviour (every image is a page).
- The file is never modified or converted. Pages are read from the archive into memory like for
  CBZ.
- Only an entry of the form `IMAGE/<file>` is a page. `\` is accepted as a separator and a
  leading `/` or `\` is ignored, so names are always relative to the archive root. Nested
  folders, `..`, drive letters such as `C:/IMAGE/...` and directory entries never match. The
  fallback above returns the archive list unchanged, as for any other comic.
- YACReader does not extract archive entries to disk, so entry names cannot cause path traversal.
- Detecting a book renamed to another extension needs the ini entry in the archive listing. The
  unarr backend leaves empty entries out of its listing, so a renamed book with an empty ini is
  read as a regular archive there. Files with the `.cbds` extension are not affected.

## Damaged books

Some books in the wild are damaged: the file keeps its size, but from some point (in the books
seen, a 64 KiB boundary) to the end the data is zeros, including the ZIP central directory. Such
a file is not a valid ZIP archive and opens with an error, or only partially, depending on how
much the decompression backend can recover from the local headers. This is a data problem of the
individual file, not a format variant.

## Unknown

- Books from older ComicBookDS versions (before the `.cbds` extension and before `Version = 200`)
  have not been seen.
- Whether other converters wrote different folder names, PNG pages or tiled pages is unknown.
- The use of `LeftToRight = 0` (all known books are left to right) has not been verified with a
  real book; mapping it to YACReader's manga mode is a possible follow-up.
