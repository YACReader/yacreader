#!/bin/bash
#podman pull ubuntu:20.04
#podman run --device=/dev/fuse --cap-add SYS_ADMIN --cap-add MKNOD --tmpfs /tmp:exec -v ./:/tmp/out --rm -ti ubuntu:20.04 /tmp/out/appimage.sh
set -ex

OUT_DIR=/tmp/out
DEST=/usr/local
YACREADER_GIT_URL='https://github.com/YACReader/yacreader.git'
YACREADER_GIT_TAG='9.8.2.2106204'
UNARR_GIT_URL='https://github.com/selmf/unarr.git'
UNARR_GIT_COMMIT='e6fb85e8cdd1b1f9086e6e42be8d385f44136faa'
GN_GIT_URL='https://gn.googlesource.com/gn'
GN_GIT_COMMIT='80a40b07305373617eba2d5878d353532af77da3'
PDFIUM_GIT_URL='https://pdfium.googlesource.com/pdfium'
# curl -sSL 'https://omahaproxy.appspot.com/linux?channel=stable' | cut -d'.' -f 3
PDFIUM_GIT_VERSION='4896'
PDFIUM_GIT_BRANCH="chromium/$PDFIUM_GIT_VERSION"
PDFIUM_BUILD_GIT_URL='https://chromium.googlesource.com/chromium/src/build.git'
# awk '/build_revision/ {print substr($2,2,40)}' pdfium/DEPS
PDFIUM_BUILD_GIT_COMMIT='893ac785aa454dcf84dcc26af1a410095c1d4fa2'
PDFIUM_ABSEIL_CPP_GIT_URL='https://chromium.googlesource.com/chromium/src/third_party/abseil-cpp'
# awk '/abseil_revision/ {print substr($2,2,40)}' pdfium/DEPS
PDFIUM_ABSEIL_CPP_GIT_COMMIT='9ffdb583473a319b3920c752badde40f91ffd609'

# building in temporary directory to keep system clean
# use RAM disk if possible (as in: not building on CI system like Travis, and RAM disk is available)
# if [[ -z "$CI" ]] && [[ -d /dev/shm ]]; then
#     TEMP_BASE=/dev/shm
# else
#     TEMP_BASE=/tmp
# fi

TEMP_BASE=/tmp
BUILD_DIR="$(mktemp -d -p "$TEMP_BASE" appimage-build-XXXXXX)"

# make sure to clean up build dir, even if errors occur
cleanup() {
    if [[ -d "$BUILD_DIR" ]]; then
        rm -rf "$BUILD_DIR"
    fi
}
trap cleanup EXIT

cd "$BUILD_DIR"

export DEBIAN_FRONTEND=noninteractive
apt -q -y update && apt -q -y upgrade
apt -q -y install build-essential \
    cmake \
    curl \
    fuse \
    git \
    libbz2-dev \
    libfreetype-dev \
    libfuse2 \
    libicu-dev \
    liblcms2-dev \
    liblzma-dev \
    libopenjp2-7-dev \
    libqt5multimedia5-plugins \
    libqt5network5 \
    libqt5sql5-sqlite \
    libqt5svg5-dev \
    libturbojpeg0-dev \
    mesa-common-dev \
    ninja-build \
    pkg-config \
    python3 \
    qt5-image-formats-plugins \
    qt5-qmake \
    qtbase5-dev \
    qtdeclarative5-dev \
    qtmultimedia5-dev \
    qtquickcontrols2-5-dev

# source the default compiler flags
eval $(dpkg-buildflags --export=sh)
# for gn
export CC=gcc CXX=g++

# compile and install libunarr
git clone "$UNARR_GIT_URL" unarr
cd unarr
git checkout "$UNARR_GIT_COMMIT"
cmake -DENABLE_7Z=ON -DCMAKE_INSTALL_PREFIX="$DEST" -DCMAKE_INSTALL_LIBDIR=lib .
make -j$(nproc)
make install
cd ..

# regenerate dynamic linker cache
ldconfig

# compile and install gn
git clone "$GN_GIT_URL" gn
cd gn
git checkout "$GN_GIT_COMMIT"
./build/gen.py
ninja -C out
strip -s out/gn
install -D out/gn "$DEST"/bin/gn
cd ..

