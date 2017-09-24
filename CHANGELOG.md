# YACReader Changelog

Please note: this is a work in progress. Keeping a changelog has been neglected
in the past so information is missing and older release information is in
spanish only. Sorry for the mess.

## 9.0.0 - unreleased

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
* New grid view
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
* A�adida opci�n para resetear el rating de un comics
* Corregidos bugs que afectaban a la informaci�n de p�gina.
* Corregido error que marcaba un comic terminado como empezado si se volv�a a leer.
* A�adidos 2 estados para las carpetas (Completo/Terminado)
* Corregido bug en la comunicaci�n YACReaderLibrary <-> YACReader
* A�adidas las acciones relativas a los comics al men� contextual de la tabla de c�mics.
* Corrgido bug que provocaba el crecimiento ilimatado del log del servidor
* Corregidos bugs menores

7.0.2 (S�lo MacOSX)
* Eliminado el uso de Poppler en la versi�n de MacOSX
* Trabajo en traducciones.
* Corregidos bugs menores

7.0.1
* A�adido QsLog a YACReader
* Corregido bug en la comunicaci�n YACReaderLibrary <-> YACReader

7.0 (Final)
* Corregidos eventos de teclado en algunos di�logos
* Corregido soporte para archivos Rar en sistemas Unix
* Corregidos problemas borrando c�mics
* Mejorada la gesti�n de errores
* Corregida la comunicaci�n entre YACReader y YACReaderLibrary
* Corregida la toolBar en MacOSX
* Mejorada la compatabilidad de OpenGL en tarjetas NVIDIA
* Corregidos bugs menores

## 6.9 - (No p�blica)
* A�adida la apertura autom�tica del siguiente/anterior c�mic al llegar al final/portada del c�mic actual
* Corregido el comportamiento del di�logo de nueva versi�n detectada. Ahora avisa una vez al d�a o si el usuario lo elige cada 14 d�as.
* Corregido el ajuste a lo ancho del t�tulo de la toolbar en YACReaderLibrary.
* A�adido log a YACReaderLibrary (permitir� a los usuarios ofrecer m�s informaci�n sobre sus bugs)
* Corregido bug en el historial de navegaci�n (y al editar comics) despu�s de usar el motor de b�squeda.

## 6.8 - (No p�blica)
* Corregido bug que causaba un cierre inesperado despu�s de cambiar el modo de sincronizaci�n vertical (flow)
* Corregido bug que causaba que la toolbar en el visor no se pudiese ocultar/mostrar sin un c�mic abierto
* Mejorada la gesti�n de errores al abrir c�mics
* Corregidos algunos bugs relacionados con la apertura de c�mics
* A�adida funci�n de rating
* El visor ahora puede abrir archivos de imagen directamente. Si se abre un archivo de imagen se abre el directorio que lo contiene con todas las im�genes.
* Corregida la ordenaci�n de carpetas y c�mics usada en la navegaci�n desde dispositivos iOS

## 6.7 - (No p�blica)
* A�adidos nuevos campos en la base de datos para almacenar informaci�n adicional sobre c�mics: rating, p�gina actual, bookmarks y configuraci�n de imagen
* A�adida comunicaci�n entre YACReaderLibrary y YACReader para poder almacenar el progreso de los c�mics e informaci�n adicional

## 6.6 - (No p�blica)
* Modificado YACReader para que abra los archivos comprimidos usando 7z.dll (.so, .dylib)
* YACReader abre ahora los c�mics por la �ltima p�gina le�da.
* Corregido bug que causaba que algunos c�mics no se pudiesen abrir desde * YACReaderLibrary en YACReader
* Corregido el modo en el que se actualizaba la "information label"

