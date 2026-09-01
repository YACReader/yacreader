add_library(image_processing STATIC
    resize_image.h
    resize_image.cpp
    lancir.h
)

target_include_directories(image_processing PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
yacreader_apply_build_options(image_processing)

target_link_libraries(image_processing PRIVATE Qt6::Gui)

# Raise the x86_64 baseline above the ancient default, on macOS only.
#
# lancir picks its SIMD path at compile time: __AVX__ selects 8-wide vectors,
# otherwise it falls back to 4-wide SSE2. macOS is the one platform where we
# know the oldest CPU we have to run on: our deployment target is macOS 12, and
# every Mac that can run it is Ivy Bridge or newer, so AVX is always available.
# AVX2 would need macOS 13 as the floor (the 2013 Mac Pro is Ivy Bridge, which
# has AVX but not AVX2). On Windows and Linux we have no such guarantee about
# the user's CPU, so those keep the portable baseline.
#
# -Xarch_x86_64 is self-gating: in a universal build it applies -mavx only to
# the x86_64 half, where -mavx is a valid option, and never to arm64. The
# IN_LIST check is therefore not required for correctness, only to keep clang
# from warning about an unused argument on arm64-only builds. SHELL: stops
# CMake from splitting or reordering the two tokens.
if(APPLE AND "x86_64" IN_LIST CMAKE_OSX_ARCHITECTURES)
    target_compile_options(image_processing PRIVATE "SHELL:-Xarch_x86_64 -mavx")
endif()
