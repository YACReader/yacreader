# YACReader Changelog

Version counting is based on semantic versioning (Major.Feature.Patch)

## WIP

### YACReader
* Show error when opening bad open recent entries
* GoTo page: Fix segfault when entering page numbers greater than comic's page count
  (toolbar and page flow)
* Initialize GotoFlow on current page, not cover
* Quick navigation mode: Fix page index error when using both mouse and keyboard for navigation
* Remove obsolete double page mode debug messages
* Support HDPI screens.
* Use one wheel mouse step per page in full page mode.
* Add updated MIME types to .desktop file for .cbz and .cbr
* Add settings to control page turning behaviour on scroll.
* Make forward and backward buttons in a mouse turn pages.
* Make forward and backward buttons in a mouse turn pages.
* Don't crash if bookmarks get corrupt for some reason.
* Magnifying glass fixes and improvements.
* Shorcuts customization fixes.

### YACReaderLibrary
* Support HDPI screens.
* Support import legacy XML info in comic files.
* Ensure that comic info exports use .ydb as file suffix.
* Enable arrow keys navigation in the folders tree view.
* Add `Publication Date` column to the comics table view.
* Use a scale effect in the comics grids on mouse over.
* Add system info to the help/about dialog to help reporting bugs.
* Fix selection when clicking on a folder in search mode.
* Fix defaul value for manga/comic mode in folders.

## 9.8.2
### YACReaderLibrary
* Fix opening comics from the continue reading banner.
* Make available next/prev comic covers in the iOS app while reading. (ios app 3.16.1 needed)
### Server
* Make available next/prev comic covers in the iOS app while reading. (ios app 3.16.1 needed)

## 9.8.1
### YACReaderLibrary
* Fix "reading lists" reading order on YACReader. Now YACReader is able to open the right comics in the right order.
### Server
* Fix "reading lists" reading order on YACReader for iOS. Now YACReader for iOS is able to open the right comics in the right order (ios app 3.15.0 or newer is needed).

## 9.8.0
### YACReader
* Add support for full manga mode.
* Fix UP,DOWN,LEFT,RIGHT shortcuts for moving a zoomed in page around.
### YACReaderLibrary
* New search engine.
* New `manga` field added to comics and folders to tag content as manga, any content added to a manga folder will become manga automatically.
* Support for HTML in comic synopsis, this fixes the synopsis when it comes from Comic Vine with HTML tags.
* Improve keyboard navigation in Comic Vine dialog.
### Server
* New `manga` field is sent to YACReader for iOS, so comics tagged as manga will be recognized as such when reading remotely or importing comics.

## 9.7.1
### YACReader
* Fix enlargement/stretching default value

## 9.7.0
### YACReader
* Image enlargement/stretching can now be disabled for fit
  to width and height
* New option to show covers as single pages in double page mode (enabled by default)

### YACReaderLibrary
* update QsLog logger to version 2.1, snapshot 46b643d5bcbc
* fix object leaks in database code
* add bidirectional sync support

### YACReaderLibraryServer
* add support for port setting from the commandline
* update internal server code to QtWebApp 1.7.11

### Refactoring
* Move QtWebApp and QsLog to new third_party directory

## 9.6.0
### Reader and Library
* RAR5 support.

### YACReader
* Fix some issues in magnifying glass when used in hdpi screens

### YACReaderLibrary
* New setting to close YACReaderLibrary to the system tray.
* Fastest sync back from iOS.
* Option to hide current/next comic banner in the grid view.
* Fix saving progress when reading in double page mode.
* Fixes in Comic Vine integration.
* Other fixes and stability improvements.

## 9.5.0
* Require at least Qt 5.4 for compiling, Qt 5.6 for qml based views

### Reader and Library
* Remove legacy OpenGL flow needed for Qt 5.3 compatibility
* Improve commandline handling

### YACReaderLibrary
* New current/next comic view in the grid view, available for folders and reading lists.
* Comics in `Reading` are now sorted by last time opened.
* Fix system log functions
* Replace Qrencode executables with dynamic loaded library
* Add prebuilt qrencode libraries for win and mac
* Update Qrencode to version 4.0.0

### Server
* New API V2 implementation.

## 9.0.0 (2018-2-18)

### Library and Reader

