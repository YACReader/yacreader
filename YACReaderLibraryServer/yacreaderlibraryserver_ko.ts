<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="ko">
<context>
    <name>DBHelper</name>
    <message>
        <location filename="../YACReaderLibrary/db_helper.cpp" line="1848"/>
        <source>The folder entry could not be found in the library database.</source>
        <translation>라이브러리 데이터베이스에서 폴더 항목을 찾을 수 없습니다.</translation>
    </message>
</context>
<context>
    <name>FileComic</name>
    <message>
        <location filename="../common/comic.cpp" line="490"/>
        <source>CRC error on page (%1): some of the pages will not be displayed correctly</source>
        <translation>%1번 페이지에서 CRC 오류 발생: 일부 페이지가 올바르게 표시되지 않을 수 있습니다</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="497"/>
        <source>Unknown error opening the file</source>
        <translation>파일을 여는 중 알 수 없는 오류가 발생했습니다</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="604"/>
        <source>7z not found</source>
        <translation>7z를 찾을 수 없습니다</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="610"/>
        <location filename="../common/comic.cpp" line="626"/>
        <source>Format not supported</source>
        <translation>지원하지 않는 형식입니다</translation>
    </message>
</context>
<context>
    <name>LibraryCreator</name>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="172"/>
        <source>Unable to create the library data folder: %1. Check folder access and available disk space.</source>
        <translation>라이브러리 데이터 폴더를 만들 수 없습니다: %1. 폴더 접근 권한과 디스크 여유 공간을 확인하세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="177"/>
        <source>The library data folder is missing: %1. Restore it or recreate the library before updating.</source>
        <translation>라이브러리 데이터 폴더가 없습니다: %1. 업데이트하기 전에 폴더를 복원하거나 라이브러리를 다시 만드세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="225"/>
        <source>The library database is missing: %1. Restore a backup or recreate the library before updating.</source>
        <translation>라이브러리 데이터베이스가 없습니다: %1. 업데이트하기 전에 백업을 복원하거나 라이브러리를 다시 만드세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="262"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="324"/>
        <source>Unable to start a library database transaction: %1</source>
        <translation>라이브러리 데이터베이스 트랜잭션을 시작할 수 없습니다: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="280"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="355"/>
        <source>Unable to commit the library database: %1</source>
        <translation>라이브러리 데이터베이스 트랜잭션을 커밋할 수 없습니다: %1</translation>
    </message>
