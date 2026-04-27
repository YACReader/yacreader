# libarchive Decompression Backend

This backend utilizes the [libarchive](https://www.libarchive.org/) library to support a wide
variety of compression formats.

This backend is currently only supported on the Linux platform.

## Using

Enable this backend by setting the `DECOMPRESSION_BACKEND` CMake option:

    cmake -B build -DDECOMPRESSION_BACKEND=libarchive

Upon success, the application can be built as normal.

## Limitations

 * libarchive has a stream-based architecture that does not (currently) offer random access.
   In practice, this means that you can only seek forward and would have to re-open an archive
   to read an entry before the current position. This doesn't seem to have a huge performance
   hit and can be mitigated by creating properly sorted archives.
 * 7z decompression is slow (but seems to be slightly faster than unarr)