## 6.5
* Nueva interfaz principal de YACReaderLibrary y YACReader
* Corregido bug que causaba que el servidor no se activase en el primer arranque en MacOSX
* Corregido bug que causaba un fallo al cerrar YACReaderLibrary cada vez que se usaba el servidor
* Nuevo dise�o para el di�logo de propiedades de los c�mics.
* A�adida navegaci�n alante y atr�s de las carpetas visitadas.
* La edici�n del nombre de una biblioteca no fuerza ahora que se recargue la biblioteca
* Corregido el color de fondo en la lupa
* Nuevo bot�n para ajustar a lo alto
* Eliminada la opci�n always on top
* Mostrar en carpeta contenedora arreglado en Windows y MacOSX

## 6.4 - (No p�blica)
Normalizado el renderizado de p�ginas en modo doble p�gina
A�adida la funci�n de borrar c�mics desde el disco duro
Nuevos iconos de la barra de herramientas de c�mics

## 6.3 (No p�blica)
* Mejorada la gesti�n de errores relacionada con las bibliotecas
* A�adido bot�n que permite ocultar las portadas en la pantalla de importaci�n
* A�adidos t�tulos "Bibliotecas" y "Carpetas" a la barra de navegaci�n
* Nuevos iconos para seleccionar la carpeta ra�z, expandir y contraer todos.
* Bot�n para cambiar el puerto del servidor por el usuario.
* Ahora las columnas de la lista de c�mics pueden reordenarse
* Ahora YACReaderLibrary s�lo permite una instancia ejecutandose.
* Columna le�do a�adida.
* Cambiado estilo de la lista de c�mics
* Corregidos bugs relacionados con realizar operaciones sobre c�mics cuando no hab�a ninguno seleccionado en la lista de c�mics

## 6.2
* Nueva ventana de "bienvenida"
* Nueva ventana de importar/actualizar
* Nuevo control para la b�squeda
* Nueva imagen para las marcas de c�mics le�dos (s�lo en OpenGL)
* Cambiada la distribuci�n de algunos iconos
* Cambiado el modo de eliminar la metainformaci�n (borrar base de datos/portadas de disco)
* Ocultadas las opciones avanzadas de configuraci�n de YACReader Flow, accesibles ahora tras pulsar un bot�n (di�logos de configuraci�n m�s simples)

## 6.0.1 - (No p�blica)
* Corregido bug al usar las teclas Inicio/fin
* Corregido bug que al arrancar YACReaderLibrary por primera vez causaba que no se * mostrasen las portadas (s�lo bajo ciertas circunstancias)
* A�adidos algunos atajos de teclado a YACReaderLibrary a los ya existentes

## 6.0
* Mejorada la velocidad de inicio gracias al uso de /LTCG como opci�n de compilaci�n
* Corregido bug relacionado con OpenGL que causaba consumo excesivo de CPU en tarjetas NVidia
* A�adidos iconos para cada tipo de archivo soportado en YACReaderLibrary
* Cambiado el icono "folder" en YACReaderLibrary
* A�adida barra para ajustar el ancho de p�gina en la toolbar de YACReader
* A�adido widget para la information label
* A�adido nuevo estilo visual a goToFlow
* A�adidos filtros para controlar el brillo, el contraste y el valor gamma
* A�adidas notificaciones de portada y �ltima p�gina
* InformationLabel se muestra ahora en la esquina superiror derecha.
* InformationLabel se muestra en 3 tama�os diferentes en funci�n de la resoluci�n
* Corregido bug que causaba que las marcas de c�mic le�do no se dibujasen adecuadamente.
* Se recuerda si se debe mostrar o no la "label" informaci�n.
* Corregido bug que provocaba el fallo de YACReader al pasar muy r�pido las p�ginas.
* A�adida columna "Tama�o" a la lista de c�mics en YACReaderLibrary
* A�adida la ordinaci�n "natural" de los comics que hay en directorio del c�mic actual.
* Corregido bug que causaba que se abriese el c�mic erroneo en YACReaderLibrary.
* Cambiado el modo en el que se cargan los lenguages, ahora se pueden a�adir traducciones sin necesidad de recompilar.
