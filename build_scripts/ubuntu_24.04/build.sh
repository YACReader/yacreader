#!/bin/bash

# Exit immediately if a command exits with a non-zero status
set -e

# Define variables
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
BUILD_DIR="$SCRIPT_DIR/yacreader_build"
YACREADER_DIR="$BUILD_DIR/yacreader"
COMPRESSED_ARCHIVE_DIR="$YACREADER_DIR/compressed_archive"
YACREADER_REPO="https://github.com/YACReader/yacreader.git"
SEVENZIP_URL="https://github.com/YACReader/yacreader-7z-deps/blob/main/7z2301-src.7z?raw=true"
SEVENZIP_ARCHIVE="$COMPRESSED_ARCHIVE_DIR/7z2301-src.7z"
SEVENZIP_DIR="$COMPRESSED_ARCHIVE_DIR/lib7zip"

# Function to download the latest release source tarball
download_latest_release() {
    echo "Fetching the latest release information..."
    LATEST_RELEASE_URL=$(curl -s https://api.github.com/repos/YACReader/yacreader/releases/latest | grep "browser_download_url.*src.tar.xz" | cut -d '"' -f 4 | head -n 1)
    if [ -z "$LATEST_RELEASE_URL" ]; then
        echo "Failed to fetch the latest release information."
        exit 1
    fi
    echo "Downloading the latest release source tarball..."
    wget "$LATEST_RELEASE_URL" -O "$BUILD_DIR/latest_release_src.tar.xz"
    echo "Extracting the latest release source tarball..."
    mkdir -p "$YACREADER_DIR"
    tar -xf "$BUILD_DIR/latest_release_src.tar.xz" -C "$YACREADER_DIR" --strip-components=1
}

# Step 1: Create a folder for the build process
if [ -d "$BUILD_DIR" ]; then
    echo "Removing existing build directory..."
    rm -rf "$BUILD_DIR"
fi
echo "Creating build directory..."
mkdir -p "$BUILD_DIR"
mkdir -p "$COMPRESSED_ARCHIVE_DIR"

# Step 2: Install required packages
echo "Updating package list and installing required packages..."
sudo apt-get update
sudo apt-get install -y qtchooser qt6-tools-dev qt6-base-dev-tools qmake6 qmake6-bin qt6-base-dev qt6-multimedia-dev qt6-tools-dev-tools libgl-dev qt6-l10n-tools libqt6opengl6-dev qt6-declarative-dev libqt6svg6-dev libqt6core5compat6-dev qml6-module-qt5compat-graphicaleffects qt6-5compat-dev  libpoppler-qt6-dev qt6-image-formats-plugins qml6-module-qtquick-controls qml6-module-qtquick-layouts qml6-module-qtqml-workerscript qml6-module-qtquick-templates 7zip-full 7zip-rar git build-essential

# Step 3: Download YACReader source code
if [ "$1" == "develop" ]; then
    if [ -d "$YACREADER_DIR" ]; then
        echo "Removing existing YACReader directory..."
        rm -rf "$YACREADER_DIR"
    fi
    echo "Cloning YACReader repository..."
    git clone "$YACREADER_REPO" "$YACREADER_DIR"
else
    download_latest_release
fi

# Step 4: Download and extract 7zip source code
echo "Downloading 7zip source code..."
wget "$SEVENZIP_URL" -O "$SEVENZIP_ARCHIVE"
echo "Extracting 7zip source code..."
7z x "$SEVENZIP_ARCHIVE" -o"$SEVENZIP_DIR"

# Step 5: Build and install 7z.so with RAR support
echo "Building and installing 7z.so with RAR support..."
cd "$SEVENZIP_DIR/CPP/7zip/Bundles/Format7zF"
make -f makefile.gcc
sudo mkdir -p /usr/lib/7zip
sudo cp ./_o/7z.so /usr/lib/7zip

# Step 6: Build YACReader
echo "Building YACReader..."
cd "$YACREADER_DIR"
export DEFINES_VAR=DEFINES+\=\"BUILD_NUMBER=\\\\\\\"CUSTOM_BUILD\\\\\\\"\"
qmake6 CONFIG+="7zip" $DEFINES_VAR
make

# Step 7: Install YACReader
echo "Installing YACReader..."
sudo make install

echo "YACReader build and installation complete."