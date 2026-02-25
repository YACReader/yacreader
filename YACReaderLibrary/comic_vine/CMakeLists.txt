# Comic Vine scraper for YACReaderLibrary

add_library(comic_vine STATIC
    comic_vine_json_parser.h
    comic_vine_json_parser.cpp
    comic_vine_all_volume_comics_retriever.h
    comic_vine_all_volume_comics_retriever.cpp
    scraper_checkbox.h
    scraper_checkbox.cpp
    comic_vine_dialog.h
    comic_vine_dialog.cpp
    comic_vine_client.h
    comic_vine_client.cpp
    scraper_lineedit.h
    scraper_lineedit.cpp
    title_header.h
    title_header.cpp
    series_question.h
    series_question.cpp
    search_single_comic.h
    search_single_comic.cpp
    search_volume.h
    search_volume.cpp
    select_comic.h
    select_comic.cpp
    select_volume.h
    select_volume.cpp
    scraper_tableview.h
    scraper_tableview.cpp
    sort_volume_comics.h
    sort_volume_comics.cpp
    scraper_scroll_label.h
    scraper_scroll_label.cpp
    scraper_results_paginator.h
    scraper_results_paginator.cpp
    api_key_dialog.h
    api_key_dialog.cpp
    model/volumes_model.h
    model/volumes_model.cpp
    model/comics_model.h
    model/comics_model.cpp
    model/json_model.h
    model/json_model.cpp
    model/response_parser.h
    model/response_parser.cpp
    model/local_comic_list_model.h
    model/local_comic_list_model.cpp
    model/volume_comics_model.h
    model/volume_comics_model.cpp
    model/selected_volume_info.h
    model/volume_search_query.h
)
target_include_directories(comic_vine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/model
)
# App-specific theme.h needed for themable.h → theme_manager.h → theme.h chain
target_include_directories(comic_vine PRIVATE
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/themes
    ${CMAKE_SOURCE_DIR}/YACReaderLibrary/db
)
target_link_libraries(comic_vine PUBLIC
    Qt::Core
    Qt::Widgets
    Qt::Network
    common_all
    common_gui
    custom_widgets_library
    db_helper
    QsLog
)
