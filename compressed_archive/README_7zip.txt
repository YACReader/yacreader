If you are trying to compile YACReader with a 7zip decompression backend,
you need to download the source code of 7zip 23.01.

Please extract it and rename the folder to lib7zip (Windows),
then copy it to $YACREADER_SRC/compressed_archive/ (this folder).

7zip is the recommended library to use is you want support for 7zip files and rar5.

The app needs to load 7z.dll/7z.so at runtime.

The repo includes prebuilt binaries for windows (7z.dll) and macos (7z.so), check dependencies/7zip.

TODO: build 7z.so for macos with asm support (faster). Format7zF bundle (all supported formats enabled)
TODO: build 7z.so for linux (x86, x86_64, arm, arm64) with asm support (faster).  Format7zF bundle (all supported formats enabled)

Please, open a PR if you are able to build 7z.so.

