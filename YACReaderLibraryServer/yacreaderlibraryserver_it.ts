<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="it_IT">
<context>
    <name>DBHelper</name>
    <message>
        <location filename="../YACReaderLibrary/db_helper.cpp" line="1848"/>
        <source>The folder entry could not be found in the library database.</source>
        <translation>La voce della cartella non è stata trovata nel database della libreria.</translation>
    </message>
</context>
<context>
    <name>FileComic</name>
    <message>
        <location filename="../common/comic.cpp" line="490"/>
        <source>CRC error on page (%1): some of the pages will not be displayed correctly</source>
        <translation>Errore CRC alla pagina (%1): alcune pagine non saranno visualizzate correttamente</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="497"/>
        <source>Unknown error opening the file</source>
        <translation>Errore sconosciuto aprendo il file</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="604"/>
        <source>7z not found</source>
        <translation>7z non trovato</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="610"/>
        <location filename="../common/comic.cpp" line="626"/>
        <source>Format not supported</source>
        <translation>Formato non supportato</translation>
    </message>
</context>
<context>
    <name>LibraryCreator</name>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="172"/>
        <source>Unable to create the library data folder: %1. Check folder access and available disk space.</source>
        <translation>Impossibile creare la cartella dati della libreria: %1. Controlla l&apos;accesso alla cartella e lo spazio disponibile sul disco.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="177"/>
        <source>The library data folder is missing: %1. Restore it or recreate the library before updating.</source>
        <translation>La cartella dati della libreria è mancante: %1. Ripristinala o ricrea la libreria prima di aggiornarla.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="225"/>
        <source>The library database is missing: %1. Restore a backup or recreate the library before updating.</source>
        <translation>Il database della libreria è mancante: %1. Ripristina un backup o ricrea la libreria prima di aggiornarla.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="262"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="324"/>
        <source>Unable to start a library database transaction: %1</source>
        <translation>Impossibile avviare una transazione nel database della libreria: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="280"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="355"/>
        <source>Unable to commit the library database: %1</source>
        <translation>Impossibile confermare la transazione nel database della libreria: %1</translation>
    </message>
