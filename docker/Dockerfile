# ============================================================================
# Stage 0: Common base, defined here like a constant for multiple subsequent stages
# ============================================================================
FROM ghcr.io/linuxserver/baseimage-ubuntu:noble AS base

# ============================================================================
# Stage 1: build-base for all build steps
# ============================================================================
FROM base AS plugin-base

ARG DEBIAN_FRONTEND="noninteractive"

# Install common build packages used by all plugin builders
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    7zip \
    build-essential \
    cmake \
    extra-cmake-modules \
    g++ \
    gcc \
    git \
    libavif-dev \
    libheif-dev \
    make \
    qt6-base-dev \
    qt6-base-private-dev \
    unzip \
    wget \
    pkg-config && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# ============================================================================
# Stage 2: 7zip builder
# ============================================================================
FROM plugin-base AS sevenzip-builder

# Download 7z source
RUN mkdir -p /src/7zip
WORKDIR /src/7zip
RUN wget "https://github.com/YACReader/yacreader-7z-deps/blob/main/7z2301-src.7z?raw=true" -O 7z2301-src.7z

# Extract 7z source
RUN 7z x 7z2301-src.7z -o/src/7zip/lib7zip && \
    rm 7z2301-src.7z

# Build 7z.so with RAR support
WORKDIR /src/7zip/lib7zip/CPP/7zip/Bundles/Format7zF
RUN mkdir -p _o
RUN make -f makefile.gcc -j$(nproc)
RUN mkdir -p /app/lib/7zip && \
    cp ./_o/7z.so /app/lib/7zip

# ============================================================================
# Stage 3: HEIC/HEIF plugin builder
# ============================================================================
FROM plugin-base AS heic-plugin-builder

WORKDIR /tmp/qt-heic
RUN git clone https://github.com/novomesk/qt-heic-image-plugin.git .
RUN mkdir build
WORKDIR /tmp/qt-heic/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_MAJOR_VERSION=6
RUN make -j$(nproc)
RUN make install DESTDIR=/output

# ============================================================================
# Stage 4: libjxl + JXL plugin builder (merged)
# (0.10.2 required for JxlEncoderDistanceFromQuality)
# ============================================================================
FROM plugin-base AS jxl-plugin-builder

# Build libjxl 0.10.2
WORKDIR /tmp/libjxl
RUN git clone --depth 1 --branch v0.10.2 https://github.com/libjxl/libjxl.git .
RUN ./deps.sh
RUN mkdir build
WORKDIR /tmp/libjxl/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_TESTING=OFF \
    -DJPEGXL_ENABLE_BENCHMARK=OFF \
    -DJPEGXL_ENABLE_EXAMPLES=OFF \
    -DJPEGXL_ENABLE_MANPAGES=OFF \
    -DJPEGXL_ENABLE_PLUGINS=OFF \
    -DJPEGXL_ENABLE_TOOLS=OFF \
    -DCMAKE_INSTALL_PREFIX=/usr
RUN make -j$(nproc)
RUN make install
RUN make install DESTDIR=/output

# Build JXL plugin
WORKDIR /tmp/qt-jxl
RUN git clone https://github.com/novomesk/qt-jpegxl-image-plugin.git .
RUN mkdir build
WORKDIR /tmp/qt-jxl/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_MAJOR_VERSION=6
RUN make -j$(nproc)
RUN make install DESTDIR=/output

# ============================================================================
# Stage 5: AVIF plugin builder
# ============================================================================
FROM plugin-base AS avif-plugin-builder

WORKDIR /tmp/qt-avif
RUN git clone https://github.com/novomesk/qt-avif-image-plugin.git .
RUN mkdir build
WORKDIR /tmp/qt-avif/build
RUN cmake .. \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_INSTALL_PREFIX=/usr \
    -DQT_MAJOR_VERSION=6
RUN make -j$(nproc)
RUN make install DESTDIR=/output

# ============================================================================
# Stage 6: YACReader builder
# ============================================================================
FROM plugin-base AS yacreader-builder

ARG YACR_VERSION="develop"
ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
LABEL maintainer="luisangelsm"

# Install YACReader-specific build packages (common packages inherited from plugin-base)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    desktop-file-utils \
    qt6-tools-dev \
    qt6-base-dev-tools \
    qmake6 \
    qmake6-bin \
    qt6-multimedia-dev \
    qt6-tools-dev-tools \
    libgl-dev \
    qt6-l10n-tools \
    libqt6opengl6-dev \
    libunarr-dev \
    qt6-declarative-dev \
    libqt6svg6-dev \
    libqt6core5compat6-dev \
    libbz2-dev \
    libglu1-mesa-dev \
    liblzma-dev \
    libqt6gui6 \
    libqt6multimedia6 \
    libqt6network6 \
    libqt6qml6 \
    libqt6quickcontrols2-6 \
    qt6-image-formats-plugins \
    libqt6sql6 \
    libqt6sql6-sqlite \
    sqlite3 \
    libsqlite3-dev \
    7zip-rar \
    libpoppler-qt6-dev \
    zlib1g-dev && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* && \
    ldconfig

# Clone YACReader repository
RUN git clone https://github.com/YACReader/yacreader.git /src/git && \
    cd /src/git && \
    git checkout $YACR_VERSION

# Copy pre-built 7z library and headers
COPY --from=sevenzip-builder /src/7zip/lib7zip /src/git/compressed_archive/lib7zip
COPY --from=sevenzip-builder /app/lib/7zip /app/lib/7zip

# Copy Qt image format plugins from separate stages
COPY --from=avif-plugin-builder /output/usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/ \
    /usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/
COPY --from=jxl-plugin-builder /output/usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/ \
    /usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/
COPY --from=heic-plugin-builder /output/usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/ \
    /usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/

# Build YACReaderLibraryServer
RUN cd /src/git/YACReaderLibraryServer && \
    qmake6 PREFIX=/app CONFIG+="7zip server_standalone" YACReaderLibraryServer.pro && \
    qmake6 -v && \
    make -j$(nproc) && \
    make install

# ============================================================================
# Stage 7: Final runtime image (minimal, no build assets)
# ============================================================================
FROM base AS runtime

ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
ENV LC_ALL="en_US.UTF-8"
ENV PATH="/app/bin:${PATH}"
LABEL maintainer="luisangelsm"

# Install only runtime dependencies (no build tools)
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libqt6core5compat6 \
    libpoppler-qt6-3t64 \
    qt6-image-formats-plugins \
    libqt6network6t64 \
    libqt6sql6-sqlite \
    libavif16 \
    libheif1 && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/* /tmp/* /var/tmp/*

# Copy YACReaderLibraryServer application from builder
COPY --from=yacreader-builder /app /app

# Copy Qt image format plugins from plugin builders
COPY --from=avif-plugin-builder /output/usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/ \
    /usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/
COPY --from=jxl-plugin-builder /output/usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/ \
    /usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/
COPY --from=heic-plugin-builder /output/usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/ \
    /usr/lib/x86_64-linux-gnu/qt6/plugins/imageformats/

# Copy libjxl 0.10.2 runtime libraries (not available in Ubuntu noble)
COPY --from=jxl-plugin-builder /output/usr/lib/ /usr/lib/

# Refresh dynamic linker cache
RUN ldconfig

# Copy linuxserver.io configuration
COPY docker/root.tar.gz /
RUN tar -xvpzf /root.tar.gz -C / && rm /root.tar.gz

# Expose server port and define volumes
EXPOSE 8080
VOLUME ["/config", "/comics"]
