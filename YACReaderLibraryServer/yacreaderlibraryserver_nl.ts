<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="nl_NL">
<context>
    <name>DBHelper</name>
    <message>
        <location filename="../YACReaderLibrary/db_helper.cpp" line="1848"/>
        <source>The folder entry could not be found in the library database.</source>
        <translation>De mapvermelding is niet gevonden in de database van de bibliotheek.</translation>
    </message>
</context>
<context>
    <name>FileComic</name>
    <message>
        <location filename="../common/comic.cpp" line="490"/>
        <source>CRC error on page (%1): some of the pages will not be displayed correctly</source>
        <translation>CRC-fout op pagina (%1): sommige pagina&apos;s worden niet correct weergegeven</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="497"/>
        <source>Unknown error opening the file</source>
        <translation>Onbekende fout bij het openen van het bestand</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="604"/>
        <source>7z not found</source>
        <translation>7z niet gevonden</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="610"/>
        <location filename="../common/comic.cpp" line="626"/>
        <source>Format not supported</source>
        <translation>Formaat niet ondersteund</translation>
    </message>
</context>
<context>
    <name>LibraryCreator</name>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="172"/>
        <source>Unable to create the library data folder: %1. Check folder access and available disk space.</source>
        <translation>Kan de gegevensmap van de bibliotheek niet maken: %1. Controleer de toegang tot de map en de beschikbare schijfruimte.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="177"/>
        <source>The library data folder is missing: %1. Restore it or recreate the library before updating.</source>
        <translation>De gegevensmap van de bibliotheek ontbreekt: %1. Herstel deze of maak de bibliotheek opnieuw aan voordat u bijwerkt.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="225"/>
        <source>The library database is missing: %1. Restore a backup or recreate the library before updating.</source>
        <translation>De bibliotheekdatabase ontbreekt: %1. Herstel een back-up of maak de bibliotheek opnieuw aan voordat u bijwerkt.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="262"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="324"/>
        <source>Unable to start a library database transaction: %1</source>
        <translation>Kan geen transactie in de bibliotheekdatabase starten: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="280"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="355"/>
        <source>Unable to commit the library database: %1</source>
        <translation>Kan de transactie in de bibliotheekdatabase niet vastleggen: %1</translation>
    </message>
