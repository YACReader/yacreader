If you are trying to compile YACReader with a 7zip decompression backend,
you need to download de source code of 7zip (Windows) or p7zip (Linux/MacOSX).

Please extract it and rename the folder to lib7zip (Windows) or libp7zip (Linux/MacOSX),
then copy it to $YACREADER_SRC/compressed_archive/ (this folder).

YACReader is compiled using 7zip 18.05 and p7zip 16.02.

7zip is the recommended library to use is you want support for 7zip files and rar5.
