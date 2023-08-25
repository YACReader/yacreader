# YACReaderLibraryServer Settings

When you launch `yacreaderlibraryserver` the app uses a settings file to determine some behaviors. The path to the settings file is printed as part of the help when you run `yacreaderlibraryserver` with no commands.

The settings file follows the `INI` format and it contains various sections, the one you should modify is `[libraryConfig]`. The settings file is shared between `YACReaderLibrary` and `YACReaderLibraryServer`, this file only describes the settings relevant for `YACReaderLibraryServer`.

The following is a template for the settings available for `YACReaderLibraryServer`, it includes the default values and the values available for each setting:

```
[libraryConfig]

; determines whether legacy metadata in xml format is parsed or not (and added to the database) when new comics are added [true|false]
IMPORT_COMIC_INFO_XML_METADATA=false

; enables libraries update at start up [true|false]
UPDATE_LIBRARIES_AT_STARTUP=false

; enables periodic libraries updates [true|false]
UPDATE_LIBRARIES_PERIODICALLY=false

; if periodic updates are enabled, this is the settings that determines how often they happen
; 0 -> 30 minutes
; 1 -> 1 hour
; 2 -> 2 hours
; 3 -> 4 hours
; 4 -> 8 hours
; 5 -> 12 hours
; 6 -> daily
UPDATE_LIBRARIES_PERIODICALLY_INTERVAL=2

; enables scheduled updates [true|false]
UPDATE_LIBRARIES_AT_CERTAIN_TIME=false

; if sheduled updates are enabled, this is the time when they happen in 24h format
UPDATE_LIBRARIES_AT_CERTAIN_TIME_TIME=00:00

```

WARNING! During library updates writes to the database are disabled! Don't schedule updates while you may be using the app actively.