* Updated Qt to 5.9.1.
* Faster (way faster) pdf backend based on pdfium.
* unarr is used now for handling compressed files (there are some temporal side
  effects: f.e. 7z is not supported). YACReader can still be compiled using 7zip
  as decompresion library.
* Fixed fullscreen context menus (windows).
* Minor fixes.
* New app icons for Windows.
* Initial support for Haiku OS

### YACReaderLibrary
* New comics view in addition to flow and grid views: info view.
* New side view for showing current comic info in the grid view
  (it can be shown using the icon in the right bottom part of the window).
* Improved the look and feel of the grid view.
* Fixed Comic Vine integration (using the new https end point).
* Usability fixes to Comic View scraper.
* Fixed UI update when clients update comics status.
* New server setting for disabling covers on folders in the client's remote
  browser (iOS), this is a workaround to fix a performance issue in large
  libraries.
* Fixed YACReader not found in macos.

### YACReader
* Added an option to disable showing the go to flow on mouse over.
* New "Quick Navigation Mode" for the go to flow feature, full-wide + a scroll
  bar, it can be enabled in the settings dialog. (thanks to Yuu Mousou).
* Zig-zag autoscroll reading mode, please see the reading section in the
  shortcuts dialog for enabling this, by default there are no keys assigned to
  the scrolling actions (thanks to Daisuke Cato).
* Menu bar added for YACReader in macos (thanks to Daisuke Cato).
* "Go to flow" is now manga aware (thanks to Daisuke Cato).
* Added "Open recent" menu entry.
* Fixed issues experienced when opening new comics too fast

### YACReaderLibraryServer (headless version of YACReaderLibrary)
* Added a systemd service file to run yacreaderlibraryserver on Linux based systems.
* Added safety checks to the commandline interface to avoid creation of empty libraries

## 8.5.0 - 2016-03-28
* headless version of YACReaderLibrary's server (no gui)
* grid view has been enhanced with a new slider for choosing covers size
 and it also includes a fully configurable background that can use
 the covers of your comics. Now every single folder will have a unique look.
* YACReader UI has been refreshed with a cleaner look.
* New fit and zoom modes, finally!.
* open recent functionality

## 8.0.0
* Reading lists
* Tags
* 'Favorites' and 'being read' lists
* New search engine, now you can filter folders and comics
* New grid viewb
* Add and delete folders
* Update a single folder (no need for updating the whole library to rescan a single folder)
* Drag and drop for adding new comics and folders
* Customizable shorcuts
* Manga mode (thank you Felix)
* Spread page detection for double page mode (including manga mode)(thank you again Felix :) )
* New view for folders not containing comics (only subfolders)
* Save selected covers to disk
* Comics in Reading Lists and Tags can be sorted by drag&drop
* Sublist in Reading Lists can by sorted by drag&drop
* Added WebP image format support
* The user has to provide its own Comic Vine API key to prevent usage limit errors from Comic Vine
* New unarr decompression backend for Linux and Unix systems
* Fixed memory and filedescriptor leaks in 7zip wrapper
* Dropped support for Qt4
* Lots of smaller bugfixes

## 7.2.0
* Added support for the new "remote read" feature in iOS devices.
* Improved stability
* Fixed broken compatibility with Windows XP
* Improved Linux "packageability" (thanks to Felix, Alexander and Yoann)**
* German translation (thanks to Gerhard Joeken)
* Bug fixes.

## 7.1.0
* Added option to reset the rating of a comic book.
* Fixed bugs affecting page information.
* Fixed bug that marked a finished comic as started if it was read again.
* Added 2 statuses for folders (Complete/Completed).
* Fixed bug in YACReaderLibrary <-> YACReader communication.
* Added actions related to comics to the context menu of the comics table.
* Fixed bug that caused unlimited server log growth.
* Fixed minor bugs

## 7.0.2 (MacOSX only)
* Removed Poppler usage in MacOSX version.
* Work on translations.
* Fixed minor bugs

## 7.0.1
* Added QsLog to YACReader
* Fixed bug in YACReaderLibrary <-> YACReader communication.

## 7.0 (Final)
* Fixed keyboard events in some dialogs
* Fixed support for Rar archives on Unix systems
* Fixed problems deleting comics
* Improved error handling
Fixed communication between YACReader and YACReaderLibrary * Fixed communication between YACReader and YACReaderLibrary
* Fixed the toolBar in MacOSX
* Improved OpenGL compatibility on NVIDIA cards
* Fixed minor bugs