</context>
<context>
    <name>LibraryMaintenanceLock</name>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="319"/>
        <source>The library lock owner changed. Retry the operation and review the new owner before recovering it.</source>
        <translation>Il proprietario del blocco della libreria è cambiato. Riprova l&apos;operazione e verifica il nuovo proprietario prima di recuperare l&apos;accesso.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="323"/>
        <source>Unable to remove the maintenance lock: %1. Make sure the file is not read-only and that you can write to the library data folder, then retry.</source>
        <translation>Impossibile rimuovere il blocco di manutenzione: %1. Assicurati che il file non sia di sola lettura e di poter scrivere nella cartella dati della libreria, quindi riprova.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="379"/>
        <source>A folder is occupying the maintenance lock file path: %1. Rename or move that folder before retrying.</source>
        <translation>Una cartella occupa il percorso del file di blocco di manutenzione: %1. Rinominala o spostala prima di riprovare.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="381"/>
        <source>Unable to create the library lock file: %1. Check write permissions for the library data folder.</source>
        <translation>Impossibile creare il file di blocco della libreria: %1. Controlla i permessi di scrittura della cartella dati della libreria.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="383"/>
        <source>The library data folder is missing: %1. Restore the folder or recreate the library before updating it.</source>
        <translation>La cartella dati della libreria è mancante: %1. Ripristina la cartella o ricrea la libreria prima di aggiornarla.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="386"/>
        <source>Unable to create the library lock file: %1. Check available disk space and access to the library data folder.</source>
        <translation>Impossibile creare il file di blocco della libreria: %1. Controlla lo spazio disponibile sul disco e l&apos;accesso alla cartella dati della libreria.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="389"/>
        <source>The library has a maintenance lock whose owner could not be verified (%1)%2</source>
        <translation>La libreria ha un blocco di manutenzione il cui proprietario non è stato verificato (%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="390"/>
        <source>Another maintenance operation is using this library (%1)%2</source>
        <translation>Un&apos;altra operazione di manutenzione sta usando questa libreria (%1)%2</translation>
    </message>
</context>
<context>
    <name>QCoreApplication</name>
    <message>
        <location filename="main.cpp" line="83"/>
        <source>
YACReaderLibraryServer is the headless (no GUI) version of YACReaderLibrary.

This application supports persistent settings. To set them up, edit this file: %1
To learn about the available settings, please check the documentation at https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</source>
        <translation>
YACReaderLibraryServer è la versione headless (senza interfaccia grafica) di YACReaderLibrary.

Questa applicazione supporta impostazioni persistenti. Per configurarle, modifica questo file: %1
Per conoscere le impostazioni disponibili, consulta la documentazione all&apos;indirizzo https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</translation>
    </message>
</context>
<context>
    <name>SearchFieldRegistry</name>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="16"/>
        <source>Text, quoted text</source>
        <translation>Testo, testo tra virgolette</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="18"/>
        <source>Integer</source>
        <translation>Intero</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="21"/>
        <source>Boolean (true / false)</source>
        <translation>Booleano (true / false)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="24"/>
        <source>Integer (number of days)</source>
        <translation>Intero (numero di giorni)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="27"/>
        <source>Enum (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</source>
        <translation>Enumerazione (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="100"/>
        <source>Comic title</source>
        <translation>Titolo del fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="101"/>
        <source>Series name</source>
        <translation>Nome della serie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="102"/>
        <source>Issue number</source>
        <translation>Numero dell&apos;albo</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="103"/>
        <source>Volume identifier</source>
        <translation>Identificatore del volume</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="104"/>
        <source>Reading format</source>
        <translation>Formato di lettura</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="105"/>
        <source>Comic rating</source>
        <translation>Valutazione del fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="106"/>
        <source>Textual tags</source>
        <translation>Tag testuali</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="108"/>
        <source>Writer credit</source>
        <translation>Credito dello sceneggiatore</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="109"/>
        <source>Penciller credit</source>
        <translation>Credito del disegnatore</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="110"/>
        <source>Inker credit</source>
        <translation>Credito dell&apos;inchiostratore</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="111"/>
        <source>Colorist credit</source>
        <translation>Credito del colorista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="112"/>
        <source>Letterer credit</source>
        <translation>Credito del letterista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="113"/>
        <source>Cover artist credit</source>
        <translation>Credito dell&apos;artista di copertina</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="114"/>
        <source>Editor credit</source>
        <translation>Credito del curatore</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="116"/>
        <source>Story arc name</source>
        <translation>Nome dell&apos;arco narrativo</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="117"/>
        <source>Position within a story arc</source>
        <translation>Posizione nell&apos;arco narrativo</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="118"/>
        <source>Number of issues in a story arc</source>
        <translation>Numero di albi nell&apos;arco narrativo</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="119"/>
        <source>Characters appearing in the comic</source>
        <translation>Personaggi presenti nel fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="120"/>
        <source>Teams appearing in the comic</source>
        <translation>Squadre presenti nel fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="121"/>
        <source>Locations appearing in the comic</source>
        <translation>Luoghi presenti nel fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="122"/>
        <source>Primary character or team</source>
        <translation>Personaggio o squadra principale</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="123"/>
        <source>Comic synopsis</source>
        <translation>Sinossi del fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="125"/>
        <source>Publisher name</source>
        <translation>Nome dell&apos;editore</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="126"/>
        <source>Publishing imprint</source>
        <translation>Marchio editoriale</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="127"/>
        <source>Publication format</source>
        <translation>Formato di pubblicazione</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="128"/>
        <source>Recommended age rating</source>
        <translation>Classificazione per età consigliata</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="129"/>
        <source>Comic genre</source>
        <translation>Genere del fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="130"/>
        <source>ISO language code</source>
        <translation>Codice lingua ISO</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="131"/>
        <source>Publication date metadata</source>
        <translation>Metadati della data di pubblicazione</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="132"/>
        <source>Series grouping metadata</source>
        <translation>Metadati di raggruppamento delle serie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="133"/>
        <source>Alternate series name</source>
        <translation>Nome alternativo della serie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="134"/>
        <source>Alternate issue number</source>
        <translation>Numero alternativo dell&apos;albo</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="135"/>
        <source>Alternate series issue count</source>
        <translation>Numero di albi della serie alternativa</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="136"/>
        <source>Number of issues in the series</source>
        <translation>Numero di albi nella serie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="138"/>
        <source>Whether the comic is marked as read</source>
        <translation>Se il fumetto è contrassegnato come letto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="139"/>
        <source>Whether reading has started</source>
        <translation>Se la lettura è iniziata</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="140"/>
        <source>Whether metadata has been edited</source>
        <translation>Se i metadati sono stati modificati</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="141"/>
        <source>Whether the comic is in color</source>
        <translation>Se il fumetto è a colori</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="142"/>
        <source>Number of pages</source>
        <translation>Numero di pagine</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="143"/>
        <source>Comic file name</source>
        <translation>Nome del file del fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="144"/>
        <source>When the item was added</source>
        <translation>Quando l&apos;elemento è stato aggiunto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="145"/>
        <source>When the comic was last opened</source>
        <translation>Quando il fumetto è stato aperto l&apos;ultima volta</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="146"/>
        <source>Comic notes</source>
        <translation>Note del fumetto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="147"/>
        <source>Review text</source>
        <translation>Testo della recensione</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="149"/>
        <source>Parent folder name</source>
        <translation>Nome della cartella superiore</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="150"/>
        <source>Default reading format for the folder</source>
        <translation>Formato di lettura predefinito della cartella</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="151"/>
        <source>Whether the folder is complete</source>
        <translation>Se la cartella è completa</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="152"/>
        <source>Whether the folder is marked as finished</source>
        <translation>Se la cartella è contrassegnata come terminata</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="153"/>
        <source>When the folder was updated</source>
        <translation>Quando la cartella è stata aggiornata</translation>
    </message>
</context>
<context>
    <name>YACReader::XMLInfoLibraryScanner</name>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="64"/>
        <source>The library database is missing. Restore a backup or recreate the library before rescanning XML information.</source>
        <translation>Il database della libreria è mancante. Ripristina un backup o ricrea la libreria prima di ripetere la scansione delle informazioni XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="76"/>
        <source>Unable to load the archive backend: %1</source>
        <translation>Impossibile caricare il backend degli archivi: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="91"/>
        <source>Unable to start an XML rescan transaction.</source>
        <translation>Impossibile avviare una transazione per la nuova scansione XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="107"/>
        <source>The selected folder is no longer in this library. Reload the library before rescanning.</source>
        <translation>La cartella selezionata non è più in questa libreria. Ricarica la libreria prima di ripetere la scansione.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="109"/>
        <source>Unable to read comics for the XML rescan.</source>
        <translation>Impossibile leggere i fumetti per la nuova scansione XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="113"/>
        <source>Unable to commit XML metadata changes.</source>
        <translation>Impossibile confermare le modifiche ai metadati XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="157"/>
        <source>Unable to update XML metadata for %1.</source>
        <translation>Impossibile aggiornare i metadati XML di %1.</translation>
    </message>
</context>
</TS>
