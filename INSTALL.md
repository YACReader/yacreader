# Building YACReader from source

YACReader and YACReaderLibrary are built using CMake. To build from the top
source directory:

```
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

To install (Linux):

```
cmake --install build
```

## Build dependencies:

- CMake >= 3.25
- C++20 compiler
- Qt >= 6.7 with the following modules:
	- Core, Core5Compat, Gui, Widgets
	- Quick, QuickControls2, QuickWidgets, Qml
	- Sql (with SQLite driver)
	- Multimedia
	- Network
	- Svg
	- OpenGLWidgets
	- ShaderTools
	- LinguistTools
	- Test (for running tests)
- Backends for pdf rendering (optional) and file
  decompression (see below)

Not all dependencies are needed at build time. For example the qml components in
YACReaderLibrary (GridView, InfoView) will only show a white page if the
required qml modules (Quick, QuickControls2) are missing.

## Backends

### Decompression

YACReader currently supports two decompression backends, 7zip and (lib)unarr. YACReader
defaults to 7zip for Windows and Mac OS and unarr for Linux and other OS, but you can
override this using the `DECOMPRESSION_BACKEND` option:

```
cmake -B build -DDECOMPRESSION_BACKEND=7zip
cmake -B build -DDECOMPRESSION_BACKEND=unarr
cmake -B build -DDECOMPRESSION_BACKEND=libarchive
```

#### 7zip

[7zip](https://www.7-zip.org/) is the default decompression backend for Windows and Mac OS builds.

It is recommended for these systems, as it currently has better support for 7z
files and supports the RAR5 format.

The 7zip source code is automatically downloaded during configuration via CMake's
FetchContent. No manual setup is needed.

As this backend is not 100% GPL compatible (unrar license restriction), it is not
recommended for installations where the license is an issue.

#### unarr

[(lib)unarr](https://github.com/selmf/unarr) is the default backend for Linux builds.

As of version 1.0.1, it supports less formats than 7zip, notably missing RAR5 support and only having
limited support for 7z on git versions. However, this is rarely an issue in practice as the vast majority
of comic books use either zip or RAR4 compression, which is handled nicely by this backend.

The unarr backend is recommended for packaging, lightweight installations and generally for all users requiring
more stability than the 7zip backend can offer.

The recommended way to use this on Linux or other *NIX is to install it as a package, but you can also do an embedded build.
For more information, please consult the [README](compressed_archive/unarr/README.txt)
### PDF

Starting with version 9.0.0 YACReader supports the following pdf rendering engines:

- poppler (Linux/Unix default)
- pdfium (default for Windows)
- pdfkit (macOS default, macOS only)
- no_pdf (disable pdf support)

To override the default for a given platform use the `PDF_BACKEND` option:

```
cmake -B build -DPDF_BACKEND=poppler
cmake -B build -DPDF_BACKEND=pdfium
cmake -B build -DPDF_BACKEND=pdfkit
cmake -B build -DPDF_BACKEND=no_pdf
```

While the Poppler backend is well tested and has been the standard for YACReader
for a long time, its performance is a bit lacking. The pdfium engine offers
much better performance (about 10x faster on some pdf files we tested).
However, at the time of this writing, it is not a library that is available
prepackaged for Linux.

### Other build options:

You can adjust the installation prefix:

```
cmake -B build -DCMAKE_INSTALL_PREFIX=/usr/local
```

Default value on Linux is `/usr`.

For packaging, use `DESTDIR` with the install command:

```
DESTDIR=/path/to/staging cmake --install build
```

To build only YACReaderLibraryServer (headless server):

```
cmake -B build -DBUILD_SERVER_STANDALONE=ON
```

### Running tests

```
ctest --test-dir build --output-on-failure
```

# Feedback and contribution

If you're compiling YACReader because there is no package available for your
Linux distribution please consider creating and submitting a package or filing a
package request for your distribution.

While we do provide packages for .deb and .rpm based distributions as well as an
AUR package for Archlinux and its derivates, we are in need of downstream packagers
that are willing to make YACReader available as a standard package for their distro.

If you are interested, please contact @selmf on the YACReader forums or write
an email to info@yacreader.com

If you have already created a package please let us know so we can add it to
our downloads list ;)