## 6.9 (internal)
* Added automatic opening of the next/previous comic when reaching the end/cover of the current comic
* Fixed behavior of new version detected dialog. Now warns once a day or if user chooses every 14 days.
* Fixed the width adjustment of the toolbar title in YACReaderLibrary.
* Added log to YACReaderLibrary (will allow users to provide more information about their bugs).
* Fixed bug in browsing history (and editing comics) after using search engine.

## 6.8 (internal)
* Fixed bug that caused unexpected shutdown after changing the vertical sync mode (flow).
* Fixed bug that caused toolbar in viewer could not be hidden/shown without an open comic book
* Improved error handling when opening comics
* Fixed some bugs related to comic opening
* Added rating function
* The viewer can now open image files directly. Opening an image file opens the directory containing the file with all images.
* Fixed sorting of folders and comics used in browsing from iOS devices.

## 6.7 (internal)
* Added new fields in the database to store additional information about comics: rating, current page, bookmarks and image settings.
* Added communication between YACReaderLibrary and YACReader to be able to store comics progress and additional information

## 6.6 (internal)
* Modified YACReader to open compressed files using 7z.dll (.so, .dylib)
* YACReader now opens comics by the last page read.
* Fixed bug that caused that some comics could not be opened from YACReaderLibrary in YACReader.
* Fixed the way in which the "information label" was updated.

## 6.5
* New main interface of YACReaderLibrary and YACReader
* Fixed bug that caused the server not to activate on first boot on MacOSX
* Fixed bug that caused a crash when closing YACReaderLibrary every time the server was used
* New design for the comic properties dialog.
* Added forward and backward navigation of visited folders.
* Editing a library name does not force library reloading now.
* Corrected the background color in the magnifying glass.
* New button to adjust to top
* Removed always on top option
* Show in container folder fixed on Windows and MacOSX

## 6.4 (internal)
* Normalized rendering of pages in double page mode
* Added delete comics from hard disk function
* New comic toolbar icons

## 6.3 (internal)
* Improved error handling related to libraries
* Added button to hide cover art in the import screen
* Added "Libraries" and "Folders" headings to the navigation bar
* New icons to select root folder, expand and collapse all.
* Button to change the server port by the user.
* Comic list columns can now be reordered.
* YACReaderLibrary now allows only one instance running.
* Read column added.
* Comic list style changed.
* Fixed bugs related to perform operations on comics when there was none selected in the comics list.

## 6.2
* New "welcome" window
* New import/update window
* New control for search
* New image for the marks of read comics (OpenGL only)
* Changed the layout of some icons
* Changed the way to delete meta information (delete database/disk covers)
* Hidden YACReader Flow advanced configuration options, accessible now after pressing a button (simpler configuration dialogs)

## 6.0.1 (internal)
* Fixed bug when using Home/end keys
* Fixed bug that when starting YACReaderLibrary for the first time caused that the covers were not shown (only under certain circumstances)
* Added some keyboard shortcuts to YACReaderLibrary to the already existing ones

## 6.0
* Improved startup speed thanks to the use of /LTCG as a compile option
* Fixed OpenGL related bug that caused excessive CPU consumption on NVidia cards
Added icons for each supported file type in YACReaderLibrary * Added icons for each supported file type in YACReaderLibrary
* Changed "folder" icon in YACReaderLibrary
Added page width adjustment bar in YACReader toolbar * Added widget for page width in YACReader toolbar
* Added widget for information label
* Added new visual style to goToFlow
* Added filters to control brightness, contrast and gamma value * Added new filters to control brightness, contrast and gamma value
* Added front page and last page notifications
* InformationLabel is now displayed in the upper right corner.
* InformationLabel is displayed in 3 different sizes depending on resolution.
* Fixed bug that caused read comic marks not to be drawn properly.
* InformationLabel now remembers whether or not to display the information label.
* Fixed bug that caused YACReader to crash when turning pages too fast.
* Added "Size" column to the comics list in YACReaderLibrary.
* Added "natural" sorting of the comics in the current comic directory.
* Fixed bug that caused the wrong comic to open in YACReaderLibrary.
* Changed the way languages are loaded, now you can add translations without recompiling.
