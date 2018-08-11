If you are trying to compile YACReader with a 7zip decompression backend,
you need to download de source code of 7zip (Windows) or p7zip (Linux/MacOSX).

Please extract it and rename the folder to lib7zip (Windows) or libp7zip (Linux/MacOSX),
then copy it to $YACREADER_SRC/compressed_archive/ (this folder).

YACReader is compiled using 7zip/p7zip 9.20.1 and will not work with newer versions.

On Linux/Unix this means your YACReader installation will stop working if you
update your installation of p7zip to a newer version. If you wish to keep using
p7zip with YACReader, you can copy 7z.so and Codecs/Rar29.so from p7zip 9.20.1
to "/usr/lib/yacreader/". YACReader will then detect these files and use
them instead of the system provided p7zip files which allows you to keep both
YACReader and an up to date p7zip installation.

Please keep in mind this is only a workaround that is provided for backwards
compatibility and not intended as a long time solution.
It is recommended that you switch to unarr as a decompression backend instead
(see README.txt in compressed_archive/unarr).