# https://aur.archlinux.org/cgit/aur.git/tree/PKGBUILD?h=libpdfium-nojs
# compile and install pdfium
git clone --branch "$PDFIUM_GIT_BRANCH" --depth 1 "$PDFIUM_GIT_URL" pdfium
git clone "$PDFIUM_BUILD_GIT_URL" pdfium/build
git -C pdfium/build checkout "$PDFIUM_BUILD_GIT_COMMIT"
# awk '/abseil_revision/ {print substr($2,2,40)}' pdfium/DEPS
git clone "$PDFIUM_ABSEIL_CPP_GIT_URL" pdfium/third_party/abseil-cpp
git -C pdfium/third_party/abseil-cpp checkout "$PDFIUM_ABSEIL_CPP_GIT_COMMIT"
# Download and decode shim header script needed to unbundle icu (gittiles is weird)
mkdir -p pdfium/tools/generate_shim_headers
curl -sSL https://chromium.googlesource.com/chromium/src/+/master/tools/generate_shim_headers/generate_shim_headers.py?format=TEXT | base64 -d > pdfium/tools/generate_shim_headers/generate_shim_headers.py
curl -sSL https://aur.archlinux.org/cgit/aur.git/plain/libpdfium.pc?h=libpdfium-nojs > libpdfium.pc
# Fix visibility for system Freetype
git -C pdfium/build cherry-pick -n bfd6ff0
# Patch abseil build to be static
sed -i -e 's/component(/static_library(/' -e 's/is_component_build(/false/' pdfium/third_party/abseil-cpp/BUILD.gn
# Use system provided icu library (unbundling)
mkdir -p pdfium/third_party/icu
ln -sf ../../build/linux/unbundle/icu.gn pdfium/third_party/icu/BUILD.gn
# Create fake gclient_args.gni file to satisfy include list for build/config/compiler/compiler.gni
touch pdfium/build/config/gclient_args.gni
# Exclude test fonts from build
sed -i '/"\/\/third_party\/test_fonts",/s/^/#/g' pdfium/testing/BUILD.gn
# Build
gn --root=pdfium gen pdfium/out/Release --args="is_clang=false use_sysroot=false is_debug=false symbol_level=0 pdf_enable_v8=false pdf_enable_xfa=false treat_warnings_as_errors=false use_system_libjpeg=true use_system_zlib=true pdf_bundle_freetype=false use_system_freetype=true use_system_lcms2=true use_system_libpng=true use_custom_libcxx=false pdf_is_standalone=true use_system_libopenjpeg2=true is_component_build=true use_gold=false"
ninja -C pdfium/out/Release pdfium
sed -e 's/@VERSION@/'"$PDFIUM_GIT_VERSION"'/g' -e 's#^\s*prefix\s*=.*$#prefix='"$DEST"'#' -i libpdfium.pc
# Install
install -D -m644 pdfium/LICENSE "$DEST/share/licenses/pdfium/LICENSE"
install -D pdfium/public/*.h --target-directory="$DEST/include/pdfium"
install -D pdfium/public/cpp/* --target-directory="$DEST/include/pdfium/cpp"
install -D pdfium/docs/* --target-directory="$DEST/share/doc/pdfium"
install -Dm755 pdfium/out/Release/libpdfium.so --target-directory="$DEST/lib"
install -Dm644 libpdfium.pc --target-directory="$DEST/lib/pkgconfig"

# regenerate dynamic linker cache
ldconfig

# compile and install YACReader
git clone --branch "$YACREADER_GIT_TAG" --depth 1 "$YACREADER_GIT_URL" yacreader
cd yacreader
qmake CONFIG+="unarr pdfium server_bundled"
make -j$(nproc)
mkdir -p "$BUILD_DIR"/AppDir{YACReader,YACReaderLibrary}
make sub-YACReader-install_subtargets INSTALL_ROOT="$BUILD_DIR/AppDirYACReader"
make sub-YACReaderLibrary-install_subtargets INSTALL_ROOT="$BUILD_DIR/AppDirYACReaderLibrary"
cd ..

# compile wrapper for YACReaderLibrary in order to find YACReader
gcc $CFLAGS $LDFLAGS -o "$BUILD_DIR/AppDirYACReaderLibrary/usr/bin/YACReader" "$OUT_DIR/YACReader.c"
strip -s "$BUILD_DIR/AppDirYACReaderLibrary/usr/bin/YACReader"

# use a custom plugin to set the PATH inside the appimage and find the YACReader wrapper
cp "$OUT_DIR/linuxdeploy-plugin-path.sh" .

# create appimages
curl -sSLo linuxdeploy-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy/releases/download/continuous/linuxdeploy-x86_64.AppImage
chmod +x linuxdeploy-x86_64.AppImage
curl -sSLo linuxdeploy-plugin-qt-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-qt/releases/download/continuous/linuxdeploy-plugin-qt-x86_64.AppImage
chmod +x linuxdeploy-plugin-qt-x86_64.AppImage
curl -sSLo linuxdeploy-plugin-appimage-x86_64.AppImage https://github.com/linuxdeploy/linuxdeploy-plugin-appimage/releases/download/continuous/linuxdeploy-plugin-appimage-x86_64.AppImage
chmod +x linuxdeploy-plugin-appimage-x86_64.AppImage

./linuxdeploy-x86_64.AppImage --appdir AppDirYACReader \
    --executable AppDirYACReader/usr/bin/YACReader \
    --desktop-file AppDirYACReader/usr/share/applications/YACReader.desktop \
    --icon-file AppDirYACReader/usr/share/icons/hicolor/scalable/apps/YACReader.svg \
    --plugin qt \
    --output appimage
./linuxdeploy-x86_64.AppImage --appdir AppDirYACReaderLibrary \
    --executable AppDirYACReaderLibrary/usr/bin/YACReaderLibrary \
    --desktop-file AppDirYACReaderLibrary/usr/share/applications/YACReaderLibrary.desktop \
    --icon-file AppDirYACReaderLibrary/usr/share/icons/hicolor/scalable/apps/YACReaderLibrary.svg \
    --plugin qt \
    --plugin path \
    --output appimage
mv YACReader*.AppImage "$OUT_DIR"
