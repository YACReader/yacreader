## Ubuntu 24.04 build script
You can use `build.sh` to build YACReader from scratch in `Ubuntu 24.04` with `7zip` (including `RAR5` support) and `poppler` (pdf). This build uses `Qt6`.

The script will create a folder next to it where all the downloads and building will happen, it will also install all the required dependencies. The script builds it and installs 7z.so in /usr/lib/7zip/. Once the scrip finishes succesfully `YACReader`, `YACReaderLibrary` and `YACReaderLibraryServer` should be installed in your system.

WARNIG: This doesn't work with the latest release (9.14), there is a bug and some of the code is still looking for the old p7zip library, you need to build from develop, to do so run: `build.sh develop`.