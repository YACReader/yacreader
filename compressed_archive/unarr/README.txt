To use unarr as a decompression engine when building YACReader, download https://github.com/zeniko/unarr/archive/master.zip and extract it in this folder.
This will build unarr as a part of YACReader (static build).

If you're on a Linux/Unix system and prefer to use unarr as a shared library, have a look at https://github.com/selmf/unarr/
This fork of unarr includes a CMake based build system that allows you to build and install unarr as a shared library. YACReader will detect and use 
the installed library at build time if it is installed.