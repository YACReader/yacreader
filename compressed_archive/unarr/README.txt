Starting with YACReader 9.0.0 all versions of YACReader use (lib)unarr >= 1.0.1
as decompression backend. For Windows and MacOSX precompiled libraries
are available in the dependencies folder (not included in the source tarballs!).

For all other operating systems or users who wish to compile unarr themselves,
source code and build instructions are available at https://github.com/selmf/unarr/

For best performance it is recommended to build and install unarr as a shared
library.

Users who prefer an embedded build can also download a snapshot from
https://github.com/selmf/unarr/archive/master.zip and extract it in this folder.
The build system will then detect the presence of the source code and include it
in the build process. However, as the embedded build option uses different
compiler flags and does not include any options to detect and make use of libraries
like zlib, bzip2 or lzma embedded builds will have slower extraction speed
and won't support zip files with bzip2 or xz compression.
