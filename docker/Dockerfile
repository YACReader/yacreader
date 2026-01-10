# Stage 1: 7z builder (cached separately for faster rebuilds)
FROM ghcr.io/linuxserver/baseimage-ubuntu:noble AS sevenzip-builder

ARG DEBIAN_FRONTEND="noninteractive"

# install minimal build packages for 7z compilation
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    gcc \
    g++ \
    make \
    unzip \
    wget \
    7zip && \
    apt-get clean && \
    rm -rf /var/lib/apt/lists/*

# download and extract 7z source
RUN mkdir -p /src/7zip && \
    cd /src/7zip && \
    wget "https://github.com/YACReader/yacreader-7z-deps/blob/main/7z2301-src.7z?raw=true" -O 7z2301-src.7z && \
    7z x 7z2301-src.7z -o/src/7zip/lib7zip && \
    rm 7z2301-src.7z

# build 7z.so with RAR support
RUN echo "Building 7z.so with RAR support..." && \
    cd "/src/7zip/lib7zip/CPP/7zip/Bundles/Format7zF" && \
    make -f makefile.gcc && \
    mkdir -p /app/lib/7zip && \
    cp ./_o/7z.so /app/lib/7zip

# Stage 2: Main application builder
FROM ghcr.io/linuxserver/baseimage-ubuntu:noble AS builder

# version, it can be a TAG or a branch
ARG YACR_VERSION="develop"

# env variables
ARG DEBIAN_FRONTEND="noninteractive"
ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
LABEL maintainer="luisangelsm"

# install build packages
RUN \
    apt-get update && \
    apt-get install -y --no-install-recommends \
    build-essential \
    desktop-file-utils \
    gcc \
    g++ \
    git \
    qt6-tools-dev \
    qt6-base-dev-tools \
    qmake6 \
    qmake6-bin \
    qt6-base-dev \
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
    make \
    sqlite3 \
    libsqlite3-dev \
    unzip \
    wget \
    7zip \
    7zip-rar \
    libpoppler-qt6-dev \
    zlib1g-dev \
    libavif-dev \
    libjxl-dev && \
    ldconfig


# clone YACReader repo
RUN git clone https://github.com/YACReader/yacreader.git /src/git && \
    cd /src/git && \
    git checkout $YACR_VERSION

# copy pre-built 7z library and headers from sevenzip-builder stage
COPY --from=sevenzip-builder /src/7zip/lib7zip /src/git/compressed_archive/lib7zip

# build yacreaderlibraryserver
RUN cd /src/git/YACReaderLibraryServer && \
    qmake6 PREFIX=/app CONFIG+="7zip server_standalone" YACReaderLibraryServer.pro && \
    qmake6 -v && \
    make  && \
    make install

# install pre-built 7z.so with RAR support
RUN mkdir -p /app/lib/7zip && \
    cp /src/git/compressed_archive/lib7zip/CPP/7zip/Bundles/Format7zF/_o/7z.so /app/lib/7zip

# Stage 2: Runtime stage
FROM ghcr.io/linuxserver/baseimage-ubuntu:noble AS runtime

# env variables
ENV APPNAME="YACReaderLibraryServer"
ENV HOME="/config"
LABEL maintainer="luisangelsm"

# Copy the built application from the builder stage
COPY --from=builder /app /app

# runtime packages
RUN apt-get update && \
    apt-get install -y --no-install-recommends \
    libqt6core5compat6 \
    libpoppler-qt6-3t64 \
    qt6-image-formats-plugins \
    libqt6network6t64 \
    libqt6sql6-sqlite \
    libavif16 \
    libjxl0.7 && \
    apt-get clean && \
ENV LC_ALL="en_US.UTF-8" \
    PATH="/app/bin:${PATH}"

# copy files
COPY root.tar.gz /
# Extract the contents of root.tar.gz into the / directory
RUN tar -xvpzf /root.tar.gz -C /

# ports and volumes
EXPOSE 8080
VOLUME ["/config", "/comics"]
