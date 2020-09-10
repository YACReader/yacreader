# YACReader Changelog

Please note: this is a work in progress. Keeping a changelog has been neglected
in the past so information is missing and older release information is in
spanish only. Sorry for the mess.

Version counting is based on semantic versioning (Major.Feature.Patch)

## WIP
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
* Añadida opción para resetear el rating de un comics
* Corregidos bugs que afectaban a la información de página.
* Corregido error que marcaba un comic terminado como empezado si se volvía a leer.
* Añadidos 2 estados para las carpetas (Completo/Terminado)
* Corregido bug en la comunicación YACReaderLibrary <-> YACReader
* Añadidas las acciones relativas a los comics al menú contextual de la tabla de cómics.
* Corrgido bug que provocaba el crecimiento ilimatado del log del servidor
* Corregidos bugs menores

## 7.0.2 (Sólo MacOSX)
* Eliminado el uso de Poppler en la versión de MacOSX
* Trabajo en traducciones.
* Corregidos bugs menores

## 7.0.1
* Añadido QsLog a YACReader
* Corregido bug en la comunicación YACReaderLibrary <-> YACReader

## 7.0 (Final)
* Corregidos eventos de teclado en algunos diálogos
* Corregido soporte para archivos Rar en sistemas Unix
* Corregidos problemas borrando cómics
* Mejorada la gestión de errores
* Corregida la comunicación entre YACReader y YACReaderLibrary
* Corregida la toolBar en MacOSX
* Mejorada la compatabilidad de OpenGL en tarjetas NVIDIA
* Corregidos bugs menores

## 6.9 (No pública)
* Añadida la apertura automática del siguiente/anterior cómic al llegar al final/portada del cómic actual
* Corregido el comportamiento del diálogo de nueva versión detectada. Ahora avisa una vez al día o si el usuario  lo elige cada 14 días.
* Corregido el ajuste a lo ancho del título de la toolbar en YACReaderLibrary.
* Añadido log a YACReaderLibrary (permitirá a los usuarios ofrecer más información sobre sus bugs)
* Corregido bug en el historial de navegación (y al editar comics) después de usar el motor de búsqueda.

## 6.8 (No pública)
* Corregido bug que causaba un cierre inesperado después de cambiar el modo de sincronización vertical (flow)
* Corregido bug que causaba que la toolbar en el visor no se pudiese ocultar/mostrar sin un cómic abierto
* Mejorada la gestión de errores al abrir cómics
* Corregidos algunos bugs relacionados con la apertura de cómics
* Añadida función de rating
* El visor ahora puede abrir archivos de imagen directamente. Si se abre un archivo de imagen se abre el directorio que lo contiene con todas las imágenes.
* Corregida la ordenación de carpetas y cómics usada en la navegación desde dispositivos iOS

## 6.7 (No pública)
* Añadidos nuevos campos en la base de datos para almacenar información adicional sobre cómics: rating, página actual, bookmarks y configuración de imagen
* Añadida comunicación entre YACReaderLibrary y YACReader para poder almacenar el progreso de los cómics e información adicional

## 6.6 (No pública)
* Modificado YACReader para que abra los archivos comprimidos usando 7z.dll (.so, .dylib)
* YACReader abre ahora los cómics por la última página leída.
* Corregido bug que causaba que algunos cómics no se pudiesen abrir desde YACReaderLibrary en YACReader
* Corregido el modo en el que se actualizaba la "information label"

## 6.5
* Nueva interfaz principal de YACReaderLibrary y YACReader
* Corregido bug que causaba que el servidor no se activase en el primer arranque en MacOSX
* Corregido bug que causaba un fallo al cerrar YACReaderLibrary cada vez que se usaba el servidor
* Nuevo diseño para el diálogo de propiedades de los cómics.
* Añadida navegación alante y atrás de las carpetas visitadas.
* La edición del nombre de una biblioteca no fuerza ahora que se recargue la biblioteca
* Corregido el color de fondo en la lupa
* Nuevo botón para ajustar a lo alto
* Eliminada la opción always on top
* Mostrar en carpeta contenedora arreglado en Windows y MacOSX

## 6.4 (No pública)
* Normalizado el renderizado de páginas en modo doble página
* Añadida la función de borrar cómics desde el disco duro
* Nuevos iconos de la barra de herramientas de cómics

## 6.3 (No pública)
* Mejorada la gestión de errores relacionada con las bibliotecas
* Añadido botón que permite ocultar las portadas en la pantalla de importación
* Añadidos títulos "Bibliotecas" y "Carpetas" a la barra de navegación
* Nuevos iconos para seleccionar la carpeta raíz, expandir y contraer todos.
* Botón para cambiar el puerto del servidor por el usuario.
* Ahora las columnas de la lista de cómics pueden reordenarse
* Ahora YACReaderLibrary sólo permite una instancia ejecutandose.
* Columna leído añadida.
* Cambiado estilo de la lista de cómics
* Corregidos bugs relacionados con realizar operaciones sobre cómics cuando no había ninguno seleccionado en la lista de cómics

## 6.2
* Nueva ventana de "bienvenida"
* Nueva ventana de importar/actualizar
* Nuevo control para la búsqueda
* Nueva imagen para las marcas de cómics leídos (sólo en OpenGL)
* Cambiada la distribución de algunos iconos
* Cambiado el modo de eliminar la metainformación (borrar base de datos/portadas de disco)
* Ocultadas las opciones avanzadas de configuración de YACReader Flow, accesibles ahora tras pulsar un botón (diálogos de configuración más simples)

## 6.0.1 (No pública)
* Corregido bug al usar las teclas Inicio/fin
* Corregido bug que al arrancar YACReaderLibrary por primera vez causaba que no se mostrasen las portadas (sólo bajo ciertas circunstancias)
* Añadidos algunos atajos de teclado a YACReaderLibrary a los ya existentes

## 6.0
* Mejorada la velocidad de inicio gracias al uso de /LTCG como opción de compilación
* Corregido bug relacionado con OpenGL que causaba consumo excesivo de CPU en tarjetas NVidia
* Añadidos iconos para cada tipo de archivo soportado en YACReaderLibrary
* Cambiado el icono "folder" en YACReaderLibrary
* Añadida barra para ajustar el ancho de página en la toolbar de YACReader
* Añadido widget para la information label
* Añadido nuevo estilo visual a goToFlow
* Añadidos filtros para controlar el brillo, el contraste y el valor gamma
* Añadidas notificaciones de portada y última página
* InformationLabel se muestra ahora en la esquina superiror derecha.
* InformationLabel se muestra en 3 tamaños diferentes en función de la resolución
* Corregido bug que causaba que las marcas de cómic leído no se dibujasen adecuadamente.
* Se recuerda si se debe mostrar o no la "label" información.
* Corregido bug que provocaba el fallo de YACReader al pasar muy rápido las páginas.
* Añadida columna "Tamaño" a la lista de cómics en YACReaderLibrary
* Añadida la ordinación "natural" de los comics que hay en directorio del cómic actual.
* Corregido bug que causaba que se abriese el cómic erroneo en YACReaderLibrary.
* Cambiado el modo en el que se cargan los lenguages, ahora se pueden añadir traducciones sin necesidad de recompilar.
