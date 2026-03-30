add_library(image_processing STATIC
    resize_image.h
    resize_image.cpp
    lancir.h
)

target_include_directories(image_processing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
yacreader_apply_build_options(image_processing)

target_link_libraries(image_processing PRIVATE Qt6::Gui)