</context>
<context>
    <name>LibraryMaintenanceLock</name>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="319"/>
        <source>The library lock owner changed. Retry the operation and review the new owner before recovering it.</source>
        <translation>De eigenaar van de bibliotheekvergrendeling is gewijzigd. Probeer de taak opnieuw en controleer de nieuwe eigenaar voordat u de toegang herstelt.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="323"/>
        <source>Unable to remove the maintenance lock: %1. Make sure the file is not read-only and that you can write to the library data folder, then retry.</source>
        <translation>Kan de onderhoudsvergrendeling niet verwijderen: %1. Controleer of het bestand niet alleen-lezen is en of u naar de gegevensmap van de bibliotheek kunt schrijven. Probeer het daarna opnieuw.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="379"/>
        <source>A folder is occupying the maintenance lock file path: %1. Rename or move that folder before retrying.</source>
        <translation>Een map neemt het pad van het onderhoudsvergrendelingsbestand in: %1. Hernoem of verplaats die map voordat u het opnieuw probeert.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="381"/>
        <source>Unable to create the library lock file: %1. Check write permissions for the library data folder.</source>
        <translation>Kan het bibliotheekvergrendelingsbestand niet maken: %1. Controleer de schrijfrechten voor de gegevensmap van de bibliotheek.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="383"/>
        <source>The library data folder is missing: %1. Restore the folder or recreate the library before updating it.</source>
        <translation>De gegevensmap van de bibliotheek ontbreekt: %1. Herstel de map of maak de bibliotheek opnieuw aan voordat u deze bijwerkt.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="389"/>
        <source>The library has a maintenance lock whose owner could not be verified (%1)%2</source>
        <translation>De bibliotheek heeft een onderhoudsvergrendeling waarvan de eigenaar niet kon worden geverifieerd (%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="390"/>
        <source>Another maintenance operation is using this library (%1)%2</source>
        <translation>Een andere onderhoudstaak gebruikt deze bibliotheek (%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="386"/>
        <source>Unable to create the library lock file: %1. Check available disk space and access to the library data folder.</source>
        <translation>Kan het bibliotheekvergrendelingsbestand niet maken: %1. Controleer de beschikbare schijfruimte en de toegang tot de gegevensmap van de bibliotheek.</translation>
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
YACReaderLibraryServer is de headless (geen gui) versie van YACReaderLibrary.

Deze applicatie ondersteunt permanente instellingen. Om ze in te stellen, bewerk dit bestand %1
Voor meer informatie over de beschikbare instellingen kunt u de documentatie raadplegen op https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</translation>
    </message>
</context>
<context>
    <name>SearchFieldRegistry</name>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="16"/>
        <source>Text, quoted text</source>
        <translation>Tekst, tekst tussen aanhalingstekens</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="18"/>
        <source>Integer</source>
        <translation>Geheel getal</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="21"/>
        <source>Boolean (true / false)</source>
        <translation>Booleaans (true / false)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="24"/>
        <source>Integer (number of days)</source>
        <translation>Geheel getal (aantal dagen)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="27"/>
        <source>Enum (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</source>
        <translation>Opsomming (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="100"/>
        <source>Comic title</source>
        <translation>Striptitel</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="101"/>
        <source>Series name</source>
        <translation>Serienaam</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="102"/>
        <source>Issue number</source>
        <translation>Uitgavenummer</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="103"/>
        <source>Volume identifier</source>
        <translation>Volumeaanduiding</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="104"/>
        <source>Reading format</source>
        <translation>Leesindeling</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="105"/>
        <source>Comic rating</source>
        <translation>Stripwaardering</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="106"/>
        <source>Textual tags</source>
        <translation>Teksttags</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="108"/>
        <source>Writer credit</source>
        <translation>Vermelding van de schrijver</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="109"/>
        <source>Penciller credit</source>
        <translation>Vermelding van de tekenaar</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="110"/>
        <source>Inker credit</source>
        <translation>Vermelding van de inkter</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="111"/>
        <source>Colorist credit</source>
        <translation>Vermelding van de inkleurder</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="112"/>
        <source>Letterer credit</source>
        <translation>Vermelding van de letteraar</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="113"/>
        <source>Cover artist credit</source>
        <translation>Vermelding van de omslagtekenaar</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="114"/>
        <source>Editor credit</source>
        <translation>Vermelding van de redacteur</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="116"/>
        <source>Story arc name</source>
        <translation>Naam van de verhaallijn</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="117"/>
        <source>Position within a story arc</source>
        <translation>Positie binnen een verhaallijn</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="118"/>
        <source>Number of issues in a story arc</source>
        <translation>Aantal uitgaven in een verhaallijn</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="119"/>
        <source>Characters appearing in the comic</source>
        <translation>Personages die in de strip voorkomen</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="120"/>
        <source>Teams appearing in the comic</source>
        <translation>Teams die in de strip voorkomen</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="121"/>
        <source>Locations appearing in the comic</source>
        <translation>Locaties die in de strip voorkomen</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="122"/>
        <source>Primary character or team</source>
        <translation>Hoofdpersonage of hoofdteam</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="123"/>
        <source>Comic synopsis</source>
        <translation>Samenvatting van de strip</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="125"/>
        <source>Publisher name</source>
        <translation>Naam van de uitgever</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="126"/>
        <source>Publishing imprint</source>
        <translation>Uitgeversimprint</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="127"/>
        <source>Publication format</source>
        <translation>Publicatie-indeling</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="128"/>
        <source>Recommended age rating</source>
        <translation>Aanbevolen leeftijdsclassificatie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="129"/>
        <source>Comic genre</source>
        <translation>Stripgenre</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="130"/>
        <source>ISO language code</source>
        <translation>ISO-taalcode</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="131"/>
        <source>Publication date metadata</source>
        <translation>Metagegevens van de publicatiedatum</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="132"/>
        <source>Series grouping metadata</source>
        <translation>Metagegevens voor seriegroepering</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="133"/>
        <source>Alternate series name</source>
        <translation>Alternatieve serienaam</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="134"/>
        <source>Alternate issue number</source>
        <translation>Alternatief uitgavenummer</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="135"/>
        <source>Alternate series issue count</source>
        <translation>Aantal uitgaven in de alternatieve serie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="136"/>
        <source>Number of issues in the series</source>
        <translation>Aantal uitgaven in de serie</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="138"/>
        <source>Whether the comic is marked as read</source>
        <translation>Of de strip als gelezen is gemarkeerd</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="139"/>
        <source>Whether reading has started</source>
        <translation>Of het lezen is begonnen</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="140"/>
        <source>Whether metadata has been edited</source>
        <translation>Of de metagegevens zijn bewerkt</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="141"/>
        <source>Whether the comic is in color</source>
        <translation>Of de strip in kleur is</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="142"/>
        <source>Number of pages</source>
        <translation>Aantal pagina&apos;s</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="143"/>
        <source>Comic file name</source>
        <translation>Bestandsnaam van de strip</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="144"/>
        <source>When the item was added</source>
        <translation>Wanneer het item is toegevoegd</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="145"/>
        <source>When the comic was last opened</source>
        <translation>Wanneer de strip voor het laatst is geopend</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="146"/>
        <source>Comic notes</source>
        <translation>Stripnotities</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="147"/>
        <source>Review text</source>
        <translation>Recensietekst</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="149"/>
        <source>Parent folder name</source>
        <translation>Naam van de bovenliggende map</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="150"/>
        <source>Default reading format for the folder</source>
        <translation>Standaard leesindeling voor de map</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="151"/>
        <source>Whether the folder is complete</source>
        <translation>Of de map compleet is</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="152"/>
        <source>Whether the folder is marked as finished</source>
        <translation>Of de map als voltooid is gemarkeerd</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="153"/>
        <source>When the folder was updated</source>
        <translation>Wanneer de map is bijgewerkt</translation>
    </message>
</context>
<context>
    <name>YACReader::XMLInfoLibraryScanner</name>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="64"/>
        <source>The library database is missing. Restore a backup or recreate the library before rescanning XML information.</source>
        <translation>De bibliotheekdatabase ontbreekt. Herstel een back-up of maak de bibliotheek opnieuw aan voordat u de XML-informatie opnieuw scant.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="76"/>
        <source>Unable to load the archive backend: %1</source>
        <translation>Kan de archiefbackend niet laden: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="91"/>
        <source>Unable to start an XML rescan transaction.</source>
        <translation>Kan geen transactie voor het opnieuw scannen van XML starten.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="107"/>
        <source>The selected folder is no longer in this library. Reload the library before rescanning.</source>
        <translation>De geselecteerde map staat niet meer in deze bibliotheek. Herlaad de bibliotheek voordat u opnieuw scant.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="109"/>
        <source>Unable to read comics for the XML rescan.</source>
        <translation>Kan strips voor het opnieuw scannen van XML niet lezen.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="113"/>
        <source>Unable to commit XML metadata changes.</source>
        <translation>Kan wijzigingen in XML-metadata niet vastleggen.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="157"/>
        <source>Unable to update XML metadata for %1.</source>
        <translation>Kan XML-metadata voor %1 niet bijwerken.</translation>
    </message>
</context>
</TS>
