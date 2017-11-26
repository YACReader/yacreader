# Building YACReader from source

YACReader and YACReaderLibrary are build using qmake. To build and install the
program, run:

> qmake-qt5 CONFIG+=[Options]  
> make  
> make install

from the source dir. For seperate builds of YACReader or YACReaderLibrary,
enter their respective subfolders and run the commands from there.

The headless version of YACReaderLibrary is located in the YACReaderLibraryServer
folder. To build it, enter the folder and run the steps as described above.


## Build dependencies:

- Qt >= 5.3 with the following modules:
	- declarative
	- quickcontrols
	- sql
	- script
	- multimedia
	- imageformats
	- opengl
	- sql-sqlite
	- network

- A pdf rendering backend (optional, see below)
- qrencode (optional)
- glu
- (lib)unarr (see below)

Please note that not all of these dependencies are needed at build time.
A good example for this is YACReaderLibrary's GridView mode which will
silently fail and only show a white page if the proper qml modules
(declarative, quickcontrols) are not missing.

## Backends

### Decompression

YACReader uses [(lib)unarr](https://github.com/selmf/unarr) for comic book
decompression. Most Linux distributions don't ship this library yet, so will
probably have to build it yourself.

We recommend using (lib)unarr as a shared library, but we also support static
and embedded builds. Please consult the [README](compressed_archive/unarr/README.txt)
for more information on this topic.

### PDF

Starting with version 9.0.0 YACReader supports the following pdf render engines:

- poppler (Linux default)
- pdfium (Default for Windows and MacOS)
- pdfkit (macOS only)
- no_pdf (disables pdf support)

To override the default for a given platform add CONFIG+=[pdfbackend] as an options
when running qmake.

While the Poppler backend is well tested and has been the standard for YACReader
for a long time, it's performance is a bit lacking. The pdfium engine offers
much better performance (about 10x faster on some pdf files we tested).
However, at the time of this writing, it is not a software that is available
prepackaged for Linux.

### Other build options:

You can adjust the installation prefix as well als the path make install uses
to install the files.

>qmake PREFIX=DIR

sets the default prefix (for example "/", "/usr", "/usr/local").

>make install INSTALL_ROOT=DIR

can be used to install to a different location, which is usefull for packaging.

Default values:

>PREFIX=/usr  
>INSTALL_ROOT=""

On embedded devices that don't support desktop OpenGL, it is recommended to use
the no_opengl config option:

qmake-qt5 CONFIG+=no_opengl

This will remove any dependency on desktop OpenGL and hardlock YACReader's
coverflow to software rendering. Please note that it doesn't actually remove
OpenGL from the build, the Qt toolkit will still make use of it.


# Feedback and contribution

If you're compiling YACReader because there is no package available for your
Linux distribution please consider creating and submitting a package or filing a
package request.

While we do provide packages for .deb based distributions as well as an AUR
package for Archlinux, we are in need of packagers especially for RPM based
distributions. If you are interested, please contact @selmf on the YACReader
forums or write an email to info@yacreader.com

If you have already created a package please let us know so we can add it to
our downloads list ;)
