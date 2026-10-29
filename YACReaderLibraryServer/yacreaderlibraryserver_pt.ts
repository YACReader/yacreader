<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="pt_BR">
<context>
    <name>DBHelper</name>
    <message>
        <location filename="../YACReaderLibrary/db_helper.cpp" line="1848"/>
        <source>The folder entry could not be found in the library database.</source>
        <translation>A entrada da pasta não foi encontrada no banco de dados da biblioteca.</translation>
    </message>
</context>
<context>
    <name>FileComic</name>
    <message>
        <location filename="../common/comic.cpp" line="490"/>
        <source>CRC error on page (%1): some of the pages will not be displayed correctly</source>
        <translation>Erro CRC na página (%1): algumas páginas não serão exibidas corretamente</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="497"/>
        <source>Unknown error opening the file</source>
        <translation>Erro desconhecido ao abrir o arquivo</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="604"/>
        <source>7z not found</source>
        <translation>7z não encontrado</translation>
    </message>
    <message>
        <location filename="../common/comic.cpp" line="610"/>
        <location filename="../common/comic.cpp" line="626"/>
        <source>Format not supported</source>
        <translation>Formato não suportado</translation>
    </message>
</context>
<context>
    <name>LibraryCreator</name>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="172"/>
        <source>Unable to create the library data folder: %1. Check folder access and available disk space.</source>
        <translation>Não foi possível criar a pasta de dados da biblioteca: %1. Verifique o acesso à pasta e o espaço disponível em disco.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="177"/>
        <source>The library data folder is missing: %1. Restore it or recreate the library before updating.</source>
        <translation>A pasta de dados da biblioteca está ausente: %1. Restaure-a ou recrie a biblioteca antes de atualizar.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="225"/>
        <source>The library database is missing: %1. Restore a backup or recreate the library before updating.</source>
        <translation>O banco de dados da biblioteca está ausente: %1. Restaure um backup ou recrie a biblioteca antes de atualizar.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="262"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="324"/>
        <source>Unable to start a library database transaction: %1</source>
        <translation>Não foi possível iniciar uma transação no banco de dados da biblioteca: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="280"/>
        <location filename="../YACReaderLibrary/library_creator.cpp" line="355"/>
        <source>Unable to commit the library database: %1</source>
        <translation>Não foi possível confirmar a transação no banco de dados da biblioteca: %1</translation>
    </message>
