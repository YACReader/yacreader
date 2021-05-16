# Building YACReader from source

YACReader and YACReaderLibrary are build using qmake. To build and install the
program, run:

```
qmake CONFIG+=[Options]
make
make install
```

from the top source dir. For separate builds of YACReader or YACReaderLibrary,
enter their respective subfolders and run the commands from there.

The headless version of YACReaderLibrary is located in the YACReaderLibraryServer
folder. To build it, enter the folder and run the commands described above.

Note: If your system has multiple versions of Qt, you need to make sure you are
using qmake for Qt5

## Build dependencies:

- Qt >= 5.9 with the following modules:
	- declarative
	- quickcontrols
	- sql
	- multimedia
	- imageformats
	- opengl
	- sql-sqlite
	- network
- Backends for pdf rendering (optional) and file
  decompression (see below)
- qrencode for QR code generation (optional)

Not all dependencies are needed at build time. For example the qml components in
YACReaderLibrary (GridView, InfoView) will only show a white page if the
required qml modules (declarative, quickcontrols) are missing.

## Backends

### Decompression

YACReader currently supports two decompression backends, 7zip and (lib)unarr. YACReader
defaults to 7zip for Windows and Mac OS and unarr for Linux and other OS, but you can
override this using one of the following config options:

`CONFIG+=7zip`

`CONFIG+=unarr`

#### 7zip

[7zip](https://www.7-zip.org/) and [p7zip](http://p7zip.sourceforge.net/)
are the default decompression backend for Windows and Mac OS builds.

They are recommended for these systems, as they currently have better support for 7z
files and support the RAR5 format.

As this backend requires specific versions of 7zip for Windows and p7zip for *NIX and
is not 100% GPL compatible (unrar License restriction), it is not recommended for
installations where you can't guarantee the installed version of (p7zip) or the license is an issue.

To build using this backend, you need to install additional sources to the build environment.
For more information, please refer to [README_7zip](compressed_archive/README_7zip.txt).

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
- pdfium (default for Windows and MacOS)
- pdfkit (MacOS only)
- no_pdf (disable pdf support)

To override the default for a given platform add CONFIG+=[pdfbackend] as an option
when running qmake.

While the Poppler backend is well tested and has been the standard for YACReader
for a long time, its performance is a bit lacking. The pdfium engine offers
much better performance (about 10x faster on some pdf files we tested).
However, at the time of this writing, it is not a library that is available
prepackaged for Linux.

### Other build options:

You can adjust the installation prefix as well als the path "make install" uses
to install the files.

`qmake PREFIX=DIR`

sets the default prefix (for example "/", "/usr", "/usr/local").

`make install INSTALL_ROOT=DIR`

can be used to install to a different location, which is usefull for packaging.

Default values:

```
PREFIX=/usr
INSTALL_ROOT=""
```

On embedded devices that don't support desktop OpenGL, it is recommended to use
the no_opengl config option:

`qmake CONFIG+=no_opengl`

This will remove any dependency on desktop OpenGL and hardlock YACReader's
coverflow to software rendering. Please note that it does not actually remove
OpenGL from the build, the Qt toolkit will still make use of it.


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