</context>
<context>
    <name>LibraryMaintenanceLock</name>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="319"/>
        <source>The library lock owner changed. Retry the operation and review the new owner before recovering it.</source>
        <translation>라이브러리 잠금 소유자가 변경되었습니다. 작업을 다시 시도하고 새 소유자를 확인한 후 접근 권한을 복구하세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="323"/>
        <source>Unable to remove the maintenance lock: %1. Make sure the file is not read-only and that you can write to the library data folder, then retry.</source>
        <translation>유지 관리 잠금을 제거할 수 없습니다: %1. 파일이 읽기 전용이 아니며 라이브러리 데이터 폴더에 쓰기 권한이 있는지 확인한 후 다시 시도하세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="379"/>
        <source>A folder is occupying the maintenance lock file path: %1. Rename or move that folder before retrying.</source>
        <translation>유지 관리 잠금 파일 경로를 폴더가 차지하고 있습니다: %1. 폴더 이름을 바꾸거나 이동한 후 다시 시도하세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="381"/>
        <source>Unable to create the library lock file: %1. Check write permissions for the library data folder.</source>
        <translation>라이브러리 잠금 파일을 만들 수 없습니다: %1. 라이브러리 데이터 폴더의 쓰기 권한을 확인하세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="383"/>
        <source>The library data folder is missing: %1. Restore the folder or recreate the library before updating it.</source>
        <translation>라이브러리 데이터 폴더가 없습니다: %1. 폴더를 복원하거나 라이브러리를 다시 만든 후 업데이트하세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="389"/>
        <source>The library has a maintenance lock whose owner could not be verified (%1)%2</source>
        <translation>이 라이브러리의 유지 관리 잠금 소유자를 확인할 수 없습니다(%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="390"/>
        <source>Another maintenance operation is using this library (%1)%2</source>
        <translation>다른 유지 관리 작업에서 이 라이브러리를 사용 중입니다(%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="386"/>
        <source>Unable to create the library lock file: %1. Check available disk space and access to the library data folder.</source>
        <translation>라이브러리 잠금 파일을 만들 수 없습니다: %1. 디스크 여유 공간과 라이브러리 데이터 폴더 접근 권한을 확인하세요.</translation>
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
YACReaderLibraryServer는 YACReaderLibrary의 headless(GUI 없는) 버전입니다.

이 응용 프로그램은 영구 설정을 지원합니다. 설정을 변경하려면 다음 파일을 편집하세요: %1
사용 가능한 설정 문서는 다음에서 확인할 수 있습니다: https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</translation>
    </message>
</context>
<context>
    <name>SearchFieldRegistry</name>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="16"/>
        <source>Text, quoted text</source>
        <translation>텍스트, 따옴표로 묶은 텍스트</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="18"/>
        <source>Integer</source>
        <translation>정수</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="21"/>
        <source>Boolean (true / false)</source>
        <translation>부울 값 (true / false)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="24"/>
        <source>Integer (number of days)</source>
        <translation>정수 (일수)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="27"/>
        <source>Enum (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</source>
        <translation>열거형 (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="100"/>
        <source>Comic title</source>
        <translation>만화 제목</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="101"/>
        <source>Series name</source>
        <translation>시리즈 이름</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="102"/>
        <source>Issue number</source>
        <translation>호 번호</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="103"/>
        <source>Volume identifier</source>
        <translation>볼륨 식별자</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="104"/>
        <source>Reading format</source>
        <translation>읽기 형식</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="105"/>
        <source>Comic rating</source>
        <translation>만화 평점</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="106"/>
        <source>Textual tags</source>
        <translation>텍스트 태그</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="108"/>
        <source>Writer credit</source>
        <translation>글 작가 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="109"/>
        <source>Penciller credit</source>
        <translation>밑그림 작가 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="110"/>
        <source>Inker credit</source>
        <translation>펜선 작가 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="111"/>
        <source>Colorist credit</source>
        <translation>채색 작가 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="112"/>
        <source>Letterer credit</source>
        <translation>식자 작가 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="113"/>
        <source>Cover artist credit</source>
        <translation>표지 작가 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="114"/>
        <source>Editor credit</source>
        <translation>편집자 크레딧</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="116"/>
        <source>Story arc name</source>
        <translation>스토리 아크 이름</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="117"/>
        <source>Position within a story arc</source>
        <translation>스토리 아크 내 위치</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="118"/>
        <source>Number of issues in a story arc</source>
        <translation>스토리 아크의 호 수</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="119"/>
        <source>Characters appearing in the comic</source>
        <translation>만화에 등장하는 등장인물</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="120"/>
        <source>Teams appearing in the comic</source>
        <translation>만화에 등장하는 팀</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="121"/>
        <source>Locations appearing in the comic</source>
        <translation>만화에 등장하는 장소</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="122"/>
        <source>Primary character or team</source>
        <translation>주요 등장인물 또는 팀</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="123"/>
        <source>Comic synopsis</source>
        <translation>만화 줄거리</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="125"/>
        <source>Publisher name</source>
        <translation>출판사 이름</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="126"/>
        <source>Publishing imprint</source>
        <translation>출판 브랜드</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="127"/>
        <source>Publication format</source>
        <translation>출판 형식</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="128"/>
        <source>Recommended age rating</source>
        <translation>권장 연령 등급</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="129"/>
        <source>Comic genre</source>
        <translation>만화 장르</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="130"/>
        <source>ISO language code</source>
        <translation>ISO 언어 코드</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="131"/>
        <source>Publication date metadata</source>
        <translation>출판일 메타데이터</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="132"/>
        <source>Series grouping metadata</source>
        <translation>시리즈 그룹화 메타데이터</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="133"/>
        <source>Alternate series name</source>
        <translation>대체 시리즈 이름</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="134"/>
        <source>Alternate issue number</source>
        <translation>대체 호 번호</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="135"/>
        <source>Alternate series issue count</source>
        <translation>대체 시리즈의 호 수</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="136"/>
        <source>Number of issues in the series</source>
        <translation>시리즈의 호 수</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="138"/>
        <source>Whether the comic is marked as read</source>
        <translation>만화를 읽음으로 표시했는지 여부</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="139"/>
        <source>Whether reading has started</source>
        <translation>읽기를 시작했는지 여부</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="140"/>
        <source>Whether metadata has been edited</source>
        <translation>메타데이터를 편집했는지 여부</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="141"/>
        <source>Whether the comic is in color</source>
        <translation>만화가 컬러인지 여부</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="142"/>
        <source>Number of pages</source>
        <translation>페이지 수</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="143"/>
        <source>Comic file name</source>
        <translation>만화 파일 이름</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="144"/>
        <source>When the item was added</source>
        <translation>항목을 추가한 시점</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="145"/>
        <source>When the comic was last opened</source>
        <translation>만화를 마지막으로 연 시점</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="146"/>
        <source>Comic notes</source>
        <translation>만화 메모</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="147"/>
        <source>Review text</source>
        <translation>리뷰 텍스트</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="149"/>
        <source>Parent folder name</source>
        <translation>상위 폴더 이름</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="150"/>
        <source>Default reading format for the folder</source>
        <translation>폴더의 기본 읽기 형식</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="151"/>
        <source>Whether the folder is complete</source>
        <translation>폴더가 완전한지 여부</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="152"/>
        <source>Whether the folder is marked as finished</source>
        <translation>폴더를 완료로 표시했는지 여부</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="153"/>
        <source>When the folder was updated</source>
        <translation>폴더를 업데이트한 시점</translation>
    </message>
</context>
<context>
    <name>YACReader::XMLInfoLibraryScanner</name>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="64"/>
        <source>The library database is missing. Restore a backup or recreate the library before rescanning XML information.</source>
        <translation>라이브러리 데이터베이스가 없습니다. XML 정보를 다시 검색하기 전에 백업을 복원하거나 라이브러리를 다시 만드세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="76"/>
        <source>Unable to load the archive backend: %1</source>
        <translation>압축 파일 백엔드를 불러올 수 없습니다: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="91"/>
        <source>Unable to start an XML rescan transaction.</source>
        <translation>XML 재검색 트랜잭션을 시작할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="107"/>
        <source>The selected folder is no longer in this library. Reload the library before rescanning.</source>
        <translation>선택한 폴더가 더 이상 이 라이브러리에 없습니다. 다시 검색하기 전에 라이브러리를 새로 불러오세요.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="109"/>
        <source>Unable to read comics for the XML rescan.</source>
        <translation>XML 재검색을 위한 만화를 읽을 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="113"/>
        <source>Unable to commit XML metadata changes.</source>
        <translation>XML 메타데이터 변경 사항을 커밋할 수 없습니다.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="157"/>
        <source>Unable to update XML metadata for %1.</source>
        <translation>%1의 XML 메타데이터를 업데이트할 수 없습니다.</translation>
    </message>
</context>
</TS>