</context>
<context>
    <name>LibraryMaintenanceLock</name>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="319"/>
        <source>The library lock owner changed. Retry the operation and review the new owner before recovering it.</source>
        <translation>O proprietário do bloqueio da biblioteca mudou. Tente a operação novamente e verifique o novo proprietário antes de recuperar o acesso.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="323"/>
        <source>Unable to remove the maintenance lock: %1. Make sure the file is not read-only and that you can write to the library data folder, then retry.</source>
        <translation>Não foi possível remover o bloqueio de manutenção: %1. Confirme que o arquivo não é somente leitura e que você pode gravar na pasta de dados da biblioteca, e tente novamente.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="379"/>
        <source>A folder is occupying the maintenance lock file path: %1. Rename or move that folder before retrying.</source>
        <translation>Uma pasta ocupa o caminho do arquivo de bloqueio de manutenção: %1. Renomeie ou mova essa pasta antes de tentar novamente.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="381"/>
        <source>Unable to create the library lock file: %1. Check write permissions for the library data folder.</source>
        <translation>Não foi possível criar o arquivo de bloqueio da biblioteca: %1. Verifique as permissões de gravação da pasta de dados da biblioteca.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="383"/>
        <source>The library data folder is missing: %1. Restore the folder or recreate the library before updating it.</source>
        <translation>A pasta de dados da biblioteca está ausente: %1. Restaure a pasta ou recrie a biblioteca antes de atualizá-la.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="389"/>
        <source>The library has a maintenance lock whose owner could not be verified (%1)%2</source>
        <translation>A biblioteca tem um bloqueio de manutenção cujo proprietário não pôde ser verificado (%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="390"/>
        <source>Another maintenance operation is using this library (%1)%2</source>
        <translation>Outra operação de manutenção está usando esta biblioteca (%1)%2</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/data_base_management.cpp" line="386"/>
        <source>Unable to create the library lock file: %1. Check available disk space and access to the library data folder.</source>
        <translation>Não foi possível criar o arquivo de bloqueio da biblioteca: %1. Verifique o espaço disponível em disco e o acesso à pasta de dados da biblioteca.</translation>
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
YACReaderLibraryServer é a versão headless (sem interface gráfica) do YACReaderLibrary.

Este aplicativo suporta configurações persistentes, para configurá-las edite este arquivo %1
Para saber mais sobre as configurações disponíveis, verifique a documentação em https://raw.githubusercontent.com/YACReader/yacreader/develop/YACReaderLibraryServer/SETTINGS_README.md</translation>
    </message>
</context>
<context>
    <name>SearchFieldRegistry</name>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="16"/>
        <source>Text, quoted text</source>
        <translation>Texto, texto entre aspas</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="18"/>
        <source>Integer</source>
        <translation>Inteiro</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="21"/>
        <source>Boolean (true / false)</source>
        <translation>Booleano (true / false)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="24"/>
        <source>Integer (number of days)</source>
        <translation>Inteiro (número de dias)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="27"/>
        <source>Enum (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</source>
        <translation>Enumeração (comic, manga, westernmanga, webcomic/web, 4koma/yonkoma)</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="100"/>
        <source>Comic title</source>
        <translation>Título do quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="101"/>
        <source>Series name</source>
        <translation>Nome da série</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="102"/>
        <source>Issue number</source>
        <translation>Número da edição</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="103"/>
        <source>Volume identifier</source>
        <translation>Identificador do volume</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="104"/>
        <source>Reading format</source>
        <translation>Formato de leitura</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="105"/>
        <source>Comic rating</source>
        <translation>Avaliação do quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="106"/>
        <source>Textual tags</source>
        <translation>Tags de texto</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="108"/>
        <source>Writer credit</source>
        <translation>Crédito do roteirista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="109"/>
        <source>Penciller credit</source>
        <translation>Crédito do desenhista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="110"/>
        <source>Inker credit</source>
        <translation>Crédito do arte-finalista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="111"/>
        <source>Colorist credit</source>
        <translation>Crédito do colorista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="112"/>
        <source>Letterer credit</source>
        <translation>Crédito do letrista</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="113"/>
        <source>Cover artist credit</source>
        <translation>Crédito do artista da capa</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="114"/>
        <source>Editor credit</source>
        <translation>Crédito do editor</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="116"/>
        <source>Story arc name</source>
        <translation>Nome do arco da história</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="117"/>
        <source>Position within a story arc</source>
        <translation>Posição dentro de um arco da história</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="118"/>
        <source>Number of issues in a story arc</source>
        <translation>Número de edições em um arco da história</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="119"/>
        <source>Characters appearing in the comic</source>
        <translation>Personagens presentes no quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="120"/>
        <source>Teams appearing in the comic</source>
        <translation>Equipes presentes no quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="121"/>
        <source>Locations appearing in the comic</source>
        <translation>Locais presentes no quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="122"/>
        <source>Primary character or team</source>
        <translation>Personagem ou equipe principal</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="123"/>
        <source>Comic synopsis</source>
        <translation>Sinopse do quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="125"/>
        <source>Publisher name</source>
        <translation>Nome da editora</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="126"/>
        <source>Publishing imprint</source>
        <translation>Selo editorial</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="127"/>
        <source>Publication format</source>
        <translation>Formato da publicação</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="128"/>
        <source>Recommended age rating</source>
        <translation>Classificação etária recomendada</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="129"/>
        <source>Comic genre</source>
        <translation>Gênero do quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="130"/>
        <source>ISO language code</source>
        <translation>Código de idioma ISO</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="131"/>
        <source>Publication date metadata</source>
        <translation>Metadados da data de publicação</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="132"/>
        <source>Series grouping metadata</source>
        <translation>Metadados de agrupamento de séries</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="133"/>
        <source>Alternate series name</source>
        <translation>Nome alternativo da série</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="134"/>
        <source>Alternate issue number</source>
        <translation>Número alternativo da edição</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="135"/>
        <source>Alternate series issue count</source>
        <translation>Quantidade de edições da série alternativa</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="136"/>
        <source>Number of issues in the series</source>
        <translation>Número de edições da série</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="138"/>
        <source>Whether the comic is marked as read</source>
        <translation>Se o quadrinho está marcado como lido</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="139"/>
        <source>Whether reading has started</source>
        <translation>Se a leitura foi iniciada</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="140"/>
        <source>Whether metadata has been edited</source>
        <translation>Se os metadados foram editados</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="141"/>
        <source>Whether the comic is in color</source>
        <translation>Se o quadrinho é colorido</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="142"/>
        <source>Number of pages</source>
        <translation>Número de páginas</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="143"/>
        <source>Comic file name</source>
        <translation>Nome do arquivo do quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="144"/>
        <source>When the item was added</source>
        <translation>Quando o item foi adicionado</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="145"/>
        <source>When the comic was last opened</source>
        <translation>Quando o quadrinho foi aberto pela última vez</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="146"/>
        <source>Comic notes</source>
        <translation>Notas do quadrinho</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="147"/>
        <source>Review text</source>
        <translation>Texto da resenha</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="149"/>
        <source>Parent folder name</source>
        <translation>Nome da pasta principal</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="150"/>
        <source>Default reading format for the folder</source>
        <translation>Formato de leitura padrão da pasta</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="151"/>
        <source>Whether the folder is complete</source>
        <translation>Se a pasta está completa</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="152"/>
        <source>Whether the folder is marked as finished</source>
        <translation>Se a pasta está marcada como finalizada</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/db/search_field_registry.cpp" line="153"/>
        <source>When the folder was updated</source>
        <translation>Quando a pasta foi atualizada</translation>
    </message>
</context>
<context>
    <name>YACReader::XMLInfoLibraryScanner</name>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="64"/>
        <source>The library database is missing. Restore a backup or recreate the library before rescanning XML information.</source>
        <translation>O banco de dados da biblioteca está ausente. Restaure um backup ou recrie a biblioteca antes de verificar novamente as informações XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="76"/>
        <source>Unable to load the archive backend: %1</source>
        <translation>Não foi possível carregar o mecanismo de arquivos compactados: %1</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="91"/>
        <source>Unable to start an XML rescan transaction.</source>
        <translation>Não foi possível iniciar uma transação de nova varredura XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="107"/>
        <source>The selected folder is no longer in this library. Reload the library before rescanning.</source>
        <translation>A pasta selecionada não está mais nesta biblioteca. Recarregue a biblioteca antes de verificar novamente.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="109"/>
        <source>Unable to read comics for the XML rescan.</source>
        <translation>Não foi possível ler os quadrinhos para a nova varredura XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="113"/>
        <source>Unable to commit XML metadata changes.</source>
        <translation>Não foi possível confirmar as alterações nos metadados XML.</translation>
    </message>
    <message>
        <location filename="../YACReaderLibrary/xml_info_library_scanner.cpp" line="157"/>
        <source>Unable to update XML metadata for %1.</source>
        <translation>Não foi possível atualizar os metadados XML de %1.</translation>
    </message>
</context>
</TS>
