If you are trying to compile YACReader with a 7zip decompression backend,
you need to download the source code of 7zip 23.01.

Please extract it and rename the folder to lib7zip (Windows),
then copy it to $YACREADER_SRC/compressed_archive/ (this folder).

7zip is the recommended library to use is you want support for 7zip files and rar5.

The app needs to load 7z.dll/7z.so(and Codecs/Rar.so) at runtime.
