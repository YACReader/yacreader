<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>DBHelper</name>
    <message>
        <location filename="../YACReaderLibrary/db_helper.cpp" line="1848"/>
        <source>The folder entry could not be found in the library database.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FileComic</name>
    <message>
        <location filename="../common/comic.cpp" line="490"/>
        <source>CRC error on page (%1): some of the pages will not be displayed correctly</source>
        <translation>第 %1 页 CRC 校验失败: 部分页面将无法正确显示</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="497"/>
        <source>Unknown error opening the file</source>
        <translation>打开文件时出现未知错误</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="604"/>
        <source>7z not found</source>
        <translation>未找到 7z</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="610"/>
        <location filename="../common/comic.cpp" line="626"/>
        <source>Format not supported</source>
        <translation>不支持的文件格式</translation>
    </message>
</context>
<context>
    <name>LibraryCreator</name>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="172"/>
        <source>Unable to create the library data folder: %1. Check folder access and available disk space.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="177"/>
        <source>The library data folder is missing: %1. Restore it or recreate the library before updating.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="225"/>
        <source>The library database is missing: %1. Restore a backup or recreate the library before updating.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="262"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="324"/>
        <source>Unable to start a library database transaction: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="280"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="355"/>
        <source>Unable to commit the library database: %1</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>LibraryMaintenanceLock</name>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="319"/>
        <source>The library lock owner changed. Retry the operation and review the new owner before recovering it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="323"/>
        <source>Unable to remove the maintenance lock: %1. Make sure the file is not read-only and that you can write to the library data folder, then retry.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="379"/>
        <source>A folder is occupying the maintenance lock file path: %1. Rename or move that folder before retrying.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="381"/>
        <source>Unable to create the library lock file: %1. Check write permissions for the library data folder.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="383"/>
        <source>The library data folder is missing: %1. Restore the folder or recreate the library before updating it.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="389"/>
        <source>The library has a maintenance lock whose owner could not be verified (%1)%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="390"/>
        <source>Another maintenance operation is using this library (%1)%2</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="386"/>
        <source>Unable to create the library lock file: %1. Check available disk space and access to the library data folder.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>QCoreApplication</name>
    <message>
        <location filename="main.cpp" line="83"/>
        <source>
YACReaderLibraryServer is the headless (no gui) version of YACReaderLibrary.

This appplication supports persistent settings, to set them up edit this file %1
To learn about the available settings please check the documentation at https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</source>
        <translation>
YACReaderLibraryServer 是 YACReaderLibrary 的无头（无 GUI）版本。

此应用程序支持持久设置，要设置它们，请编辑此文件 %1
要了解可用设置，请查看文档：https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</translation>
    </message>
</context>
<context>
    <name>SearchFieldRegistry</name>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="16"/>
        <source>Text, quoted text</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="18"/>
        <source>Integer</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="21"/>
        <source>Boolean (true / false)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="24"/>
        <source>Integer (number of days)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="27"/>
        <source>Enum (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="100"/>
        <source>Comic title</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="101"/>
        <source>Series name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="102"/>
        <source>Issue number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="103"/>
        <source>Volume identifier</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="104"/>
        <source>Reading format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="105"/>
        <source>Comic rating</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="106"/>
        <source>Textual tags</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="108"/>
        <source>Writer credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="109"/>
        <source>Penciller credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="110"/>
        <source>Inker credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="111"/>
        <source>Colorist credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="112"/>
        <source>Letterer credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="113"/>
        <source>Cover artist credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="114"/>
        <source>Editor credit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="116"/>
        <source>Story arc name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="117"/>
        <source>Position within a story arc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="118"/>
        <source>Number of issues in a story arc</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="119"/>
        <source>Characters appearing in the comic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="120"/>
        <source>Teams appearing in the comic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="121"/>
        <source>Locations appearing in the comic</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="122"/>
        <source>Primary character or team</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="123"/>
        <source>Comic synopsis</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="125"/>
        <source>Publisher name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="126"/>
        <source>Publishing imprint</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="127"/>
        <source>Publication format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="128"/>
        <source>Recommended age rating</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="129"/>
        <source>Comic genre</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="130"/>
        <source>ISO language code</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="131"/>
        <source>Publication date metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="132"/>
        <source>Series grouping metadata</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="133"/>
        <source>Alternate series name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="134"/>
        <source>Alternate issue number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="135"/>
        <source>Alternate series issue count</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="136"/>
        <source>Number of issues in the series</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="138"/>
        <source>Whether the comic is marked as read</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="139"/>
        <source>Whether reading has started</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="140"/>
        <source>Whether metadata has been edited</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="141"/>
        <source>Whether the comic is in color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="142"/>
        <source>Number of pages</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="143"/>
        <source>Comic file name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="144"/>
        <source>When the item was added</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="145"/>
        <source>When the comic was last opened</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="146"/>
        <source>Comic notes</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="147"/>
        <source>Review text</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="149"/>
        <source>Parent folder name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="150"/>
        <source>Default reading format for the folder</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="151"/>
        <source>Whether the folder is complete</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="152"/>
        <source>Whether the folder is marked as finished</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="153"/>
        <source>When the folder was updated</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>YACReader::XMLInfoLibraryScanner</name>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="64"/>
        <source>The library database is missing. Restore a backup or recreate the library before rescanning XML information.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="76"/>
        <source>Unable to load the archive backend: %1</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="91"/>
        <source>Unable to start an XML rescan transaction.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="107"/>
        <source>The selected folder is no longer in this library. Reload the library before rescanning.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="109"/>
        <source>Unable to read comics for the XML rescan.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="113"/>
        <source>Unable to commit XML metadata changes.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="157"/>
        <source>Unable to update XML metadata for %1.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
